#include "TAStructureSolver.h"

namespace
{
    bool IsValidConstraint(
        const FTADistanceConstraint& Constraint,
        const TArray<FTAStructureNode>& Nodes)
    {
        return
            !Constraint.bBroken
            && Nodes.IsValidIndex(Constraint.NodeA)
            && Nodes.IsValidIndex(Constraint.NodeB)
            && Constraint.NodeA != Constraint.NodeB;
    }
}

void TAStructureSolver::PredictPositions(
    const FTAStructureSolverConfig& Config,
    const double DeltaTimeSeconds,
    TArray<FTAStructureNode>& Nodes)
{
    if (DeltaTimeSeconds <= 0.0)
    {
        return;
    }

    for (FTAStructureNode& Node : Nodes)
    {
        Node.PreviousPositionM = Node.PositionM;

        if (Node.bPinned || Node.InverseMassPerKg <= 0.0)
        {
            Node.VelocityMps = FVector3d::ZeroVector;
            continue;
        }

        Node.VelocityMps += Config.GravityMps2 * DeltaTimeSeconds;
        Node.PositionM += Node.VelocityMps * DeltaTimeSeconds;
    }
}

void TAStructureSolver::SolveDistanceConstraints(
    const FTAStructureSolverConfig& Config,
    const double DeltaTimeSeconds,
    TArray<FTAStructureNode>& Nodes,
    TArray<FTADistanceConstraint>& Constraints)
{
    if (DeltaTimeSeconds <= 0.0)
    {
        return;
    }

    const int32 Iterations = FMath::Max(1, Config.ConstraintIterations);
    const double DtSquared = DeltaTimeSeconds * DeltaTimeSeconds;

    for (FTADistanceConstraint& Constraint : Constraints)
    {
        Constraint.AccumulatedLambda = 0.0;
    }

    for (int32 Iteration = 0; Iteration < Iterations; ++Iteration)
    {
        for (FTADistanceConstraint& Constraint : Constraints)
        {
            if (!IsValidConstraint(Constraint, Nodes))
            {
                continue;
            }

            FTAStructureNode& NodeA = Nodes[Constraint.NodeA];
            FTAStructureNode& NodeB = Nodes[Constraint.NodeB];

            const FVector3d Delta = NodeB.PositionM - NodeA.PositionM;
            const double LengthM = Delta.Length();

            if (LengthM <= UE_DOUBLE_SMALL_NUMBER)
            {
                continue;
            }

            const FVector3d Direction = Delta / LengthM;

            const double InverseMassA =
                NodeA.bPinned ? 0.0 : FMath::Max(0.0, NodeA.InverseMassPerKg);

            const double InverseMassB =
                NodeB.bPinned ? 0.0 : FMath::Max(0.0, NodeB.InverseMassPerKg);

            const double WeightSum = InverseMassA + InverseMassB;
            if (WeightSum <= UE_DOUBLE_SMALL_NUMBER)
            {
                continue;
            }

            const double ConstraintErrorM =
                LengthM - FMath::Max(UE_DOUBLE_SMALL_NUMBER, Constraint.RestLengthM);

            const double AlphaTilde =
                FMath::Max(0.0, Constraint.Compliance) / DtSquared;

            const double DeltaLambda =
                (-ConstraintErrorM - AlphaTilde * Constraint.AccumulatedLambda)
                / (WeightSum + AlphaTilde);

            Constraint.AccumulatedLambda += DeltaLambda;

            const double MaxCorrection =
                FMath::Max(0.0, Config.MaxPositionCorrectionM);

            const FVector3d CorrectionA =
                (-Direction * (InverseMassA * DeltaLambda))
                .GetClampedToMaxSize(MaxCorrection);

            const FVector3d CorrectionB =
                (Direction * (InverseMassB * DeltaLambda))
                .GetClampedToMaxSize(MaxCorrection);

            if (!NodeA.bPinned)
            {
                NodeA.PositionM += CorrectionA;
            }

            if (!NodeB.bPinned)
            {
                NodeB.PositionM += CorrectionB;
            }
        }
    }
}

void TAStructureSolver::UpdatePlasticityAndFracture(
    const FTAStructureSolverConfig& Config,
    TArray<FTAStructureNode>& Nodes,
    TArray<FTADistanceConstraint>& Constraints)
{
    for (FTADistanceConstraint& Constraint : Constraints)
    {
        if (!IsValidConstraint(Constraint, Nodes))
        {
            continue;
        }

        const FTAStructureNode& NodeA = Nodes[Constraint.NodeA];
        const FTAStructureNode& NodeB = Nodes[Constraint.NodeB];

        const double CurrentLengthM =
            (NodeB.PositionM - NodeA.PositionM).Length();

        const double RestLengthM =
            FMath::Max(UE_DOUBLE_SMALL_NUMBER, Constraint.RestLengthM);

        const double Strain =
            (CurrentLengthM - RestLengthM) / RestLengthM;

        const double AbsStrain = FMath::Abs(Strain);

        const double FractureThreshold =
            FMath::Max(
                FMath::Max(0.0, Constraint.YieldStrain),
                FMath::Max(0.0, Constraint.FractureStrain));

        if (AbsStrain >= FractureThreshold)
        {
            Constraint.bBroken = true;
            continue;
        }

        if (AbsStrain <= FMath::Max(0.0, Constraint.YieldStrain))
        {
            continue;
        }

        const double ExcessStrain =
            AbsStrain - Constraint.YieldStrain;

        const double Flow =
            FMath::Clamp(Constraint.PlasticFlowRate01, 0.0, 1.0);

        const double RequestedRestChangeM =
            FMath::Sign(Strain)
            * ExcessStrain
            * RestLengthM
            * Flow;

        const double MaxRestChangeM =
            RestLengthM
            * FMath::Max(0.0, Config.MaxPlasticRestChangeFractionPerStep);

        const double AppliedRestChangeM =
            FMath::Clamp(
                RequestedRestChangeM,
                -MaxRestChangeM,
                MaxRestChangeM);

        Constraint.RestLengthM =
            FMath::Max(
                UE_DOUBLE_SMALL_NUMBER,
                Constraint.RestLengthM + AppliedRestChangeM);

        Constraint.AccumulatedPlasticStrain +=
            FMath::Abs(AppliedRestChangeM) / RestLengthM;
    }
}

void TAStructureSolver::FinalizeVelocities(
    const double DeltaTimeSeconds,
    TArray<FTAStructureNode>& Nodes)
{
    if (DeltaTimeSeconds <= 0.0)
    {
        return;
    }

    for (FTAStructureNode& Node : Nodes)
    {
        if (Node.bPinned || Node.InverseMassPerKg <= 0.0)
        {
            Node.VelocityMps = FVector3d::ZeroVector;
            continue;
        }

        Node.VelocityMps =
            (Node.PositionM - Node.PreviousPositionM) / DeltaTimeSeconds;
    }
}

void TAStructureSolver::Step(
    const FTAStructureSolverConfig& Config,
    const double DeltaTimeSeconds,
    TArray<FTAStructureNode>& Nodes,
    TArray<FTADistanceConstraint>& Constraints)
{
    PredictPositions(Config, DeltaTimeSeconds, Nodes);
    SolveDistanceConstraints(Config, DeltaTimeSeconds, Nodes, Constraints);
    UpdatePlasticityAndFracture(Config, Nodes, Constraints);
    FinalizeVelocities(DeltaTimeSeconds, Nodes);
}
