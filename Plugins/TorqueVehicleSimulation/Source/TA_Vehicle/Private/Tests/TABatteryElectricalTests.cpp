#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TABatteryElectrical.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTABatteryNoLoadTest,
    "TorqueAtlas.Vehicle.Electrical.BatteryNoLoadHoldsOpenCircuitVoltage",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTABatteryNoLoadTest::RunTest(
    const FString& Parameters)
{
    FTABatteryConfig Config;

    FTABatteryState State;
    State.StateOfCharge01 = 1.0;

    FTABatteryInput Input;

    FTABatteryOutput Output;

    TestTrue(
        TEXT("No-load battery step succeeds"),
        TABatteryElectrical::Step(
            Config,
            Input,
            State,
            Output));

    TestTrue(
        TEXT("No-load terminal voltage equals OCV"),
        FMath::IsNearlyEqual(
            Output.TerminalVoltageV,
            Output.OpenCircuitVoltageV,
            1.0e-12));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTABatteryStarterSagTest,
    "TorqueAtlas.Vehicle.Electrical.StarterLoadCausesVoltageSag",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTABatteryStarterSagTest::RunTest(
    const FString& Parameters)
{
    FTABatteryConfig Config;
    Config.InternalResistanceOhm = 0.020;

    FTABatteryState State;

    FTABatteryInput Input;
    Input.RequestedLoadCurrentA = 300.0;

    FTABatteryOutput Output;

    TestTrue(
        TEXT("Starter-load battery step succeeds"),
        TABatteryElectrical::Step(
            Config,
            Input,
            State,
            Output));

    TestTrue(
        TEXT("Load causes terminal voltage sag"),
        Output.TerminalVoltageV
            < Output.OpenCircuitVoltageV);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTABatteryResistanceSagTest,
    "TorqueAtlas.Vehicle.Electrical.HigherResistanceCausesMoreSag",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTABatteryResistanceSagTest::RunTest(
    const FString& Parameters)
{
    FTABatteryConfig LowResistance;
    LowResistance.InternalResistanceOhm = 0.010;

    FTABatteryConfig HighResistance =
        LowResistance;

    HighResistance.InternalResistanceOhm = 0.040;

    FTABatteryState LowState;
    FTABatteryState HighState;

    FTABatteryInput Input;
    Input.RequestedLoadCurrentA = 200.0;

    FTABatteryOutput LowOutput;
    FTABatteryOutput HighOutput;

    TestTrue(
        TEXT("Low-resistance step succeeds"),
        TABatteryElectrical::Step(
            LowResistance,
            Input,
            LowState,
            LowOutput));

    TestTrue(
        TEXT("High-resistance step succeeds"),
        TABatteryElectrical::Step(
            HighResistance,
            Input,
            HighState,
            HighOutput));

    TestTrue(
        TEXT("Higher internal resistance causes lower terminal voltage"),
        HighOutput.TerminalVoltageV
            < LowOutput.TerminalVoltageV);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTABatteryChargeTest,
    "TorqueAtlas.Vehicle.Electrical.AlternatorCanChargeBattery",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTABatteryChargeTest::RunTest(
    const FString& Parameters)
{
    FTABatteryConfig Config;

    FTABatteryState State;
    State.StateOfCharge01 = 0.50;

    const double InitialSoc =
        State.StateOfCharge01;

    FTABatteryInput Input;
    Input.RequestedLoadCurrentA = 20.0;
    Input.AlternatorCurrentA = 60.0;
    Input.DeltaTimeSeconds = 60.0;

    FTABatteryOutput Output;

    TestTrue(
        TEXT("Charging battery step succeeds"),
        TABatteryElectrical::Step(
            Config,
            Input,
            State,
            Output));

    TestTrue(
        TEXT("Net alternator surplus charges battery"),
        State.StateOfCharge01
            > InitialSoc);

    TestTrue(
        TEXT("Battery current is negative while charging"),
        Output.BatteryCurrentA < 0.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTABatteryValidationTest,
    "TorqueAtlas.Vehicle.Electrical.BatteryConfigValidation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTABatteryValidationTest::RunTest(
    const FString& Parameters)
{
    FTABatteryConfig Config;

    TestTrue(
        TEXT("Default battery config validates"),
        TABatteryElectrical::ValidateConfig(Config));

    Config.CapacityAh = 0.0;

    TestFalse(
        TEXT("Zero battery capacity is rejected"),
        TABatteryElectrical::ValidateConfig(Config));

    return true;
}

#endif
