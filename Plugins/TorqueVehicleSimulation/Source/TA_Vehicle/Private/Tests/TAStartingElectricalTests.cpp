#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TAStartingElectrical.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAStartingElectricalConvergenceTest,
    "TorqueAtlas.Vehicle.Electrical.StartingSystem.Converges",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAStartingElectricalConvergenceTest::RunTest(
    const FString& Parameters)
{
    FTAStartingElectricalConfig Config;

    FTAStartingElectricalState State;
    TAStartingElectrical::InitializeState(State);

    FTAStartingElectricalInput Input;
    Input.bStarterEngaged = true;

    FTAStartingElectricalOutput Output;

    TestTrue(
        TEXT("Starting electrical solve succeeds"),
        TAStartingElectrical::Step(
            Config,
            Input,
            State,
            Output));

    TestTrue(
        TEXT("Starter/battery coupling converges"),
        Output.bConverged);

    TestTrue(
        TEXT("Coupled bus voltage sags below open-circuit voltage"),
        Output.CoupledBusVoltageV
            < Output.Battery.OpenCircuitVoltageV);

    TestTrue(
        TEXT("Coupled starter produces crank torque"),
        Output.Starter.CrankshaftTorqueNm > 0.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAStartingElectricalWeakBatteryTest,
    "TorqueAtlas.Vehicle.Electrical.StartingSystem.WeakBatteryReducesCrankTorque",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAStartingElectricalWeakBatteryTest::RunTest(
    const FString& Parameters)
{
    FTAStartingElectricalConfig HealthyConfig;

    FTAStartingElectricalConfig WeakConfig =
        HealthyConfig;

    WeakConfig.Battery.InternalResistanceOhm =
        0.080;

    FTAStartingElectricalState HealthyState;
    FTAStartingElectricalState WeakState;
    TAStartingElectrical::InitializeState(HealthyState);
    TAStartingElectrical::InitializeState(WeakState);

    FTAStartingElectricalInput Input;
    Input.bStarterEngaged = true;

    FTAStartingElectricalOutput HealthyOutput;
    FTAStartingElectricalOutput WeakOutput;

    TestTrue(
        TEXT("Healthy start solve succeeds"),
        TAStartingElectrical::Step(
            HealthyConfig,
            Input,
            HealthyState,
            HealthyOutput));

    TestTrue(
        TEXT("Weak-battery start solve succeeds"),
        TAStartingElectrical::Step(
            WeakConfig,
            Input,
            WeakState,
            WeakOutput));

    TestTrue(
        TEXT("Weak battery produces lower bus voltage"),
        WeakOutput.CoupledBusVoltageV
            < HealthyOutput.CoupledBusVoltageV);

    TestTrue(
        TEXT("Weak battery produces lower starter crank torque"),
        WeakOutput.Starter.CrankshaftTorqueNm
            < HealthyOutput.Starter.CrankshaftTorqueNm);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAStartingElectricalBackEmfTest,
    "TorqueAtlas.Vehicle.Electrical.StartingSystem.EngineSpeedReducesStarterLoad",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAStartingElectricalBackEmfTest::RunTest(
    const FString& Parameters)
{
    FTAStartingElectricalConfig Config;

    FTAStartingElectricalState SlowState;
    FTAStartingElectricalState FastState;
    TAStartingElectrical::InitializeState(SlowState);
    TAStartingElectrical::InitializeState(FastState);

    FTAStartingElectricalInput Slow;
    Slow.bStarterEngaged = true;
    Slow.EngineAngularSpeedRadPerSec = 0.0;

    FTAStartingElectricalInput Fast =
        Slow;
    Fast.EngineAngularSpeedRadPerSec = 40.0;

    FTAStartingElectricalOutput SlowOutput;
    FTAStartingElectricalOutput FastOutput;

    TestTrue(
        TEXT("Slow start solve succeeds"),
        TAStartingElectrical::Step(
            Config,
            Slow,
            SlowState,
            SlowOutput));

    TestTrue(
        TEXT("Fast start solve succeeds"),
        TAStartingElectrical::Step(
            Config,
            Fast,
            FastState,
            FastOutput));

    TestTrue(
        TEXT("Back EMF reduces starter current at higher engine speed"),
        FastOutput.Starter.CurrentA
            < SlowOutput.Starter.CurrentA);

    return true;
}

#endif
