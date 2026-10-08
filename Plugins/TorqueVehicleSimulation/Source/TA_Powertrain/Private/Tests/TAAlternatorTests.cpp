#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TAAlternator.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAAlternatorCutInTest,
    "TorqueAtlas.Powertrain.Alternator.BelowCutInProducesNoOutput",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAAlternatorCutInTest::RunTest(
    const FString& Parameters)
{
    FTAAlternatorConfig Config;

    FTAAlternatorInput Input;
    Input.EngineRPM = 500.0;
    Input.BusVoltageV = 12.0;

    FTAAlternatorOutput Output;

    TestTrue(
        TEXT("Alternator solve succeeds"),
        TAAlternator::Calculate(
            Config,
            Input,
            Output));

    TestEqual(
        TEXT("Below cut-in produces zero current"),
        Output.OutputCurrentA,
        0.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAAlternatorLowVoltageTest,
    "TorqueAtlas.Powertrain.Alternator.LowBusVoltageProducesChargeCurrent",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAAlternatorLowVoltageTest::RunTest(
    const FString& Parameters)
{
    FTAAlternatorConfig Config;

    FTAAlternatorInput Input;
    Input.EngineRPM = 3000.0;
    Input.BusVoltageV = 12.0;

    FTAAlternatorOutput Output;

    TestTrue(
        TEXT("Charging alternator solve succeeds"),
        TAAlternator::Calculate(
            Config,
            Input,
            Output));

    TestTrue(
        TEXT("Low bus voltage produces current"),
        Output.OutputCurrentA > 0.0);

    TestTrue(
        TEXT("Electrical output creates mechanical engine load"),
        Output.MechanicalLoadTorqueNm > 0.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAAlternatorRegulatedVoltageTest,
    "TorqueAtlas.Powertrain.Alternator.AtRegulatedVoltageOutputFallsToZero",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAAlternatorRegulatedVoltageTest::RunTest(
    const FString& Parameters)
{
    FTAAlternatorConfig Config;

    FTAAlternatorInput Input;
    Input.EngineRPM = 3000.0;
    Input.BusVoltageV = Config.RegulatedVoltageV;

    FTAAlternatorOutput Output;

    TestTrue(
        TEXT("Regulated-voltage solve succeeds"),
        TAAlternator::Calculate(
            Config,
            Input,
            Output));

    TestEqual(
        TEXT("Regulated voltage removes charge demand"),
        Output.OutputCurrentA,
        0.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAAlternatorHealthTest,
    "TorqueAtlas.Powertrain.Alternator.HealthScalesOutput",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAAlternatorHealthTest::RunTest(
    const FString& Parameters)
{
    FTAAlternatorConfig Config;

    FTAAlternatorInput Healthy;
    Healthy.EngineRPM = 3000.0;
    Healthy.BusVoltageV = 12.0;
    Healthy.Health01 = 1.0;

    FTAAlternatorInput Damaged =
        Healthy;

    Damaged.Health01 = 0.5;

    FTAAlternatorOutput HealthyOutput;
    FTAAlternatorOutput DamagedOutput;

    TestTrue(
        TEXT("Healthy alternator solve succeeds"),
        TAAlternator::Calculate(
            Config,
            Healthy,
            HealthyOutput));

    TestTrue(
        TEXT("Damaged alternator solve succeeds"),
        TAAlternator::Calculate(
            Config,
            Damaged,
            DamagedOutput));

    TestTrue(
        TEXT("Half health halves current output"),
        FMath::IsNearlyEqual(
            DamagedOutput.OutputCurrentA,
            0.5 * HealthyOutput.OutputCurrentA,
            1.0e-9));

    return true;
}

#endif
