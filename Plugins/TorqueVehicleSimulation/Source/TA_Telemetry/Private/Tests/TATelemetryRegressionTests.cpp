#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TATelemetryRegression.h"

namespace
{
    FTATelemetryRingBuffer MakeTelemetryTrace(
        const uint32 Hash = 1234u)
    {
        FTATelemetryRingBuffer Buffer;
        Buffer.Initialize(8);

        const double Speeds[] =
        {
            0.0,
            5.0,
            10.0,
            15.0
        };

        const double Loads[] =
        {
            3200.0,
            3300.0,
            3400.0,
            3500.0
        };

        const double YawRates[] =
        {
            -0.1,
            -0.2,
            -0.8,
            -0.4
        };

        for (int32 Index = 0;
             Index < 4;
             ++Index)
        {
            FTAVehicleTelemetrySample Sample;

            Sample.SimulationTick =
                static_cast<uint64>(Index);

            Sample.PhysicsConfigHash =
                Hash;

            Sample.EngineRPM =
                1000.0 + 500.0 * Index;

            Sample.ChassisLinearVelocityWorldMps.X =
                Speeds[Index];

            Sample.ChassisAngularVelocityWorldRadPerSec.Z =
                YawRates[Index];

            Sample.WheelVerticalLoadN[0] =
                Loads[Index];

            Sample.TireRadialDeflectionM[0] =
                0.010 + 0.001 * Index;

            Buffer.Push(Sample);
        }

        return Buffer;
    }

    FTATelemetryRegressionConfig MakePassingConfig()
    {
        FTATelemetryRegressionConfig Config;
        Config.ScenarioId =
            TEXT("AccelerationSmoke");
        Config.ExpectedPhysicsConfigHash =
            1234u;
        Config.MinimumRequiredSamples =
            4;

        FTATelemetryMetricEnvelope FinalSpeed;
        FinalSpeed.Metric =
            ETATelemetryMetric::ChassisForwardSpeedMps;
        FinalSpeed.Statistic =
            ETATelemetryStatistic::Final;
        FinalSpeed.MinimumAllowed =
            14.5;
        FinalSpeed.MaximumAllowed =
            15.5;

        Config.Envelopes.Add(
            FinalSpeed);

        FTATelemetryMetricEnvelope LateMeanSpeed;
        LateMeanSpeed.Metric =
            ETATelemetryMetric::ChassisForwardSpeedMps;
        LateMeanSpeed.Statistic =
            ETATelemetryStatistic::Mean;
        LateMeanSpeed.StartFraction01 =
            0.5;
        LateMeanSpeed.EndFraction01 =
            1.0;
        LateMeanSpeed.MinimumAllowed =
            12.4;
        LateMeanSpeed.MaximumAllowed =
            12.6;

        Config.Envelopes.Add(
            LateMeanSpeed);

        FTATelemetryMetricEnvelope FrontLeftMaxLoad;
        FrontLeftMaxLoad.Metric =
            ETATelemetryMetric::WheelVerticalLoadN;
        FrontLeftMaxLoad.Statistic =
            ETATelemetryStatistic::Maximum;
        FrontLeftMaxLoad.WheelIndex =
            0;
        FrontLeftMaxLoad.MinimumAllowed =
            3490.0;
        FrontLeftMaxLoad.MaximumAllowed =
            3510.0;

        Config.Envelopes.Add(
            FrontLeftMaxLoad);

        FTATelemetryMetricEnvelope AbsoluteYawPeak;
        AbsoluteYawPeak.Metric =
            ETATelemetryMetric::ChassisYawRateRadPerSec;
        AbsoluteYawPeak.Statistic =
            ETATelemetryStatistic::AbsoluteMaximum;
        AbsoluteYawPeak.MinimumAllowed =
            0.79;
        AbsoluteYawPeak.MaximumAllowed =
            0.81;

        Config.Envelopes.Add(
            AbsoluteYawPeak);

        return Config;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTATelemetryRegressionScenarioPassTest,
    "TorqueAtlas.Telemetry.Regression.ScenarioPass",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)

bool FTATelemetryRegressionScenarioPassTest::RunTest(
    const FString& Parameters)
{
    const FTATelemetryRingBuffer Buffer =
        MakeTelemetryTrace();

    const FTATelemetryRegressionConfig Config =
        MakePassingConfig();

    FTATelemetryRegressionResult Result;

    TestTrue(
        TEXT("Regression evaluation succeeds"),
        TATelemetryRegression::Evaluate(
            Buffer,
            Config,
            Result));

    TestTrue(
        TEXT("Scenario passes"),
        Result.bPassed);

    TestEqual(
        TEXT("Scenario ID retained"),
        Result.ScenarioId,
        Config.ScenarioId);

    TestEqual(
        TEXT("Observed physics hash retained"),
        Result.ObservedPhysicsConfigHash,
        1234u);

    TestTrue(
        TEXT("Physics hash is consistent"),
        Result.bPhysicsConfigHashConsistent);

    TestTrue(
        TEXT("Physics hash matches expectation"),
        Result.bPhysicsConfigHashMatched);

    TestEqual(
        TEXT("All envelopes pass"),
        Result.PassedEnvelopeCount,
        Config.Envelopes.Num());

    TestEqual(
        TEXT("No envelope fails"),
        Result.FailedEnvelopeCount,
        0);

    TestEqual(
        TEXT("Absolute maximum is positive magnitude"),
        Result.EnvelopeResults[3].ObservedValue,
        0.8);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTATelemetryRegressionHashMismatchTest,
    "TorqueAtlas.Telemetry.Regression.HashMismatchFailsScenario",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)

bool FTATelemetryRegressionHashMismatchTest::RunTest(
    const FString& Parameters)
{
    const FTATelemetryRingBuffer Buffer =
        MakeTelemetryTrace(777u);

    FTATelemetryRegressionConfig Config =
        MakePassingConfig();

    Config.ExpectedPhysicsConfigHash =
        1234u;

    FTATelemetryRegressionResult Result;

    TestTrue(
        TEXT("Regression evaluation itself succeeds"),
        TATelemetryRegression::Evaluate(
            Buffer,
            Config,
            Result));

    TestFalse(
        TEXT("Hash mismatch fails scenario"),
        Result.bPassed);

    TestTrue(
        TEXT("Trace hash remains internally consistent"),
        Result.bPhysicsConfigHashConsistent);

    TestFalse(
        TEXT("Expected hash mismatch reported"),
        Result.bPhysicsConfigHashMatched);

    TestTrue(
        TEXT("Hash mismatch adds diagnostic failure"),
        Result.Failures.Num() > 0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTATelemetryRegressionMixedHashTest,
    "TorqueAtlas.Telemetry.Regression.MixedHashTraceRejected",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)

bool FTATelemetryRegressionMixedHashTest::RunTest(
    const FString& Parameters)
{
    FTATelemetryRingBuffer Buffer =
        MakeTelemetryTrace();

    FTAVehicleTelemetrySample Mixed;
    Mixed.SimulationTick = 4;
    Mixed.PhysicsConfigHash = 9999u;
    Mixed.ChassisLinearVelocityWorldMps.X = 15.0;
    Mixed.WheelVerticalLoadN[0] = 3500.0;

    Buffer.Push(Mixed);

    FTATelemetryRegressionConfig Config =
        MakePassingConfig();

    Config.MinimumRequiredSamples =
        5;

    FTATelemetryRegressionResult Result;

    TestTrue(
        TEXT("Mixed-hash trace evaluates"),
        TATelemetryRegression::Evaluate(
            Buffer,
            Config,
            Result));

    TestFalse(
        TEXT("Mixed-hash trace fails"),
        Result.bPassed);

    TestFalse(
        TEXT("Mixed-hash trace is reported inconsistent"),
        Result.bPhysicsConfigHashConsistent);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTATelemetryRegressionExportTest,
    "TorqueAtlas.Telemetry.Regression.MachineReadableExports",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)

bool FTATelemetryRegressionExportTest::RunTest(
    const FString& Parameters)
{
    const FTATelemetryRingBuffer Buffer =
        MakeTelemetryTrace();

    const FTATelemetryRegressionConfig Config =
        MakePassingConfig();

    FTATelemetryRegressionResult Result;

    TestTrue(
        TEXT("Regression evaluation succeeds"),
        TATelemetryRegression::Evaluate(
            Buffer,
            Config,
            Result));

    const FString Csv =
        TATelemetryRegression::ExportCsv(
            Result);

    const FString JsonLines =
        TATelemetryRegression::ExportJsonLines(
            Result);

    TestTrue(
        TEXT("CSV contains scenario identity"),
        Csv.Contains(
            TEXT("AccelerationSmoke")));

    TestTrue(
        TEXT("CSV contains observed hash"),
        Csv.Contains(
            TEXT("1234")));

    TestTrue(
        TEXT("CSV contains metric name"),
        Csv.Contains(
            TEXT("ChassisForwardSpeedMps")));

    TestTrue(
        TEXT("JSON-lines contains scenario field"),
        JsonLines.Contains(
            TEXT("\"scenario_id\":\"AccelerationSmoke\"")));

    TestTrue(
        TEXT("JSON-lines contains metric field"),
        JsonLines.Contains(
            TEXT("\"metric\":\"WheelVerticalLoadN\"")));

    TestTrue(
        TEXT("JSON-lines contains pass state"),
        JsonLines.Contains(
            TEXT("\"pass\":true")));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTATelemetryRegressionInvalidWheelTest,
    "TorqueAtlas.Telemetry.Regression.InvalidWheelRejected",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)

bool FTATelemetryRegressionInvalidWheelTest::RunTest(
    const FString& Parameters)
{
    FTATelemetryRegressionConfig Config;
    Config.ScenarioId =
        TEXT("BadWheel");

    FTATelemetryMetricEnvelope Envelope;
    Envelope.Metric =
        ETATelemetryMetric::WheelVerticalLoadN;
    Envelope.WheelIndex =
        TAPrototypeTelemetryWheelCount;

    Config.Envelopes.Add(
        Envelope);

    TestFalse(
        TEXT("Out-of-range wheel metric config is rejected"),
        TATelemetryRegression::ValidateConfig(
            Config));

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTATelemetryRegressionFunctionalDamageMetricsTest,
    "TorqueAtlas.Telemetry.Regression.FunctionalDamageMetrics",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)

bool FTATelemetryRegressionFunctionalDamageMetricsTest::RunTest(
    const FString& Parameters)
{
    FTATelemetryRingBuffer Buffer;

    TestTrue(
        TEXT("Damage regression buffer initializes"),
        Buffer.Initialize(4));

    for (int32 Index = 0;
         Index < 4;
         ++Index)
    {
        FTAVehicleTelemetrySample Sample;
        Sample.PhysicsConfigHash = 2468u;

        Sample.SteeringRackDamage01 =
            0.1 * static_cast<double>(Index);

        Sample.SteeringCommandAuthority01 =
            1.0 - 0.1 * static_cast<double>(Index);

        Sample.WheelHubDamage01[1] =
            0.2 * static_cast<double>(Index);

        Sample.WheelHubBrakeEfficiency01[1] =
            1.0 - 0.15 * static_cast<double>(Index);

        Sample.SuspensionSpringEfficiency01[0] =
            1.0 - 0.20 * static_cast<double>(Index);

        Sample.AntiRollLinkEfficiency01[0] =
            1.0 - 0.25 * static_cast<double>(Index);

        Sample.BrakeTemperatureC[0] =
            100.0 + 100.0 * static_cast<double>(Index);

        Sample.BrakeThermalTorqueFactor01[0] =
            1.0 - 0.10 * static_cast<double>(Index);

        Sample.ElectricalDamage01 =
            0.10 * static_cast<double>(Index);

        Sample.StarterEfficiency01 =
            1.0 - 0.20 * static_cast<double>(Index);

        Sample.FuelDeliveryEfficiency01 =
            1.0 - 0.25 * static_cast<double>(Index);

        Buffer.Push(Sample);
    }

    FTATelemetryRegressionConfig Config;
    Config.ScenarioId =
        TEXT("FunctionalDamageMetrics");
    Config.ExpectedPhysicsConfigHash =
        2468u;
    Config.MinimumRequiredSamples =
        4;

    FTATelemetryMetricEnvelope SteeringDamage;
    SteeringDamage.Metric =
        ETATelemetryMetric::SteeringRackDamage01;
    SteeringDamage.Statistic =
        ETATelemetryStatistic::Maximum;
    SteeringDamage.MinimumAllowed =
        0.29;
    SteeringDamage.MaximumAllowed =
        0.31;

    Config.Envelopes.Add(
        SteeringDamage);

    FTATelemetryMetricEnvelope SteeringAuthority;
    SteeringAuthority.Metric =
        ETATelemetryMetric::SteeringCommandAuthority01;
    SteeringAuthority.Statistic =
        ETATelemetryStatistic::Minimum;
    SteeringAuthority.MinimumAllowed =
        0.69;
    SteeringAuthority.MaximumAllowed =
        0.71;

    Config.Envelopes.Add(
        SteeringAuthority);

    FTATelemetryMetricEnvelope HubDamage;
    HubDamage.Metric =
        ETATelemetryMetric::WheelHubDamage01;
    HubDamage.Statistic =
        ETATelemetryStatistic::Maximum;
    HubDamage.WheelIndex =
        1;
    HubDamage.MinimumAllowed =
        0.59;
    HubDamage.MaximumAllowed =
        0.61;

    Config.Envelopes.Add(
        HubDamage);

    FTATelemetryMetricEnvelope HubBrake;
    HubBrake.Metric =
        ETATelemetryMetric::WheelHubBrakeEfficiency01;
    HubBrake.Statistic =
        ETATelemetryStatistic::Minimum;
    HubBrake.WheelIndex =
        1;
    HubBrake.MinimumAllowed =
        0.54;
    HubBrake.MaximumAllowed =
        0.56;

    Config.Envelopes.Add(
        HubBrake);

    FTATelemetryMetricEnvelope SpringEfficiency;
    SpringEfficiency.Metric =
        ETATelemetryMetric::SuspensionSpringEfficiency01;
    SpringEfficiency.Statistic =
        ETATelemetryStatistic::Minimum;
    SpringEfficiency.WheelIndex =
        0;
    SpringEfficiency.MinimumAllowed =
        0.39;
    SpringEfficiency.MaximumAllowed =
        0.41;

    Config.Envelopes.Add(
        SpringEfficiency);

    FTATelemetryMetricEnvelope AntiRollEfficiency;
    AntiRollEfficiency.Metric =
        ETATelemetryMetric::AntiRollLinkEfficiency01;
    AntiRollEfficiency.Statistic =
        ETATelemetryStatistic::Minimum;
    AntiRollEfficiency.WheelIndex =
        0;
    AntiRollEfficiency.MinimumAllowed =
        0.24;
    AntiRollEfficiency.MaximumAllowed =
        0.26;

    Config.Envelopes.Add(
        AntiRollEfficiency);

    FTATelemetryMetricEnvelope BrakeTemperature;
    BrakeTemperature.Metric =
        ETATelemetryMetric::BrakeTemperatureC;
    BrakeTemperature.Statistic =
        ETATelemetryStatistic::Maximum;
    BrakeTemperature.WheelIndex =
        0;
    BrakeTemperature.MinimumAllowed =
        399.0;
    BrakeTemperature.MaximumAllowed =
        401.0;

    Config.Envelopes.Add(
        BrakeTemperature);

    FTATelemetryMetricEnvelope BrakeFade;
    BrakeFade.Metric =
        ETATelemetryMetric::BrakeThermalTorqueFactor01;
    BrakeFade.Statistic =
        ETATelemetryStatistic::Minimum;
    BrakeFade.WheelIndex =
        0;
    BrakeFade.MinimumAllowed =
        0.69;
    BrakeFade.MaximumAllowed =
        0.71;

    Config.Envelopes.Add(
        BrakeFade);

    FTATelemetryMetricEnvelope ElectricalDamage;
    ElectricalDamage.Metric =
        ETATelemetryMetric::ElectricalDamage01;
    ElectricalDamage.Statistic =
        ETATelemetryStatistic::Maximum;
    ElectricalDamage.MinimumAllowed =
        0.29;
    ElectricalDamage.MaximumAllowed =
        0.31;

    Config.Envelopes.Add(
        ElectricalDamage);

    FTATelemetryMetricEnvelope StarterEfficiency;
    StarterEfficiency.Metric =
        ETATelemetryMetric::StarterEfficiency01;
    StarterEfficiency.Statistic =
        ETATelemetryStatistic::Minimum;
    StarterEfficiency.MinimumAllowed =
        0.39;
    StarterEfficiency.MaximumAllowed =
        0.41;

    Config.Envelopes.Add(
        StarterEfficiency);

    FTATelemetryMetricEnvelope FuelDeliveryEfficiency;
    FuelDeliveryEfficiency.Metric =
        ETATelemetryMetric::FuelDeliveryEfficiency01;
    FuelDeliveryEfficiency.Statistic =
        ETATelemetryStatistic::Minimum;
    FuelDeliveryEfficiency.MinimumAllowed =
        0.24;
    FuelDeliveryEfficiency.MaximumAllowed =
        0.26;

    Config.Envelopes.Add(
        FuelDeliveryEfficiency);

    FTATelemetryRegressionResult Result;

    TestTrue(
        TEXT("Functional damage regression evaluates"),
        TATelemetryRegression::Evaluate(
            Buffer,
            Config,
            Result));

    TestTrue(
        TEXT("Functional damage metrics pass expected envelopes"),
        Result.bPassed);

    TestEqual(
        TEXT("All eleven functional/thermal envelopes pass"),
        Result.PassedEnvelopeCount,
        11);

    return true;
}

#endif
