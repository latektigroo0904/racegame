#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TABrakeThermal.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTABrakeThermalHeatingFadeTest,
    "TorqueAtlas.Vehicle.Brakes.HeatingCausesFade",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTABrakeThermalHeatingFadeTest::RunTest(
    const FString& Parameters)
{
    FTABrakeThermalConfig Config;
    Config.ThermalMassJPerC = 1000.0;
    Config.CoolingWPerC = 0.0;
    Config.FadeStartTemperatureC = 100.0;
    Config.FadeEndTemperatureC = 200.0;
    Config.MinimumFadeTorqueFactor01 = 0.30;
    Config.WearEnergyCapacityJ = 1.0e12;

    FTABrakeThermalState State;
    TABrakeThermal::InitializeState(
        Config,
        State);

    const double InitialTemperatureC =
        State.TemperatureC;

    FTABrakeThermalOutput Output;

    for (int32 StepIndex = 0;
         StepIndex < 20;
         ++StepIndex)
    {
        TestTrue(
            TEXT("Brake thermal heating step succeeds"),
            TABrakeThermal::Update(
                Config,
                1000.0,
                20.0,
                0.5,
                State,
                Output));
    }

    TestTrue(
        TEXT("Brake temperature rises under friction work"),
        State.TemperatureC
            > InitialTemperatureC);

    TestTrue(
        TEXT("Brake crosses fade threshold"),
        State.TemperatureC
            > Config.FadeStartTemperatureC);

    TestTrue(
        TEXT("Thermal brake torque factor falls below one"),
        State.ThermalTorqueFactor01
            < 1.0);

    TestTrue(
        TEXT("Thermal torque factor respects configured fade floor"),
        State.ThermalTorqueFactor01
            >= Config.MinimumFadeTorqueFactor01);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTABrakeThermalCoolingTest,
    "TorqueAtlas.Vehicle.Brakes.CoolsTowardAmbient",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTABrakeThermalCoolingTest::RunTest(
    const FString& Parameters)
{
    FTABrakeThermalConfig Config;
    Config.ThermalMassJPerC = 10000.0;
    Config.CoolingWPerC = 150.0;
    Config.AmbientTemperatureC = 20.0;

    FTABrakeThermalState State;
    TABrakeThermal::InitializeState(
        Config,
        State);

    State.TemperatureC =
        300.0;

    FTABrakeThermalOutput Output;

    TestTrue(
        TEXT("Brake cooling step succeeds"),
        TABrakeThermal::Update(
            Config,
            0.0,
            0.0,
            10.0,
            State,
            Output));

    TestTrue(
        TEXT("Hot brake cools without friction input"),
        State.TemperatureC
            < 300.0);

    TestTrue(
        TEXT("Cooling does not undershoot ambient"),
        State.TemperatureC
            >= Config.AmbientTemperatureC);

    TestTrue(
        TEXT("Cooling power is positive above ambient"),
        Output.CoolingPowerW > 0.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTABrakeThermalWearTest,
    "TorqueAtlas.Vehicle.Brakes.FrictionEnergyCreatesWear",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTABrakeThermalWearTest::RunTest(
    const FString& Parameters)
{
    FTABrakeThermalConfig Config;
    Config.ThermalMassJPerC = 1.0e9;
    Config.CoolingWPerC = 0.0;
    Config.HeatFraction01 = 1.0;
    Config.FadeStartTemperatureC = 10000.0;
    Config.FadeEndTemperatureC = 11000.0;
    Config.WearEnergyCapacityJ = 100000.0;
    Config.WearTorqueLossAtEnd01 = 0.40;

    FTABrakeThermalState State;
    TABrakeThermal::InitializeState(
        Config,
        State);

    FTABrakeThermalOutput Output;

    TestTrue(
        TEXT("Brake wear step succeeds"),
        TABrakeThermal::Update(
            Config,
            1000.0,
            10.0,
            10.0,
            State,
            Output));

    TestTrue(
        TEXT("Configured wear energy reaches end-of-life"),
        FMath::IsNearlyEqual(
            State.Wear01,
            1.0,
            1.0e-9));

    TestTrue(
        TEXT("End-of-life wear reduces available torque factor"),
        FMath::IsNearlyEqual(
            State.WearTorqueFactor01,
            0.60,
            1.0e-9));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTABrakeThermalValidationTest,
    "TorqueAtlas.Vehicle.Brakes.ConfigValidation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTABrakeThermalValidationTest::RunTest(
    const FString& Parameters)
{
    FTABrakeThermalConfig Config;

    TestTrue(
        TEXT("Default brake thermal config validates"),
        TABrakeThermal::ValidateConfig(
            Config));

    Config.FadeEndTemperatureC =
        Config.FadeStartTemperatureC;

    TestFalse(
        TEXT("Non-increasing fade range is rejected"),
        TABrakeThermal::ValidateConfig(
            Config));

    Config =
        FTABrakeThermalConfig{};

    Config.WearEnergyCapacityJ =
        0.0;

    TestFalse(
        TEXT("Non-positive wear-energy capacity is rejected"),
        TABrakeThermal::ValidateConfig(
            Config));

    return true;
}

#endif
