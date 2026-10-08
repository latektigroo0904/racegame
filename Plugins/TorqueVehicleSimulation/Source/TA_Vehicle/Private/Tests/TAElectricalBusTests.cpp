#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TAElectricalBus.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAElectricalBusHealthyTest,
    "TorqueAtlas.Vehicle.Electrical.Bus.HealthyCriticalConsumersHaveAuthority",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAElectricalBusHealthyTest::RunTest(
    const FString& Parameters)
{
    FTAElectricalBusConfig Config;

    FTAElectricalBusState State;
    TAElectricalBus::InitializeState(State);

    FTAElectricalBusInput Input;
    Input.EngineRPM = 3000.0;

    FTAElectricalBusOutput Output;

    TestTrue(
        TEXT("Healthy electrical bus step succeeds"),
        TAElectricalBus::Step(
            Config,
            Input,
            State,
            Output));

    const int32 Ecu =
        static_cast<int32>(
            ETAElectricalConsumer::EcuIgnition);

    TestTrue(
        TEXT("Healthy ECU has voltage authority"),
        Output.ConsumerVoltageAuthority01[Ecu]
            > 0.0);

    TestFalse(
        TEXT("Healthy bus does not brown out critical consumers"),
        Output.bCriticalBrownout);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAElectricalBusStarterSagTest,
    "TorqueAtlas.Vehicle.Electrical.Bus.StarterLoadReducesVoltage",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAElectricalBusStarterSagTest::RunTest(
    const FString& Parameters)
{
    FTAElectricalBusConfig Config;
    Config.Battery.InternalResistanceOhm = 0.025;

    FTAElectricalBusState IdleState;
    FTAElectricalBusState StartState;
    TAElectricalBus::InitializeState(IdleState);
    TAElectricalBus::InitializeState(StartState);

    FTAElectricalBusInput IdleInput;

    FTAElectricalBusInput StartInput;
    StartInput.bStarterEngaged = true;
    StartInput.StarterRequestedCurrentA = 300.0;

    FTAElectricalBusOutput IdleOutput;
    FTAElectricalBusOutput StartOutput;

    TestTrue(
        TEXT("Idle bus step succeeds"),
        TAElectricalBus::Step(
            Config,
            IdleInput,
            IdleState,
            IdleOutput));

    TestTrue(
        TEXT("Starter bus step succeeds"),
        TAElectricalBus::Step(
            Config,
            StartInput,
            StartState,
            StartOutput));

    TestTrue(
        TEXT("Starter draw reduces bus voltage"),
        StartOutput.BusVoltageV
            < IdleOutput.BusVoltageV);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAElectricalBusConnectionFailureTest,
    "TorqueAtlas.Vehicle.Electrical.Bus.ConsumerDisconnectIsLocal",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAElectricalBusConnectionFailureTest::RunTest(
    const FString& Parameters)
{
    FTAElectricalBusConfig Config;

    FTAElectricalBusState State;
    TAElectricalBus::InitializeState(State);

    const int32 FuelPump =
        static_cast<int32>(
            ETAElectricalConsumer::FuelPump);

    const int32 Ecu =
        static_cast<int32>(
            ETAElectricalConsumer::EcuIgnition);

    State.ConsumerConnectionHealth01[FuelPump] =
        0.0;

    FTAElectricalBusInput Input;
    Input.EngineRPM = 2500.0;

    FTAElectricalBusOutput Output;

    TestTrue(
        TEXT("Disconnected fuel-pump bus step succeeds"),
        TAElectricalBus::Step(
            Config,
            Input,
            State,
            Output));

    TestEqual(
        TEXT("Fuel pump loses voltage authority"),
        Output.ConsumerVoltageAuthority01[FuelPump],
        0.0);

    TestTrue(
        TEXT("ECU remains powered by separate connection"),
        Output.ConsumerVoltageAuthority01[Ecu]
            > 0.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAElectricalBusAlternatorSupportTest,
    "TorqueAtlas.Vehicle.Electrical.Bus.AlternatorReducesBatteryDischarge",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAElectricalBusAlternatorSupportTest::RunTest(
    const FString& Parameters)
{
    FTAElectricalBusConfig Config;

    FTAElectricalBusState NoAlternatorState;
    FTAElectricalBusState AlternatorState;
    TAElectricalBus::InitializeState(NoAlternatorState);
    TAElectricalBus::InitializeState(AlternatorState);

    FTAElectricalBusInput NoAlternatorInput;
    NoAlternatorInput.EngineRPM = 0.0;

    FTAElectricalBusInput AlternatorInput =
        NoAlternatorInput;

    AlternatorInput.EngineRPM = 3000.0;

    FTAElectricalBusOutput NoAlternatorOutput;
    FTAElectricalBusOutput AlternatorOutput;

    TestTrue(
        TEXT("No-alternator bus step succeeds"),
        TAElectricalBus::Step(
            Config,
            NoAlternatorInput,
            NoAlternatorState,
            NoAlternatorOutput));

    TestTrue(
        TEXT("Alternator-supported bus step succeeds"),
        TAElectricalBus::Step(
            Config,
            AlternatorInput,
            AlternatorState,
            AlternatorOutput));

    TestTrue(
        TEXT("Alternator lowers battery discharge current"),
        AlternatorOutput.Battery.BatteryCurrentA
            < NoAlternatorOutput.Battery.BatteryCurrentA);

    return true;
}

#endif
