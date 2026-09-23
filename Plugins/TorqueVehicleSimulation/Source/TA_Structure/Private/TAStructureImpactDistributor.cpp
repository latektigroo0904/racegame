#include "TAStructureImpactDistributor.h"

namespace
{
    struct FSymmetricMatrix3
    {
        double XX = 0.0;
        double YY = 0.0;
        double ZZ = 0.0;
        double XY = 0.0;
        double XZ = 0.0;
        double YZ = 0.0;
    };

    bool SolveSymmetric3x3(
        const FSymmetricMatrix3& M,
        const FVector3d& B,
        FVector3d& OutX)
    {
        const double A = M.XX;
        const double Bv = M.XY;
        const double C = M.XZ;
        const double D = M.YY;
        const double E = M.YZ;
        const double F = M.ZZ;

        const double Cof00 = D * F - E * E;
        const double Cof01 = C * E - Bv * F;
        const double Cof02 = Bv * E - C * D;
        const double Cof11 = A * F - C * C;
        const double Cof12 = Bv * C - A * E;
        const double Cof22 = A * D - Bv * Bv;

        const double Determinant =
            A * Cof00
            + Bv * Cof01
            + C * Cof02;

        if (!FMath::IsFinite(Determinant) ||
            FMath::Abs(Determinant) <= 1.0e-12)
        {
            return false;
        }

        const double InvDet =
            1.0 / Determinant;

        OutX.X =
            InvDet
            * (Cof00 * B.X
                + Cof01 * B.Y
                + Cof02 * B.Z);

        OutX.Y =
            InvDet
            * (Cof01 * B.X
                + Cof11 * B.Y
                + Cof12 * B.Z);

        OutX.Z =
            InvDet
            * (Cof02 * B.X
                + Cof12 * B.Y
                + Cof22 * B.Z);

        return
            FMath::IsFinite(OutX.X)
            && FMath::IsFinite(OutX.Y)
            && FMath::IsFinite(OutX.Z);
    }

    double GetNodeMassKg(
        const FTAStructureNode& Node)
    {
        if (Node.bPinned ||
            Node.InverseMassPerKg <= UE_DOUBLE_SMALL_NUMBER)
        {
            return 0.0;
        }

        return
            1.0 / Node.InverseMassPerKg;
    }

    FVector3d CalculateLinearImpulseSum(
        const TArray<FVector3d>& Impulses)
    {
        FVector3d Sum =
            FVector3d::ZeroVector;

        for (const FVector3d& Impulse : Impulses)
        {
            Sum += Impulse;
        }

        return Sum;
    }

    FVector3d CalculateAngularImpulseSum(
        const TArray<FTAStructureNode>& Nodes,
        const TArray<int32>& NodeIndices,
        const TArray<FVector3d>& Impulses,
        const FVector3d& CenterOfMassLocalM)
    {
        FVector3d Sum =
            FVector3d::ZeroVector;

        for (int32 ScratchIndex = 0;
             ScratchIndex < NodeIndices.Num();
             ++ScratchIndex)
        {
            const int32 NodeIndex =
                NodeIndices[ScratchIndex];

            const FVector3d R =
                Nodes[NodeIndex].PositionM
                - CenterOfMassLocalM;

            Sum +=
                FVector3d::CrossProduct(
                    R,
                    Impulses[ScratchIndex]);
        }

        return Sum;
    }

    FSymmetricMatrix3 CalculatePointMassInertia(
        const TArray<FTAStructureNode>& Nodes,
        const TArray<int32>& NodeIndices,
        const FVector3d& CenterOfMassLocalM)
    {
        FSymmetricMatrix3 I;

        for (const int32 NodeIndex : NodeIndices)
        {
            const FTAStructureNode& Node =
                Nodes[NodeIndex];

            const double MassKg =
                GetNodeMassKg(Node);

            if (MassKg <= 0.0)
            {
                continue;
            }

            const FVector3d R =
                Node.PositionM
                - CenterOfMassLocalM;

            I.XX +=
                MassKg
                * (R.Y * R.Y + R.Z * R.Z);

            I.YY +=
                MassKg
                * (R.X * R.X + R.Z * R.Z);

            I.ZZ +=
                MassKg
                * (R.X * R.X + R.Y * R.Y);

            I.XY -=
                MassKg * R.X * R.Y;

            I.XZ -=
                MassKg * R.X * R.Z;

            I.YZ -=
                MassKg * R.Y * R.Z;
        }

        return I;
    }
}

bool TAStructureImpactDistributor::DistributeImpactAsInternalDeformation(
    const FTAStructureImpactConfig& Config,
    const FTAStructureImpactInput& Input,
    TArray<FTAStructureNode>& InOutNodes,
    FTAStructureImpactScratch& InOutScratch,
    FTAStructureImpactOutput& OutOutput)
{
    OutOutput =
        FTAStructureImpactOutput{};

    InOutScratch.ResetKeepCapacity();

    const double RadiusM =
        FMath::Max(
            0.0,
            Config.DistributionRadiusM);

    const double ImpulseMagnitudeNs =
        Input.CollisionImpulseLocalNs.Length();

    const double DeformationFraction =
        FMath::Clamp(
            Config.DeformationImpulseFraction01,
            0.0,
            1.0);

    OutOutput.RequestedDeformationImpulseNs =
        ImpulseMagnitudeNs
        * DeformationFraction;

    if (RadiusM <= UE_DOUBLE_SMALL_NUMBER ||
        ImpulseMagnitudeNs <= UE_DOUBLE_SMALL_NUMBER ||
        DeformationFraction <= 0.0 ||
        InOutNodes.Num() <= 0)
    {
        return false;
    }

    const double FalloffExponent =
        FMath::Max(
            0.01,
            Config.DistanceFalloffExponent);

    double WeightSum = 0.0;
    double AffectedMassKg = 0.0;
    FVector3d AffectedMassPositionSum =
        FVector3d::ZeroVector;

    for (int32 NodeIndex = 0;
         NodeIndex < InOutNodes.Num();
         ++NodeIndex)
    {
        const FTAStructureNode& Node =
            InOutNodes[NodeIndex];

        const double MassKg =
            GetNodeMassKg(Node);

        if (MassKg <= 0.0)
        {
            continue;
        }

        const double DistanceM =
            (Node.PositionM
                - Input.ContactPointLocalM).Length();

        if (DistanceM > RadiusM)
        {
            continue;
        }

        const double Normalized =
            FMath::Clamp(
                1.0
                - DistanceM / RadiusM,
                0.0,
                1.0);

        const double Weight =
            FMath::Pow(
                Normalized,
                FalloffExponent);

        if (Weight <= 1.0e-12)
        {
            continue;
        }

        InOutScratch.NodeIndices.Add(
            NodeIndex);

        InOutScratch.Weights.Add(
            Weight);

        InOutScratch.InternalImpulsesNs.Add(
            FVector3d::ZeroVector);

        WeightSum +=
            Weight;

        AffectedMassKg +=
            MassKg;

        AffectedMassPositionSum +=
            Node.PositionM
            * MassKg;
    }

    OutOutput.AffectedNodeCount =
        InOutScratch.NodeIndices.Num();

    if (OutOutput.AffectedNodeCount < 3 ||
        WeightSum <= UE_DOUBLE_SMALL_NUMBER ||
        AffectedMassKg <= UE_DOUBLE_SMALL_NUMBER)
    {
        return false;
    }

    const FVector3d AffectedCenterOfMassLocalM =
        AffectedMassPositionSum
        / AffectedMassKg;

    const FVector3d RequestedInternalImpulseNs =
        Input.CollisionImpulseLocalNs
        * DeformationFraction;

    for (int32 ScratchIndex = 0;
         ScratchIndex < InOutScratch.NodeIndices.Num();
         ++ScratchIndex)
    {
        InOutScratch.InternalImpulsesNs[ScratchIndex] =
            RequestedInternalImpulseNs
            * (InOutScratch.Weights[ScratchIndex]
                / WeightSum);
    }

    // Remove rigid translation. The chassis solver already owns the external
    // collision impulse, so the structure receives deformation modes only.
    const FVector3d CandidateLinearImpulseNs =
        CalculateLinearImpulseSum(
            InOutScratch.InternalImpulsesNs);

    for (int32 ScratchIndex = 0;
         ScratchIndex < InOutScratch.NodeIndices.Num();
         ++ScratchIndex)
    {
        const int32 NodeIndex =
            InOutScratch.NodeIndices[ScratchIndex];

        const double NodeMassKg =
            GetNodeMassKg(
                InOutNodes[NodeIndex]);

        InOutScratch.InternalImpulsesNs[ScratchIndex] -=
            CandidateLinearImpulseNs
            * (NodeMassKg / AffectedMassKg);
    }

    // Remove rigid rotation about the affected-node mass centroid.
    const FVector3d CandidateAngularImpulseNms =
        CalculateAngularImpulseSum(
            InOutNodes,
            InOutScratch.NodeIndices,
            InOutScratch.InternalImpulsesNs,
            AffectedCenterOfMassLocalM);

    const FSymmetricMatrix3 Inertia =
        CalculatePointMassInertia(
            InOutNodes,
            InOutScratch.NodeIndices,
            AffectedCenterOfMassLocalM);

    FVector3d RigidOmegaImpulseEquivalent =
        FVector3d::ZeroVector;

    if (!SolveSymmetric3x3(
            Inertia,
            CandidateAngularImpulseNms,
            RigidOmegaImpulseEquivalent))
    {
        return false;
    }

    for (int32 ScratchIndex = 0;
         ScratchIndex < InOutScratch.NodeIndices.Num();
         ++ScratchIndex)
    {
        const int32 NodeIndex =
            InOutScratch.NodeIndices[ScratchIndex];

        const FTAStructureNode& Node =
            InOutNodes[NodeIndex];

        const double NodeMassKg =
            GetNodeMassKg(Node);

        const FVector3d R =
            Node.PositionM
            - AffectedCenterOfMassLocalM;

        const FVector3d RigidRotationImpulseNs =
            NodeMassKg
            * FVector3d::CrossProduct(
                RigidOmegaImpulseEquivalent,
                R);

        InOutScratch.InternalImpulsesNs[ScratchIndex] -=
            RigidRotationImpulseNs;
    }

    // Uniform scaling preserves zero linear/angular rigid modes.
    double MaxDeltaVelocityMps = 0.0;

    for (int32 ScratchIndex = 0;
         ScratchIndex < InOutScratch.NodeIndices.Num();
         ++ScratchIndex)
    {
        const FTAStructureNode& Node =
            InOutNodes[
                InOutScratch.NodeIndices[ScratchIndex]];

        const double DeltaVelocityMps =
            InOutScratch.InternalImpulsesNs[ScratchIndex].Length()
            * FMath::Max(
                0.0,
                Node.InverseMassPerKg);

        MaxDeltaVelocityMps =
            FMath::Max(
                MaxDeltaVelocityMps,
                DeltaVelocityMps);
    }

    const double VelocityLimitMps =
        FMath::Max(
            0.0,
            Config.MaxNodeDeltaVelocityMps);

    double UniformScale = 1.0;

    if (VelocityLimitMps > 0.0 &&
        MaxDeltaVelocityMps > VelocityLimitMps)
    {
        UniformScale =
            VelocityLimitMps
            / MaxDeltaVelocityMps;
    }

    if (UniformScale < 1.0)
    {
        for (FVector3d& Impulse :
             InOutScratch.InternalImpulsesNs)
        {
            Impulse *=
                UniformScale;
        }
    }

    double InjectedEnergyJ = 0.0;
    double AppliedImpulseL1Ns = 0.0;

    for (int32 ScratchIndex = 0;
         ScratchIndex < InOutScratch.NodeIndices.Num();
         ++ScratchIndex)
    {
        const int32 NodeIndex =
            InOutScratch.NodeIndices[ScratchIndex];

        FTAStructureNode& Node =
            InOutNodes[NodeIndex];

        const FVector3d InternalImpulseNs =
            InOutScratch.InternalImpulsesNs[ScratchIndex];

        const FVector3d DeltaVelocityMps =
            InternalImpulseNs
            * FMath::Max(
                0.0,
                Node.InverseMassPerKg);

        Node.VelocityMps +=
            DeltaVelocityMps;

        AppliedImpulseL1Ns +=
            InternalImpulseNs.Length();

        const double NodeMassKg =
            GetNodeMassKg(Node);

        InjectedEnergyJ +=
            0.5
            * NodeMassKg
            * DeltaVelocityMps.SquaredLength();
    }

    OutOutput.ResidualLinearImpulseNs =
        CalculateLinearImpulseSum(
            InOutScratch.InternalImpulsesNs);

    OutOutput.ResidualAngularImpulseNms =
        CalculateAngularImpulseSum(
            InOutNodes,
            InOutScratch.NodeIndices,
            InOutScratch.InternalImpulsesNs,
            AffectedCenterOfMassLocalM);

    OutOutput.AppliedInternalImpulseL1Ns =
        AppliedImpulseL1Ns;

    OutOutput.InjectedDeformationKineticEnergyJ =
        InjectedEnergyJ;

    OutOutput.bApplied =
        AppliedImpulseL1Ns > UE_DOUBLE_SMALL_NUMBER;

    return OutOutput.bApplied;
}
