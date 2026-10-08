#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TABrakeHydraulics.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTABrakeHydraulicsPressureBuildTest,
    "TorqueAtlas.Vehicle.Brakes.Hydraulics.PressureBuildAndRelease",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTABrakeHydraulicsPressureBuildTest::RunTest(
    const FString& Parameters)
{
    FTABrakeHydraulicConfig Config;
    Config.PressureRiseRatePaPerSec = 1.0e6;
    Config.PressureReleaseRatePaPerSec = 2.0e6;

    FTABrakeHydraulicState State;
    TABrakeHydraulics::InitializeState(Config, State);

    FTABrakeHydraulicInput Input;
    Input.Pedal01 = 1.0;
    Input.DeltaTimeSeconds = 0.1;

    FTABrakeHydraulicOutput Output;

    TestTrue(
        TEXT("Pressure build step succeeds"),
        TABrakeHydraulics::Step(
            Config,
            Input,
            State,
            Output));

    TestTrue(
        TEXT("Front pressure rise is rate-limited"),
        FMath::IsNearlyEqual(
            State.FrontCircuitPressurePa,
            100000.0,
            1.0e-6));

    TestTrue(
        TEXT("Rear pressure rise is rate-limited equally before ratio target is reached"),
        FMath::IsNearlyEqual(
            State.RearCircuitPressurePa,
            100000.0,
            1.0e-6));

    Input.Pedal01 = 0.0;

    TestTrue(
        TEXT("Pressure release step succeeds"),
        TABrakeHydraulics::Step(
            Config,
            Input,
            State,
            Output));

    TestTrue(
        TEXT("Pressure releases toward zero"),
        State.FrontCircuitPressurePa
            < 100000.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTABrakeHydraulicsBiasTorqueTest,
    "TorqueAtlas.Vehicle.Brakes.Hydraulics.BiasAndTorqueConversion",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTABrakeHydraulicsBiasTorqueTest::RunTest(
    const FString& Parameters)
{
    FTABrakeHydraulicConfig Config;
    Config.PressureRiseRatePaPerSec = 1.0e12;
    Config.PressureReleaseRatePaPerSec = 1.0e12;
    Config.FrontPressureRatio01 = 1.0;
    Config.RearPressureRatio01 = 0.5;

    FTABrakeHydraulicState State;
    TABrakeHydraulics::InitializeState(Config, State);

    FTABrakeHydraulicInput Input;
    Input.Pedal01 = 1.0;
    Input.DeltaTimeSeconds = 1.0 / 240.0;

    FTABrakeHydraulicOutput Output;

    TestTrue(
        TEXT("Bias step succeeds"),
        TABrakeHydraulics::Step(
            Config,
            Input,
            State,
            Output));

    TestTrue(
        TEXT("Rear target pressure is half front target pressure"),
        FMath::IsNearlyEqual(
            Output.RearTargetPressurePa,
            0.5 * Output.FrontTargetPressurePa,
            1.0e-6));

    TestTrue(
        TEXT("Front hydraulic brake torque is positive"),
        Output.FrontCornerRawBrakeTorqueNm > 0.0);

    TestTrue(
        TEXT("Rear hydraulic brake torque is positive"),
        Output.RearCornerRawBrakeTorqueNm > 0.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTABrakeHydraulicsCircuitHealthTest,
    "TorqueAtlas.Vehicle.Brakes.Hydraulics.CircuitHealthReducesPressure",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTABrakeHydraulicsCircuitHealthTest::RunTest(
    const FString& Parameters)
{
    FTABrakeHydraulicConfig Config;
    Config.PressureRiseRatePaPerSec = 1.0e12;

    FTABrakeHydraulicState State;
    TABrakeHydraulics::InitializeState(Config, State);
    State.FrontCircuitHealth01 = 0.25;

    FTABrakeHydraulicInput Input;
    Input.Pedal01 = 1.0;

    FTABrakeHydraulicOutput Output;

    TestTrue(
        TEXT("Damaged circuit step succeeds"),
        TABrakeHydraulics::Step(
            Config,
            Input,
            State,
            Output));

    TestTrue(
        TEXT("Front effective pressure is quartered by circuit health"),
        FMath::IsNearlyEqual(
            Output.FrontEffectivePressurePa,
            0.25 * State.FrontCircuitPressurePa,
            1.0e-6));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTABrakeHydraulicsBoilingTest,
    "TorqueAtlas.Vehicle.Brakes.Hydraulics.BoilingReducesPressureTransfer",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTABrakeHydraulicsBoilingTest::RunTest(
    const FString& Parameters)
{
    FTABrakeHydraulicConfig Config;
    Config.FluidDryBoilingPointC = 250.0;
    Config.FluidWetBoilingPointC = 150.0;
    Config.FluidWaterContamination01 = 0.50;
    Config.VaporTransitionRangeC = 20.0;
    Config.MaxPressureLossAtFullVapor01 = 0.80;
    Config.PressureRiseRatePaPerSec = 1.0e12;

    const double BoilingPointC =
        TABrakeHydraulics::CalculateBoilingPointC(Config);

    TestTrue(
        TEXT("Boiling point interpolates between dry and wet values"),
        FMath::IsNearlyEqual(
            BoilingPointC,
            200.0,
            1.0e-9));

    FTABrakeHydraulicState State;
    TABrakeHydraulics::InitializeState(Config, State);
    State.FluidTemperatureC = 220.0;

    FTABrakeHydraulicInput Input;
    Input.Pedal01 = 1.0;

    FTABrakeHydraulicOutput Output;

    TestTrue(
        TEXT("Boiling-state brake step succeeds"),
        TABrakeHydraulics::Step(
            Config,
            Input,
            State,
            Output));

    TestTrue(
        TEXT("Full transition-range superheat reaches full vapor fraction"),
        FMath::IsNearlyEqual(
            Output.VaporFraction01,
            1.0,
            1.0e-9));

    TestTrue(
        TEXT("Full vapor fraction reduces pressure transfer to configured floor"),
        FMath::IsNearlyEqual(
            Output.FluidPressureTransfer01,
            0.20,
            1.0e-9));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTABrakeHydraulicsFluidThermalTest,
    "TorqueAtlas.Vehicle.Brakes.Hydraulics.FluidHeatingAndCooling",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTABrakeHydraulicsFluidThermalTest::RunTest(
    const FString& Parameters)
{
    FTABrakeHydraulicConfig Config;
    Config.FluidThermalMassJPerC = 1000.0;
    Config.FluidCoolingWPerC = 10.0;
    Config.FluidAmbientTemperatureC = 20.0;

    FTABrakeHydraulicState State;
    TABrakeHydraulics::InitializeState(Config, State);

    TestTrue(
        TEXT("Fluid heating update succeeds"),
        TABrakeHydraulics::UpdateFluidTemperature(
            Config,
            1000.0,
            10.0,
            State));

    TestTrue(
        TEXT("Conducted heat raises fluid temperature"),
        State.FluidTemperatureC > 20.0);

    const double HeatedTemperatureC =
        State.FluidTemperatureC;

    TestTrue(
        TEXT("Fluid cooling update succeeds"),
        TABrakeHydraulics::UpdateFluidTemperature(
            Config,
            0.0,
            10.0,
            State));

    TestTrue(
        TEXT("Cooling lowers hot fluid temperature"),
        State.FluidTemperatureC < HeatedTemperatureC);

    TestTrue(
        TEXT("Cooling never goes below ambient"),
        State.FluidTemperatureC
            >= Config.FluidAmbientTemperatureC);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTABrakeHydraulicsValidationTest,
    "TorqueAtlas.Vehicle.Brakes.Hydraulics.ConfigValidation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTABrakeHydraulicsValidationTest::RunTest(
    const FString& Parameters)
{
    FTABrakeHydraulicConfig Config;

    TestTrue(
        TEXT("Default hydraulic config validates"),
        TABrakeHydraulics::ValidateConfig(Config));

    Config.MasterCylinderAreaM2 = 0.0;

    TestFalse(
        TEXT("Zero master cylinder area is rejected"),
        TABrakeHydraulics::ValidateConfig(Config));

    Config = FTABrakeHydraulicConfig{};
    Config.FluidWetBoilingPointC =
        Config.FluidDryBoilingPointC;

    TestFalse(
        TEXT("Non-decreasing wet boiling point is rejected"),
        TABrakeHydraulics::ValidateConfig(Config));

    return true;
}

#endif
