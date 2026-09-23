#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TATelemetryRegression.h"

namespace
{
    FTATelemetryRingBuffer MakeProfileTrace()
    {
        FTATelemetryRingBuffer Buffer;
        Buffer.Initialize(8);

        for (int32 Index = 0;
             Index < 4;
             ++Index)
        {
            FTAVehicleTelemetrySample Sample;

            Sample.SimulationTick =
                static_cast<uint64>(Index);

            Sample.PhysicsConfigHash =
                55u;

            Sample.ChassisLinearVelocityWorldMps.X =
                static_cast<double>(Index);

            Sample.WheelVerticalLoadN[1] =
                3000.0
                + 100.0
                * static_cast<double>(Index);

            Buffer.Push(
                Sample);
        }

        return Buffer;
    }

    FTATelemetryScenarioProfileConfig MakeProfileConfig()
    {
        FTATelemetryScenarioProfileConfig Config;

        Config.ScenarioId =
            TEXT("ProfileSmoke");

        Config.BaselineVersion =
            2;

        Config.bTrustedBaseline =
            true;

        Config.ExpectedPhysicsConfigHash =
            55u;

        Config.MinimumRequiredSamples =
            4;

        FTATelemetryMetricProfile Speed;

        Speed.Metric =
            ETATelemetryMetric::ChassisForwardSpeedMps;

        Speed.StartFraction01 =
            0.0;

        Speed.EndFraction01 =
            1.0;

        Speed.SteadyStateFraction01 =
            0.5;

        Speed.MinimumEnvelope.MinInclusive =
            -0.1;

        Speed.MinimumEnvelope.MaxInclusive =
            0.1;

        Speed.MaximumEnvelope.MinInclusive =
            2.9;

        Speed.MaximumEnvelope.MaxInclusive =
            3.1;

        Speed.SteadyStateEnvelope.MinInclusive =
            2.4;

        Speed.SteadyStateEnvelope.MaxInclusive =
            2.6;

        Config.Metrics.Add(
            Speed);

        FTATelemetryMetricProfile Load;

        Load.Metric =
            ETATelemetryMetric::WheelVerticalLoadN;

        Load.WheelIndex =
            1;

        Load.SteadyStateFraction01 =
            0.5;

        Load.MinimumEnvelope.MinInclusive =
            2990.0;

        Load.MinimumEnvelope.MaxInclusive =
            3010.0;

        Load.MaximumEnvelope.MinInclusive =
            3290.0;

        Load.MaximumEnvelope.MaxInclusive =
            3310.0;

        Load.SteadyStateEnvelope.MinInclusive =
            3240.0;

        Load.SteadyStateEnvelope.MaxInclusive =
            3260.0;

        Config.Metrics.Add(
            Load);

        return Config;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTATelemetryScenarioProfilePassTest,
    "TorqueAtlas.Telemetry.Profile.PassAndSummary",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)

bool FTATelemetryScenarioProfilePassTest::RunTest(
    const FString& Parameters)
{
    const FTATelemetryRingBuffer Buffer =
        MakeProfileTrace();

    const FTATelemetryScenarioProfileConfig Config =
        MakeProfileConfig();

    FTATelemetryScenarioProfileResult Result;

    TestTrue(
        TEXT("Profile evaluation succeeds"),
        TATelemetryRegression::EvaluateProfile(
            Buffer,
            Config,
            Result));

    TestTrue(
        TEXT("Profile passes"),
        Result.bPassed);

    TestEqual(
        TEXT("Profile baseline version retained"),
        Result.BaselineVersion,
        2);

    TestTrue(
        TEXT("Trusted state retained"),
        Result.bTrustedBaseline);

    TestEqual(
        TEXT("Observed hash retained"),
        Result.ObservedPhysicsConfigHash,
        55u);

    TestEqual(
        TEXT("Two metric reports emitted"),
        Result.MetricResults.Num(),
        2);

    const FTARegressionMetricSummary& SpeedSummary =
        Result.MetricResults[0].Evaluation.Summary;

    TestEqual(
        TEXT("Speed minimum"),
        SpeedSummary.MinValue,
        0.0);

    TestEqual(
        TEXT("Speed maximum"),
        SpeedSummary.MaxValue,
        3.0);

    TestEqual(
        TEXT("Speed mean"),
        SpeedSummary.MeanValue,
        1.5);

    TestEqual(
        TEXT("Speed trailing steady-state mean"),
        SpeedSummary.SteadyStateMean,
        2.5);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTATelemetryScenarioProfileReportExportTest,
    "TorqueAtlas.Telemetry.Profile.MachineReadableReport",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)

bool FTATelemetryScenarioProfileReportExportTest::RunTest(
    const FString& Parameters)
{
    const FTATelemetryRingBuffer Buffer =
        MakeProfileTrace();

    const FTATelemetryScenarioProfileConfig Config =
        MakeProfileConfig();

    FTATelemetryScenarioProfileResult Result;

    TestTrue(
        TEXT("Profile evaluation succeeds"),
        TATelemetryRegression::EvaluateProfile(
            Buffer,
            Config,
            Result));

    const FString Csv =
        TATelemetryRegression::ExportProfileCsv(
            Result);

    const FString JsonLines =
        TATelemetryRegression::ExportProfileJsonLines(
            Result);

    TestTrue(
        TEXT("Profile CSV includes observed minimum column"),
        Csv.Contains(
            TEXT("observed_min")));

    TestTrue(
        TEXT("Profile CSV includes steady-state column"),
        Csv.Contains(
            TEXT("observed_steady_state")));

    TestTrue(
        TEXT("Profile JSONL includes scenario ID"),
        JsonLines.Contains(
            TEXT("\"scenario_id\":\"ProfileSmoke\"")));

    TestTrue(
        TEXT("Profile JSONL includes observed maximum"),
        JsonLines.Contains(
            TEXT("\"observed_max\":3")));

    TestTrue(
        TEXT("Profile JSONL includes expected steady-state range"),
        JsonLines.Contains(
            TEXT("\"expected_steady_state\"")));

    TestTrue(
        TEXT("Profile JSONL marks trusted baseline"),
        JsonLines.Contains(
            TEXT("\"trusted_baseline\":true")));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTATelemetryScenarioProfileTrustedHashTest,
    "TorqueAtlas.Telemetry.Profile.TrustedRequiresHash",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)

bool FTATelemetryScenarioProfileTrustedHashTest::RunTest(
    const FString& Parameters)
{
    FTATelemetryScenarioProfileConfig Config =
        MakeProfileConfig();

    Config.ExpectedPhysicsConfigHash =
        0u;

    TestFalse(
        TEXT("Trusted profile without config hash is rejected"),
        TATelemetryRegression::ValidateProfileConfig(
            Config));

    Config.bTrustedBaseline =
        false;

    TestTrue(
        TEXT("Provisional profile may omit config hash"),
        TATelemetryRegression::ValidateProfileConfig(
            Config));

    return true;
}

#endif
