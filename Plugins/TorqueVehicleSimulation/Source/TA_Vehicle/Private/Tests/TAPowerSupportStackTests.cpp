#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TAPowerSupportStack.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAPowerSupportFuelPumpDisconnectTest,
    "TorqueAtlas.Vehicle.PowerSupport.FuelPumpDisconnectReducesFuelDelivery",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAPowerSupportFuelPumpDisconnectTest::RunTest(
    const FString& Parameters)
{
    FTAPowerSupportStackConfig Config;
    Config.Fuel.PressureRiseRatePaPerSec = 1.0e12;

    FTAPowerSupportStackState Healthy;
    FTAPowerSupportStackState Disconnected;
    TAPowerSupportStack::InitializeState(Config, Healthy);
    TAPowerSupportStack::InitializeState(Config, Disconnected);

    const int32 FuelPump =
        static_cast<int32>(
            ETAElectricalConsumer::FuelPump);

    Disconnected.Electrical
        .ConsumerConnectionHealth01[FuelPump] =
        0.0;

    FTAPowerSupportStackInput Input;
    Input.EngineRPM = 2500.0;
    Input.DeltaTimeSeconds = 0.1;

    FTAPowerSupportStackOutput HealthyOutput;
    FTAPowerSupportStackOutput DisconnectedOutput;

    TestTrue(
        TEXT("Healthy power-support step succeeds"),
        TAPowerSupportStack::Step(
            Config,
            Input,
            Healthy,
            HealthyOutput));

    TestTrue(
        TEXT("Disconnected fuel-pump step succeeds"),
        TAPowerSupportStack::Step(
            Config,
            Input,
            Disconnected,
            DisconnectedOutput));

    TestTrue(
        TEXT("Fuel-pump disconnect removes pump authority"),
        DisconnectedOutput.FuelPumpAuthority01
            < HealthyOutput.FuelPumpAuthority01);

    TestTrue(
        TEXT("Fuel-pump disconnect lowers rail pressure"),
        DisconnectedOutput.Fuel.RailPressurePa
            < HealthyOutput.Fuel.RailPressurePa);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAPowerSupportCoolingFanDisconnectTest,
    "TorqueAtlas.Vehicle.PowerSupport.FanDisconnectReducesLowSpeedCooling",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAPowerSupportCoolingFanDisconnectTest::RunTest(
    const FString& Parameters)
{
    FTAPowerSupportStackConfig Config;

    FTAPowerSupportStackState Healthy;
    FTAPowerSupportStackState Disconnected;
    TAPowerSupportStack::InitializeState(Config, Healthy);
    TAPowerSupportStack::InitializeState(Config, Disconnected);

    const int32 CoolingFan =
        static_cast<int32>(
            ETAElectricalConsumer::CoolingFan);

    Disconnected.Electrical
        .ConsumerConnectionHealth01[CoolingFan] =
        0.0;

    FTAPowerSupportStackInput Input;
    Input.EngineRPM = 2500.0;
    Input.RamAirflowAuthority01 = 0.0;

    FTAPowerSupportStackOutput HealthyOutput;
    FTAPowerSupportStackOutput DisconnectedOutput;

    TestTrue(
        TEXT("Healthy cooling-support step succeeds"),
        TAPowerSupportStack::Step(
            Config,
            Input,
            Healthy,
            HealthyOutput));

    TestTrue(
        TEXT("Disconnected fan step succeeds"),
        TAPowerSupportStack::Step(
            Config,
            Input,
            Disconnected,
            DisconnectedOutput));

    TestTrue(
        TEXT("Fan disconnect reduces low-speed airflow authority"),
        DisconnectedOutput.EffectiveCoolingAirflowAuthority01
            < HealthyOutput.EffectiveCoolingAirflowAuthority01);

    TestTrue(
        TEXT("Fan disconnect reduces cooling authority"),
        DisconnectedOutput.Coolant.CoolingAuthority01
            < HealthyOutput.Coolant.CoolingAuthority01);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAPowerSupportRamAirTest,
    "TorqueAtlas.Vehicle.PowerSupport.RamAirCanSustainCoolingWithoutFan",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAPowerSupportRamAirTest::RunTest(
    const FString& Parameters)
{
    FTAPowerSupportStackConfig Config;

    FTAPowerSupportStackState State;
    TAPowerSupportStack::InitializeState(Config, State);

    const int32 CoolingFan =
        static_cast<int32>(
            ETAElectricalConsumer::CoolingFan);

    State.Electrical
        .ConsumerConnectionHealth01[CoolingFan] =
        0.0;

    FTAPowerSupportStackInput Input;
    Input.EngineRPM = 3000.0;
    Input.RamAirflowAuthority01 = 0.8;

    FTAPowerSupportStackOutput Output;

    TestTrue(
        TEXT("Ram-air support step succeeds"),
        TAPowerSupportStack::Step(
            Config,
            Input,
            State,
            Output));

    TestTrue(
        TEXT("Ram airflow remains available without cooling fan"),
        FMath::IsNearlyEqual(
            Output.EffectiveCoolingAirflowAuthority01,
            0.8,
            1.0e-12));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAPowerSupportEcuDisconnectTest,
    "TorqueAtlas.Vehicle.PowerSupport.EcuDisconnectIsExplicit",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAPowerSupportEcuDisconnectTest::RunTest(
    const FString& Parameters)
{
    FTAPowerSupportStackConfig Config;

    FTAPowerSupportStackState State;
    TAPowerSupportStack::InitializeState(Config, State);

    const int32 Ecu =
        static_cast<int32>(
            ETAElectricalConsumer::EcuIgnition);

    State.Electrical
        .ConsumerConnectionHealth01[Ecu] =
        0.0;

    FTAPowerSupportStackInput Input;
    Input.EngineRPM = 2000.0;

    FTAPowerSupportStackOutput Output;

    TestTrue(
        TEXT("ECU-disconnect support step succeeds"),
        TAPowerSupportStack::Step(
            Config,
            Input,
            State,
            Output));

    TestEqual(
        TEXT("ECU disconnect removes engine-control authority"),
        Output.EngineControlAuthority01,
        0.0);

    return true;
}

#endif
