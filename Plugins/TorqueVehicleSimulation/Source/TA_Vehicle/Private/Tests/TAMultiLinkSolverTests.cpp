#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TAMultiLinkSolver.h"

namespace
{
    FTAMultiLinkSolverConfig MakeRearRightGeometry()
    {
        FTAMultiLinkSolverConfig Config;

        Config.Links[0].ChassisPickupReference = FVector3d(-1.15, 0.35, -0.18);
        Config.Links[0].UprightPickupReference = FVector3d(-1.20, 0.68, -0.24);

        Config.Links[1].ChassisPickupReference = FVector3d(-1.50, 0.36, -0.20);
        Config.Links[1].UprightPickupReference = FVector3d(-1.42, 0.69, -0.25);

        Config.Links[2].ChassisPickupReference = FVector3d(-1.10, 0.34, -0.46);
        Config.Links[2].UprightPickupReference = FVector3d(-1.18, 0.72, -0.49);

        Config.Links[3].ChassisPickupReference = FVector3d(-1.53, 0.33, -0.47);
        Config.Links[3].UprightPickupReference = FVector3d(-1.45, 0.73, -0.50);

        Config.Links[4].ChassisPickupReference = FVector3d(-1.50, 0.38, -0.36);
        Config.Links[4].UprightPickupReference = FVector3d(-1.48, 0.71, -0.38);

        Config.WheelCenterReference = FVector3d(-1.31, 0.765, -0.45);

        Config.DamperChassisReference = FVector3d(-1.31, 0.40, -0.05);
        Config.DamperUprightReference = FVector3d(-1.31, 0.64, -0.40);

        Config.WheelForwardReference = FVector3d(1.0, 0.0, 0.0);
        Config.WheelUpReference = FVector3d(0.0, 0.0, 1.0);

        Config.SideSign = 1.0;
        Config.MinTravelM = -0.07;
        Config.MaxTravelM = 0.09;
        Config.MaxIterations = 180;
        Config.PositionToleranceM = 0.001;

        return Config;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAMultiLinkReferenceTest,
    "TorqueAtlas.Suspension.MultiLink.ReferenceGeometry",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAMultiLinkReferenceTest::RunTest(const FString& Parameters)
{
    const FTAMultiLinkSolverConfig Config =
        MakeRearRightGeometry();

    TestTrue(
        TEXT("Rear multi-link geometry validates"),
        TAMultiLinkSolver::ValidateConfig(Config));

    FTAMultiLinkRuntimeState State;
    FTAMultiLinkSolveOutput Output;
    FTAMultiLinkSolveInput Input;

    TestTrue(
        TEXT("Reference rear geometry converges"),
        TAMultiLinkSolver::Solve(
            Config,
            Input,
            State,
            Output));

    TestTrue(
        TEXT("Reference wheel center reconstructs"),
        Output.WheelCenterLocalM.Equals(
            Config.WheelCenterReference,
            0.001));

    TestTrue(
        TEXT("Reference toe near zero"),
        FMath::Abs(Output.ToeRad)
            < FMath::DegreesToRadians(0.1));

    TestTrue(
        TEXT("Reference camber near zero"),
        FMath::Abs(Output.CamberRad)
            < FMath::DegreesToRadians(0.1));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAMultiLinkTravelTest,
    "TorqueAtlas.Suspension.MultiLink.BumpDroopAndDamper",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAMultiLinkTravelTest::RunTest(const FString& Parameters)
{
    const FTAMultiLinkSolverConfig Config =
        MakeRearRightGeometry();

    FTAMultiLinkRuntimeState State;
    FTAMultiLinkSolveOutput ReferenceOutput;
    FTAMultiLinkSolveInput ReferenceInput;

    TestTrue(
        TEXT("Reference solve succeeds"),
        TAMultiLinkSolver::Solve(
            Config,
            ReferenceInput,
            State,
            ReferenceOutput));

    const double ReferenceDamperLength =
        ReferenceOutput.DamperLengthM;

    FTAMultiLinkSolveInput BumpInput;
    BumpInput.TravelM = 0.05;

    FTAMultiLinkSolveOutput BumpOutput;

    TestTrue(
        TEXT("Rear bump solve succeeds"),
        TAMultiLinkSolver::Solve(
            Config,
            BumpInput,
            State,
            BumpOutput));

    TestTrue(
        TEXT("Rear wheel reaches requested bump height"),
        FMath::IsNearlyEqual(
            BumpOutput.WheelCenterLocalM.Z,
            Config.WheelCenterReference.Z + 0.05,
            0.0015));

    TestTrue(
        TEXT("Damper length responds to bump"),
        FMath::Abs(
            BumpOutput.DamperLengthM
            - ReferenceDamperLength) > 0.001);

    FTAMultiLinkSolveInput DroopInput;
    DroopInput.TravelM = -0.05;

    FTAMultiLinkSolveOutput DroopOutput;

    TestTrue(
        TEXT("Rear droop solve succeeds"),
        TAMultiLinkSolver::Solve(
            Config,
            DroopInput,
            State,
            DroopOutput));

    TestTrue(
        TEXT("Rear wheel reaches requested droop height"),
        FMath::IsNearlyEqual(
            DroopOutput.WheelCenterLocalM.Z,
            Config.WheelCenterReference.Z - 0.05,
            0.0015));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAMultiLinkMirrorTest,
    "TorqueAtlas.Suspension.MultiLink.MirrorGeometry",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAMultiLinkMirrorTest::RunTest(const FString& Parameters)
{
    const FTAMultiLinkSolverConfig Right =
        MakeRearRightGeometry();

    const FTAMultiLinkSolverConfig Left =
        TAMultiLinkSolver::MirrorAcrossCenterline(
            Right);

    TestTrue(
        TEXT("Mirrored rear geometry validates"),
        TAMultiLinkSolver::ValidateConfig(Left));

    TestTrue(
        TEXT("Rear wheel center Y mirrors"),
        FMath::IsNearlyEqual(
            Left.WheelCenterReference.Y,
            -Right.WheelCenterReference.Y,
            1.0e-9));

    TestTrue(
        TEXT("Rear side sign mirrors"),
        Left.SideSign == -Right.SideSign);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAMultiLinkDamageTest,
    "TorqueAtlas.Suspension.MultiLink.PickupDamageChangesAlignment",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAMultiLinkDamageTest::RunTest(const FString& Parameters)
{
    const FTAMultiLinkSolverConfig Config =
        MakeRearRightGeometry();

    FTAMultiLinkRuntimeState ReferenceState;
    FTAMultiLinkSolveOutput ReferenceOutput;
    FTAMultiLinkSolveInput ReferenceInput;

    TestTrue(
        TEXT("Reference solve succeeds"),
        TAMultiLinkSolver::Solve(
            Config,
            ReferenceInput,
            ReferenceState,
            ReferenceOutput));

    FTAMultiLinkRuntimeState DamagedState;
    FTAMultiLinkSolveOutput DamagedOutput;
    FTAMultiLinkSolveInput DamagedInput;

    // Shift one upper/rearward chassis pickup inward.
    DamagedInput.Damage.ChassisPickupOffsets[1] =
        FVector3d(0.0, -0.02, 0.0);

    TestTrue(
        TEXT("Damaged rear geometry still converges"),
        TAMultiLinkSolver::Solve(
            Config,
            DamagedInput,
            DamagedState,
            DamagedOutput));

    const double AlignmentDelta =
        FMath::Abs(
            DamagedOutput.CamberRad
            - ReferenceOutput.CamberRad)
        + FMath::Abs(
            DamagedOutput.ToeRad
            - ReferenceOutput.ToeRad);

    TestTrue(
        TEXT("Individual pickup displacement changes rear alignment"),
        AlignmentDelta
            > FMath::DegreesToRadians(0.25));

    return true;
}

#endif
