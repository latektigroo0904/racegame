#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TAEngineSupportStack.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAEngineSupportFuelAuthorityTest,
    "TorqueAtlas.Vehicle.EngineSupport.FuelAndEcuMultiplyCombustionAuthority",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAEngineSupportFuelAuthorityTest::RunTest(
    const FString& Parameters)
{
    FTAEngineSupportStackConfig Config;
    Config.PowerSupport.Fuel.PressureRiseRatePaPerSec = 1.0e12;

    FTAEngineSupportStackState State;
    TAEngineSupportStack::InitializeState(Config, State);

    FTAEngineSupportStackInput Input;
    Input.PowerSupport.EngineRPM = 2500.0;
    Input.PowerSupport.DeltaTimeSeconds = 0.1;

    FTAEngineSupportStackOutput Output;

    TestTrue(
        TEXT("Healthy engine-support step succeeds"),
        TAEngineSupportStack::Step(
            Config,
            Input,
            State,
            Output));

    TestTrue(
        TEXT("Healthy support produces combustion authority"),
        Output.CombustionAuthority01 > 0.0);

    const int32 Ecu =
        static_cast<int32>(
            ETAElectricalConsumer::EcuIgnition);

    State.PowerSupport.Electrical
        .ConsumerConnectionHealth01[Ecu] =
        0.0;

    TestTrue(
        TEXT("ECU-disconnected engine-support step succeeds"),
        TAEngineSupportStack::Step(
            Config,
            Input,
            State,
            Output));

    TestEqual(
        TEXT("ECU disconnect removes combustion authority"),
        Output.CombustionAuthority01,
        0.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAEngineSupportOilAuthorityTest,
    "TorqueAtlas.Vehicle.EngineSupport.OilPumpDamageReducesLubricationAuthority",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAEngineSupportOilAuthorityTest::RunTest(
    const FString& Parameters)
{
    FTAEngineSupportStackConfig Config;

    FTAEngineSupportStackState Healthy;
    FTAEngineSupportStackState Damaged;
    TAEngineSupportStack::InitializeState(Config, Healthy);
    TAEngineSupportStack::InitializeState(Config, Damaged);

    FTAEngineSupportStackInput HealthyInput;
    HealthyInput.PowerSupport.EngineRPM = 3000.0;
    HealthyInput.OilPumpHealth01 = 1.0;

    FTAEngineSupportStackInput DamagedInput =
        HealthyInput;

    DamagedInput.OilPumpHealth01 = 0.1;

    FTAEngineSupportStackOutput HealthyOutput;
    FTAEngineSupportStackOutput DamagedOutput;

    TestTrue(
        TEXT("Healthy oil-support step succeeds"),
        TAEngineSupportStack::Step(
            Config,
            HealthyInput,
            Healthy,
            HealthyOutput));

    TestTrue(
        TEXT("Damaged oil-support step succeeds"),
        TAEngineSupportStack::Step(
            Config,
            DamagedInput,
            Damaged,
            DamagedOutput));

    TestTrue(
        TEXT("Oil pump damage reduces lubrication authority"),
        DamagedOutput.LubricationAuthority01
            < HealthyOutput.LubricationAuthority01);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAEngineSupportAlternatorLoadTest,
    "TorqueAtlas.Vehicle.EngineSupport.AlternatorLoadIsExposed",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAEngineSupportAlternatorLoadTest::RunTest(
    const FString& Parameters)
{
    FTAEngineSupportStackConfig Config;

    FTAEngineSupportStackState State;
    TAEngineSupportStack::InitializeState(Config, State);

    FTAEngineSupportStackInput Input;
    Input.PowerSupport.EngineRPM = 3000.0;

    FTAEngineSupportStackOutput Output;

    TestTrue(
        TEXT("Alternator-load engine-support step succeeds"),
        TAEngineSupportStack::Step(
            Config,
            Input,
            State,
            Output));

    TestTrue(
        TEXT("Electrical generation exposes mechanical load torque"),
        Output.AlternatorMechanicalLoadTorqueNm >= 0.0);

    return true;
}

#endif
