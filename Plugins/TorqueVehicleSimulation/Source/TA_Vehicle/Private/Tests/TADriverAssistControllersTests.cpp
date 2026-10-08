#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TADriverAssistControllers.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTATractionControlReductionTest,
    "TorqueAtlas.Vehicle.Assists.TCS.ExcessSlipReducesTorqueAuthority",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTATractionControlReductionTest::RunTest(
    const FString& Parameters)
{
    FTATractionControlConfig Config;

    FTATractionControlState State;
    TADriverAssistControllers::InitializeTractionState(State);

    FTATractionControlInput Input;
    Input.Throttle01 = 1.0;
    Input.VehicleSpeedMps = 15.0;
    Input.MaxDrivenSlipRatio = 0.25;
    Input.DeltaTimeSeconds = 0.05;

    FTATractionControlOutput Output;

    TestTrue(
        TEXT("TCS reduction step succeeds"),
        TADriverAssistControllers::StepTractionControl(
            Config,
            Input,
            State,
            Output));

    TestTrue(
        TEXT("TCS reports torque reduction"),
        Output.bReducingTorque);

    TestTrue(
        TEXT("TCS reduces engine torque authority"),
        Output.EngineTorqueAuthority01 < 1.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTATractionControlRecoveryTest,
    "TorqueAtlas.Vehicle.Assists.TCS.RecoveredSlipRestoresTorque",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTATractionControlRecoveryTest::RunTest(
    const FString& Parameters)
{
    FTATractionControlConfig Config;

    FTATractionControlState State;
    State.EngineTorqueAuthority01 = 0.40;

    FTATractionControlInput Input;
    Input.Throttle01 = 0.8;
    Input.VehicleSpeedMps = 15.0;
    Input.MaxDrivenSlipRatio = 0.02;
    Input.DeltaTimeSeconds = 0.1;

    FTATractionControlOutput Output;

    TestTrue(
        TEXT("TCS recovery step succeeds"),
        TADriverAssistControllers::StepTractionControl(
            Config,
            Input,
            State,
            Output));

    TestTrue(
        TEXT("Recovered slip raises torque authority"),
        Output.EngineTorqueAuthority01 > 0.40);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAStabilityReferenceTest,
    "TorqueAtlas.Vehicle.Assists.ESC.ReferenceYawSign",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAStabilityReferenceTest::RunTest(
    const FString& Parameters)
{
    FTAStabilityControlConfig Config;

    FTAStabilityControlInput Input;
    Input.RoadWheelSteerAngleRad = 0.10;
    Input.VehicleSpeedMps = 20.0;
    Input.MeasuredYawRateRadPerSec = 0.0;

    FTAStabilityControlOutput Output;

    TestTrue(
        TEXT("ESC reference calculation succeeds"),
        TADriverAssistControllers::CalculateStabilityRequest(
            Config,
            Input,
            Output));

    TestTrue(
        TEXT("Positive steering creates positive reference yaw rate"),
        Output.ReferenceYawRateRadPerSec > 0.0);

    TestTrue(
        TEXT("Under-yaw creates positive corrective yaw request"),
        Output.DesiredCorrectiveYawMomentNm > 0.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAStabilityOverYawTest,
    "TorqueAtlas.Vehicle.Assists.ESC.OverYawRequestsOpposingMoment",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAStabilityOverYawTest::RunTest(
    const FString& Parameters)
{
    FTAStabilityControlConfig Config;

    FTAStabilityControlInput Input;
    Input.RoadWheelSteerAngleRad = 0.08;
    Input.VehicleSpeedMps = 20.0;
    Input.MeasuredYawRateRadPerSec = 1.0;

    FTAStabilityControlOutput Output;

    TestTrue(
        TEXT("ESC over-yaw calculation succeeds"),
        TADriverAssistControllers::CalculateStabilityRequest(
            Config,
            Input,
            Output));

    TestTrue(
        TEXT("Over-yaw creates opposing corrective yaw request"),
        Output.DesiredCorrectiveYawMomentNm < 0.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAStabilityLowSpeedTest,
    "TorqueAtlas.Vehicle.Assists.ESC.LowSpeedIsTransparent",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAStabilityLowSpeedTest::RunTest(
    const FString& Parameters)
{
    FTAStabilityControlConfig Config;

    FTAStabilityControlInput Input;
    Input.RoadWheelSteerAngleRad = 0.5;
    Input.VehicleSpeedMps = 1.0;
    Input.MeasuredYawRateRadPerSec = 1.0;

    FTAStabilityControlOutput Output;

    TestTrue(
        TEXT("Low-speed ESC calculation succeeds"),
        TADriverAssistControllers::CalculateStabilityRequest(
            Config,
            Input,
            Output));

    TestEqual(
        TEXT("Low-speed ESC makes no yaw-moment request"),
        Output.DesiredCorrectiveYawMomentNm,
        0.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAAssistConfigValidationTest,
    "TorqueAtlas.Vehicle.Assists.ConfigValidation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAAssistConfigValidationTest::RunTest(
    const FString& Parameters)
{
    FTATractionControlConfig Tcs;
    FTAStabilityControlConfig Esc;

    TestTrue(
        TEXT("Default TCS config validates"),
        TADriverAssistControllers::ValidateTractionConfig(Tcs));

    TestTrue(
        TEXT("Default ESC config validates"),
        TADriverAssistControllers::ValidateStabilityConfig(Esc));

    Tcs.RecoverSlipRatio = Tcs.EnterSlipRatio;

    TestFalse(
        TEXT("TCS without hysteresis is rejected"),
        TADriverAssistControllers::ValidateTractionConfig(Tcs));

    Esc.WheelbaseM = 0.0;

    TestFalse(
        TEXT("ESC zero wheelbase is rejected"),
        TADriverAssistControllers::ValidateStabilityConfig(Esc));

    return true;
}

#endif
