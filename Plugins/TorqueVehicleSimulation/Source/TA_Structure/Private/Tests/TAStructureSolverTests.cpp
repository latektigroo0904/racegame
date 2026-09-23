#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TAStructureSolver.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAStructureSolverConstraintTest,
    "TorqueAtlas.Structure.Solver.DistanceConstraintCorrection",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAStructureSolverConstraintTest::RunTest(const FString& Parameters)
{
    TArray<FTAStructureNode> Nodes;
    Nodes.SetNum(2);

    Nodes[0].PositionM = FVector3d(0.0, 0.0, 0.0);
    Nodes[0].PreviousPositionM = Nodes[0].PositionM;
    Nodes[0].bPinned = true;
    Nodes[0].InverseMassPerKg = 0.0;

    Nodes[1].PositionM = FVector3d(1.20, 0.0, 0.0);
    Nodes[1].PreviousPositionM = Nodes[1].PositionM;
    Nodes[1].InverseMassPerKg = 1.0;

    TArray<FTADistanceConstraint> Constraints;
    Constraints.SetNum(1);

    Constraints[0].NodeA = 0;
    Constraints[0].NodeB = 1;
    Constraints[0].RestLengthM = 1.0;
    Constraints[0].Compliance = 0.0;
    Constraints[0].YieldStrain = 1.0;
    Constraints[0].FractureStrain = 2.0;

    FTAStructureSolverConfig Config;
    Config.GravityMps2 = FVector3d::ZeroVector;
    Config.ConstraintIterations = 8;
    Config.MaxPositionCorrectionM = 1.0;

    const double ErrorBefore =
        FMath::Abs((Nodes[1].PositionM - Nodes[0].PositionM).Length() - 1.0);

    TAStructureSolver::SolveDistanceConstraints(
        Config,
        1.0 / 240.0,
        Nodes,
        Constraints);

    const double ErrorAfter =
        FMath::Abs((Nodes[1].PositionM - Nodes[0].PositionM).Length() - 1.0);

    TestTrue(
        TEXT("Distance constraint reduces geometric error"),
        ErrorAfter < ErrorBefore);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAStructureSolverPlasticityTest,
    "TorqueAtlas.Structure.Solver.PlasticityAndFracture",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAStructureSolverPlasticityTest::RunTest(const FString& Parameters)
{
    TArray<FTAStructureNode> Nodes;
    Nodes.SetNum(2);

    Nodes[0].PositionM = FVector3d(0.0, 0.0, 0.0);
    Nodes[1].PositionM = FVector3d(1.15, 0.0, 0.0);

    TArray<FTADistanceConstraint> Constraints;
    Constraints.SetNum(1);

    FTADistanceConstraint& Constraint = Constraints[0];
    Constraint.NodeA = 0;
    Constraint.NodeB = 1;
    Constraint.RestLengthM = 1.0;
    Constraint.YieldStrain = 0.05;
    Constraint.FractureStrain = 0.30;
    Constraint.PlasticFlowRate01 = 1.0;

    FTAStructureSolverConfig Config;
    Config.MaxPlasticRestChangeFractionPerStep = 0.10;

    const double RestBefore = Constraint.RestLengthM;

    TAStructureSolver::UpdatePlasticityAndFracture(
        Config,
        Nodes,
        Constraints);

    TestFalse(
        TEXT("Moderate over-yield strain does not immediately fracture"),
        Constraint.bBroken);

    TestTrue(
        TEXT("Over-yield strain permanently changes rest length"),
        Constraint.RestLengthM > RestBefore);

    Nodes[1].PositionM = FVector3d(1.60, 0.0, 0.0);

    TAStructureSolver::UpdatePlasticityAndFracture(
        Config,
        Nodes,
        Constraints);

    TestTrue(
        TEXT("Extreme strain fractures constraint"),
        Constraint.bBroken);

    return true;
}

#endif
