#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TABrakeHydraulics.h"

namespace
{
    int32 Corner(const ETABrakeCornerIndex Index)
    {
        return static_cast<int32>(Index);
    }
}

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
        TABrakeHydraulics::Step(Config, Input, State, Output));

    const int32 FrontLeft =
        Corner(ETABrakeCornerIndex::FrontLeft);

    TestTrue(
        TEXT("Corner pressure rise is rate-limited"),
        FMath::IsNearlyEqual(
            State.CornerLinePressurePa[FrontLeft],
            100000.0,
            1.0e-6));

    Input.Pedal01 = 0.0;

    TestTrue(
        TEXT("Pressure release step succeeds"),
        TABrakeHydraulics::Step(Config, Input, State, Output));

    TestTrue(
        TEXT("Corner pressure releases toward zero"),
        State.CornerLinePressurePa[FrontLeft] < 100000.0);

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

    FTABrakeHydraulicOutput Output;

    TestTrue(
        TEXT("Bias step succeeds"),
        TABrakeHydraulics::Step(Config, Input, State, Output));

    const int32 FrontLeft =
        Corner(ETABrakeCornerIndex::FrontLeft);

    const int32 RearLeft =
        Corner(ETABrakeCornerIndex::RearLeft);

    TestTrue(
        TEXT("Rear target pressure is half front target pressure"),
        FMath::IsNearlyEqual(
            Output.CornerTargetPressurePa[RearLeft],
            0.5 * Output.CornerTargetPressurePa[FrontLeft],
            1.0e-6));

    TestTrue(
        TEXT("Front hydraulic torque is positive"),
        Output.CornerRawBrakeTorqueNm[FrontLeft] > 0.0);

    TestTrue(
        TEXT("Rear hydraulic torque is positive"),
        Output.CornerRawBrakeTorqueNm[RearLeft] > 0.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTABrakeHydraulicsDiagonalCircuitHealthTest,
    "TorqueAtlas.Vehicle.Brakes.Hydraulics.DiagonalCircuitHealthMapsToCorners",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTABrakeHydraulicsDiagonalCircuitHealthTest::RunTest(
    const FString& Parameters)
{
    FTABrakeHydraulicConfig Config;
    Config.CircuitTopology =
        ETABrakeCircuitTopology::Diagonal;
    Config.PressureRiseRatePaPerSec = 1.0e12;

    FTABrakeHydraulicState State;
    TABrakeHydraulics::InitializeState(Config, State);
    State.CircuitAHealth01 = 0.25;

    FTABrakeHydraulicInput Input;
    Input.Pedal01 = 1.0;

    FTABrakeHydraulicOutput Output;

    TestTrue(
        TEXT("Damaged diagonal circuit step succeeds"),
        TABrakeHydraulics::Step(Config, Input, State, Output));

    const int32 FrontLeft =
        Corner(ETABrakeCornerIndex::FrontLeft);

    const int32 FrontRight =
        Corner(ETABrakeCornerIndex::FrontRight);

    const int32 RearRight =
        Corner(ETABrakeCornerIndex::RearRight);

    TestTrue(
        TEXT("Circuit A reduces front-left pressure"),
        Output.CornerEffectivePressurePa[FrontLeft]
            < Output.CornerEffectivePressurePa[FrontRight]);

    TestTrue(
        TEXT("Circuit A also reduces diagonally opposite rear-right pressure"),
        FMath::IsNearlyEqual(
            Output.CornerEffectivePressurePa[RearRight],
            0.25
                * State.CornerLinePressurePa[RearRight]
                * Output.FluidPressureTransfer01,
            1.0e-6));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTABrakeHydraulicsIndependentCornerModulationTest,
    "TorqueAtlas.Vehicle.Brakes.Hydraulics.CornerModulationIsIndependent",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTABrakeHydraulicsIndependentCornerModulationTest::RunTest(
    const FString& Parameters)
{
    FTABrakeHydraulicConfig Config;
    Config.PressureRiseRatePaPerSec = 1.0e12;
    Config.PressureReleaseRatePaPerSec = 1.0e12;

    FTABrakeHydraulicState State;
    TABrakeHydraulics::InitializeState(Config, State);

    FTABrakeHydraulicInput Input;
    Input.Pedal01 = 1.0;

    const int32 FrontLeft =
        Corner(ETABrakeCornerIndex::FrontLeft);

    const int32 FrontRight =
        Corner(ETABrakeCornerIndex::FrontRight);

    Input.CornerPressureModulation01[FrontLeft] = 0.0;
    Input.CornerPressureModulation01[FrontRight] = 1.0;

    FTABrakeHydraulicOutput Output;

    TestTrue(
        TEXT("Independent corner modulation step succeeds"),
        TABrakeHydraulics::Step(Config, Input, State, Output));

    TestEqual(
        TEXT("Released front-left corner reaches zero target pressure"),
        Output.CornerTargetPressurePa[FrontLeft],
        0.0);

    TestTrue(
        TEXT("Front-right retains brake pressure"),
        Output.CornerEffectivePressurePa[FrontRight] > 0.0);

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

    TestTrue(
        TEXT("Boiling point interpolates"),
        FMath::IsNearlyEqual(
            TABrakeHydraulics::CalculateBoilingPointC(Config),
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
        TABrakeHydraulics::Step(Config, Input, State, Output));

    TestTrue(
        TEXT("Full transition superheat reaches full vapor"),
        FMath::IsNearlyEqual(Output.VaporFraction01, 1.0, 1.0e-9));

    TestTrue(
        TEXT("Full vapor reduces pressure transfer to configured floor"),
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

    const double HeatedTemperatureC =
        State.FluidTemperatureC;

    TestTrue(
        TEXT("Conducted heat raises fluid temperature"),
        HeatedTemperatureC > 20.0);

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

    return true;
}

#endif
