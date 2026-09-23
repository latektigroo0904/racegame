#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TADoubleWishboneSolver.h"

namespace
{
    FTADoubleWishboneSolverConfig MakeFrontRightGeometry()
    {
        FTADoubleWishboneSolverConfig Config;
        FTADoubleWishboneHardpoints& H = Config.Hardpoints;

        H.UpperInnerA = FVector3d(1.48, 0.38, -0.22);
        H.UpperInnerB = FVector3d(1.14, 0.38, -0.22);

        H.LowerInnerA = FVector3d(1.50, 0.35, -0.48);
        H.LowerInnerB = FVector3d(1.12, 0.35, -0.48);

        H.TieRodInner = FVector3d(1.12, 0.35, -0.38);

        H.DamperChassis = FVector3d(1.31, 0.40, -0.05);
        H.DamperLowerArmReference = FVector3d(1.31, 0.52, -0.43);

        H.UpperBallJointReference = FVector3d(1.31, 0.70, -0.25);
        H.LowerBallJointReference = FVector3d(1.31, 0.73, -0.50);
        H.TieRodOuterReference = FVector3d(1.12, 0.71, -0.39);
        H.WheelCenterReference = FVector3d(1.31, 0.775, -0.45);

        H.WheelForwardReference = FVector3d(1.0, 0.0, 0.0);
        H.WheelUpReference = FVector3d(0.0, 0.0, 1.0);

        H.SteeringRackAxisLocal = FVector3d(0.0, 1.0, 0.0);
        H.SideSign = 1.0;

        Config.MinTravelM = -0.07;
        Config.MaxTravelM = 0.09;
        Config.MaxIterations = 80;
        Config.PositionToleranceM = 0.0005;

        return Config;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTADoubleWishboneReferenceTest,
    "TorqueAtlas.Suspension.DoubleWishbone.ReferenceGeometry",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTADoubleWishboneReferenceTest::RunTest(const FString& Parameters)
{
    const FTADoubleWishboneSolverConfig Config =
        MakeFrontRightGeometry();

    TestTrue(
        TEXT("Reference hardpoints validate"),
        TADoubleWishboneSolver::ValidateConfig(Config));

    FTADoubleWishboneState State;
    FTADoubleWishboneSolveOutput Output;
    FTADoubleWishboneSolveInput Input;

    TestTrue(
        TEXT("Reference geometry converges"),
        TADoubleWishboneSolver::Solve(
            Config,
            Input,
            State,
            Output));

    TestTrue(
        TEXT("Reference wheel center is reconstructed"),
        Output.WheelCenterLocalM.Equals(
            Config.Hardpoints.WheelCenterReference,
            0.001));

    TestTrue(
        TEXT("Reference toe remains near zero"),
        FMath::Abs(Output.ToeRad) < FMath::DegreesToRadians(0.1));

    TestTrue(
        TEXT("Reference camber remains near zero"),
        FMath::Abs(Output.CamberRad) < FMath::DegreesToRadians(0.1));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTADoubleWishboneTravelSteeringTest,
    "TorqueAtlas.Suspension.DoubleWishbone.TravelAndSteering",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTADoubleWishboneTravelSteeringTest::RunTest(const FString& Parameters)
{
    const FTADoubleWishboneSolverConfig Config =
        MakeFrontRightGeometry();

    FTADoubleWishboneState State;
    FTADoubleWishboneSolveOutput ReferenceOutput;
    FTADoubleWishboneSolveInput ReferenceInput;

    TestTrue(
        TEXT("Reference solve succeeds"),
        TADoubleWishboneSolver::Solve(
            Config,
            ReferenceInput,
            State,
            ReferenceOutput));

    FTADoubleWishboneSolveInput BumpInput;
    BumpInput.TravelM = 0.05;

    FTADoubleWishboneSolveOutput BumpOutput;

    TestTrue(
        TEXT("Bump travel solve succeeds"),
        TADoubleWishboneSolver::Solve(
            Config,
            BumpInput,
            State,
            BumpOutput));

    TestTrue(
        TEXT("Wheel center follows requested bump travel"),
        FMath::IsNearlyEqual(
            BumpOutput.WheelCenterLocalM.Z,
            Config.Hardpoints.WheelCenterReference.Z + 0.05,
            0.001));

    FTADoubleWishboneSolveInput SteerInput;
    SteerInput.RackDisplacementM = 0.01;

    FTADoubleWishboneSolveOutput SteerOutput;

    State.bHasValidPreviousSolution = false;

    TestTrue(
        TEXT("Rack displacement solve succeeds"),
        TADoubleWishboneSolver::Solve(
            Config,
            SteerInput,
            State,
            SteerOutput));

    TestTrue(
        TEXT("Tie-rod geometry changes toe"),
        FMath::Abs(SteerOutput.ToeRad - ReferenceOutput.ToeRad)
            > FMath::DegreesToRadians(0.5));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTADoubleWishboneDamageTest,
    "TorqueAtlas.Suspension.DoubleWishbone.DamagedPickupChangesAlignment",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTADoubleWishboneDamageTest::RunTest(const FString& Parameters)
{
    const FTADoubleWishboneSolverConfig Config =
        MakeFrontRightGeometry();

    FTADoubleWishboneState ReferenceState;
    FTADoubleWishboneSolveOutput ReferenceOutput;
    FTADoubleWishboneSolveInput ReferenceInput;

    TestTrue(
        TEXT("Reference solve succeeds"),
        TADoubleWishboneSolver::Solve(
            Config,
            ReferenceInput,
            ReferenceState,
            ReferenceOutput));

    FTADoubleWishboneState DamagedState;
    FTADoubleWishboneSolveOutput DamagedOutput;
    FTADoubleWishboneSolveInput DamagedInput;

    DamagedInput.Damage.UpperInnerA = FVector3d(0.0, -0.02, 0.0);
    DamagedInput.Damage.UpperInnerB = FVector3d(0.0, -0.02, 0.0);

    TestTrue(
        TEXT("Damaged pickup geometry still converges"),
        TADoubleWishboneSolver::Solve(
            Config,
            DamagedInput,
            DamagedState,
            DamagedOutput));

    TestTrue(
        TEXT("Pickup displacement changes camber geometrically"),
        FMath::Abs(DamagedOutput.CamberRad - ReferenceOutput.CamberRad)
            > FMath::DegreesToRadians(1.0));

    TestTrue(
        TEXT("Damage result remains within solver tolerance"),
        DamagedOutput.MaxConstraintResidualM
            <= Config.PositionToleranceM);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTADoubleWishboneInvalidGeometryTest,
    "TorqueAtlas.Suspension.DoubleWishbone.RejectsDegenerateGeometry",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTADoubleWishboneInvalidGeometryTest::RunTest(const FString& Parameters)
{
    FTADoubleWishboneSolverConfig Config =
        MakeFrontRightGeometry();

    Config.Hardpoints.UpperInnerB =
        Config.Hardpoints.UpperInnerA;

    TestFalse(
        TEXT("Degenerate upper-arm axis is rejected"),
        TADoubleWishboneSolver::ValidateConfig(Config));

    return true;
}

#endif
