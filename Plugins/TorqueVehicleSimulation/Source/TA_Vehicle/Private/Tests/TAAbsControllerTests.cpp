#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TAAbsController.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAAbsInactiveTransparencyTest,
    "TorqueAtlas.Vehicle.Brakes.ABS.InactiveRecoversPressure",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAAbsInactiveTransparencyTest::RunTest(
    const FString& Parameters)
{
    FTAAbsConfig Config;

    FTAAbsState State;
    TAAbsController::InitializeState(State);
    State.PressureModulation01 = 0.50;

    FTAAbsInput Input;
    Input.BrakeRequest01 = 0.0;
    Input.VehicleSpeedMps = 20.0;
    Input.DeltaTimeSeconds = 0.1;

    FTAAbsOutput Output;

    TestTrue(
        TEXT("Inactive ABS step succeeds"),
        TAAbsController::Step(
            Config,
            Input,
            State,
            Output));

    TestTrue(
        TEXT("ABS is inactive without brake request"),
        Output.Mode == ETAAbsMode::Inactive);

    TestTrue(
        TEXT("Inactive ABS recovers pressure authority toward one"),
        Output.PressureModulation01 > 0.50);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAAbsSlipReleaseTest,
    "TorqueAtlas.Vehicle.Brakes.ABS.LockSlipTriggersRelease",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAAbsSlipReleaseTest::RunTest(
    const FString& Parameters)
{
    FTAAbsConfig Config;

    FTAAbsState State;
    TAAbsController::InitializeState(State);

    FTAAbsInput Input;
    Input.BrakeRequest01 = 1.0;
    Input.VehicleSpeedMps = 25.0;
    Input.SlipRatio = -0.25;
    Input.DeltaTimeSeconds = 0.01;

    FTAAbsOutput Output;

    TestTrue(
        TEXT("ABS release step succeeds"),
        TAAbsController::Step(
            Config,
            Input,
            State,
            Output));

    TestTrue(
        TEXT("Excess braking slip triggers release"),
        Output.Mode == ETAAbsMode::Release);

    TestTrue(
        TEXT("Release reduces pressure modulation"),
        Output.PressureModulation01 < 1.0);

    TestTrue(
        TEXT("Slip trigger is reported"),
        Output.bReleaseTriggeredBySlip);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAAbsWheelDecelReleaseTest,
    "TorqueAtlas.Vehicle.Brakes.ABS.WheelDecelerationCanTriggerRelease",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAAbsWheelDecelReleaseTest::RunTest(
    const FString& Parameters)
{
    FTAAbsConfig Config;

    FTAAbsState State;
    TAAbsController::InitializeState(State);

    FTAAbsInput Input;
    Input.BrakeRequest01 = 0.8;
    Input.VehicleSpeedMps = 20.0;
    Input.SlipRatio = -0.05;
    Input.WheelAngularDecelerationRadPerSec2 = 220.0;

    FTAAbsOutput Output;

    TestTrue(
        TEXT("Wheel-deceleration ABS step succeeds"),
        TAAbsController::Step(
            Config,
            Input,
            State,
            Output));

    TestTrue(
        TEXT("Wheel deceleration triggers release"),
        Output.Mode == ETAAbsMode::Release);

    TestTrue(
        TEXT("Wheel-deceleration trigger is reported"),
        Output.bReleaseTriggeredByWheelDeceleration);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAAbsHoldBuildHysteresisTest,
    "TorqueAtlas.Vehicle.Brakes.ABS.HoldAndBuildHysteresis",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAAbsHoldBuildHysteresisTest::RunTest(
    const FString& Parameters)
{
    FTAAbsConfig Config;

    FTAAbsState State;
    TAAbsController::InitializeState(State);
    State.PressureModulation01 = 0.40;

    FTAAbsInput Input;
    Input.BrakeRequest01 = 1.0;
    Input.VehicleSpeedMps = 20.0;

    FTAAbsOutput Output;

    Input.SlipRatio = -0.14;

    TestTrue(
        TEXT("Hold-region step succeeds"),
        TAAbsController::Step(
            Config,
            Input,
            State,
            Output));

    TestTrue(
        TEXT("Slip between thresholds holds pressure"),
        Output.Mode == ETAAbsMode::Hold);

    const double HeldPressure01 =
        Output.PressureModulation01;

    Input.SlipRatio = -0.05;

    TestTrue(
        TEXT("Build-region step succeeds"),
        TAAbsController::Step(
            Config,
            Input,
            State,
            Output));

    TestTrue(
        TEXT("Recovered slip triggers build"),
        Output.Mode == ETAAbsMode::Build);

    TestTrue(
        TEXT("Build raises modulation above held value"),
        Output.PressureModulation01
            > HeldPressure01);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAAbsLowSpeedDisableTest,
    "TorqueAtlas.Vehicle.Brakes.ABS.LowSpeedDisablesModulation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAAbsLowSpeedDisableTest::RunTest(
    const FString& Parameters)
{
    FTAAbsConfig Config;

    FTAAbsState State;
    TAAbsController::InitializeState(State);

    FTAAbsInput Input;
    Input.BrakeRequest01 = 1.0;
    Input.VehicleSpeedMps = 0.5;
    Input.SlipRatio = -1.0;

    FTAAbsOutput Output;

    TestTrue(
        TEXT("Low-speed ABS step succeeds"),
        TAAbsController::Step(
            Config,
            Input,
            State,
            Output));

    TestTrue(
        TEXT("ABS is inactive below minimum speed"),
        Output.Mode == ETAAbsMode::Inactive);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAAbsValidationTest,
    "TorqueAtlas.Vehicle.Brakes.ABS.ConfigValidation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAAbsValidationTest::RunTest(
    const FString& Parameters)
{
    FTAAbsConfig Config;

    TestTrue(
        TEXT("Default ABS config validates"),
        TAAbsController::ValidateConfig(Config));

    Config.BuildSlipMagnitude =
        Config.ReleaseSlipMagnitude;

    TestFalse(
        TEXT("Missing slip hysteresis is rejected"),
        TAAbsController::ValidateConfig(Config));

    return true;
}

#endif
