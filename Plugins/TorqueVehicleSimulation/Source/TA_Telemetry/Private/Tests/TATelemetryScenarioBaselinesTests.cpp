#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TATelemetryScenarioBaselines.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTATelemetryScenarioBaselineSetTest,
    "TorqueAtlas.Telemetry.Scenarios.BuildAllProvisional",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)

bool FTATelemetryScenarioBaselineSetTest::RunTest(
    const FString& Parameters)
{
    constexpr uint32 PhysicsHash =
        0xA1B2C3D4u;

    TArray<FTATelemetryScenarioBaseline> Baselines;

    TATelemetryScenarioBaselines::BuildAllProvisional(
        PhysicsHash,
        Baselines);

    TestEqual(
        TEXT("Six first proving-ground scenarios exist"),
        Baselines.Num(),
        6);

    TSet<FName> ScenarioIds;

    for (const FTATelemetryScenarioBaseline& Baseline :
         Baselines)
    {
        TestTrue(
            TEXT("Each provisional baseline validates"),
            TATelemetryScenarioBaselines::ValidateBaseline(
                Baseline));

        TestFalse(
            TEXT("Provisional baseline is never marked trusted"),
            Baseline.bTrustedBaseline);

        TestEqual(
            TEXT("Expected physics hash propagates into scenario"),
            Baseline.Regression.ExpectedPhysicsConfigHash,
            PhysicsHash);

        TestTrue(
            TEXT("Scenario contains at least one metric envelope"),
            Baseline.Regression.Envelopes.Num() > 0);

        TestTrue(
            TEXT("Scenario contains at least one rich profile metric"),
            Baseline.Profile.Metrics.Num() > 0);

        TestEqual(
            TEXT("Profile expected physics hash matches envelope config"),
            Baseline.Profile.ExpectedPhysicsConfigHash,
            Baseline.Regression.ExpectedPhysicsConfigHash);

        TestFalse(
            TEXT("Scenario ID is unique"),
            ScenarioIds.Contains(
                Baseline.ScenarioId));

        ScenarioIds.Add(
            Baseline.ScenarioId);
    }

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTATelemetryScenarioTrustedHashRuleTest,
    "TorqueAtlas.Telemetry.Scenarios.TrustedRequiresHash",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)

bool FTATelemetryScenarioTrustedHashRuleTest::RunTest(
    const FString& Parameters)
{
    FTATelemetryScenarioBaseline Baseline;

    TestTrue(
        TEXT("Provisional static-settle baseline builds without required hash"),
        TATelemetryScenarioBaselines::BuildProvisional(
            ETARegressionScenarioKind::StaticSettle,
            0u,
            Baseline));

    Baseline.bTrustedBaseline =
        true;

    Baseline.Profile.bTrustedBaseline =
        true;

    TestFalse(
        TEXT("Trusted baseline without physics hash is rejected"),
        TATelemetryScenarioBaselines::ValidateBaseline(
            Baseline));

    Baseline.Regression.ExpectedPhysicsConfigHash =
        123u;

    Baseline.Profile.ExpectedPhysicsConfigHash =
        123u;

    TestTrue(
        TEXT("Trusted baseline with explicit physics hash validates structurally"),
        TATelemetryScenarioBaselines::ValidateBaseline(
            Baseline));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTATelemetryScenarioCrashCoverageTest,
    "TorqueAtlas.Telemetry.Scenarios.CrashIncludesDamageHandlingMetrics",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)

bool FTATelemetryScenarioCrashCoverageTest::RunTest(
    const FString& Parameters)
{
    FTATelemetryScenarioBaseline Baseline;

    TestTrue(
        TEXT("Synthetic crash baseline builds"),
        TATelemetryScenarioBaselines::BuildProvisional(
            ETARegressionScenarioKind::SyntheticFrontCornerCrash,
            4321u,
            Baseline));

    bool bHasToe = false;
    bool bHasCamber = false;
    bool bHasTireForce = false;
    bool bHasCooling = false;
    bool bHasSteeringDamage = false;
    bool bHasHubDamage = false;

    for (const FTATelemetryMetricEnvelope& Envelope :
         Baseline.Regression.Envelopes)
    {
        bHasToe |=
            Envelope.Metric ==
                ETATelemetryMetric::WheelToeRad;

        bHasCamber |=
            Envelope.Metric ==
                ETATelemetryMetric::WheelCamberRad;

        bHasTireForce |=
            Envelope.Metric ==
                ETATelemetryMetric::TireLateralForceN;

        bHasCooling |=
            Envelope.Metric ==
                ETATelemetryMetric::CoolingEfficiency01;

        bHasSteeringDamage |=
            Envelope.Metric ==
                ETATelemetryMetric::SteeringRackDamage01;

        bHasHubDamage |=
            Envelope.Metric ==
                ETATelemetryMetric::WheelHubDamage01;
    }

    TestTrue(
        TEXT("Crash scenario watches wheel toe"),
        bHasToe);

    TestTrue(
        TEXT("Crash scenario watches wheel camber"),
        bHasCamber);

    TestTrue(
        TEXT("Crash scenario watches tire force"),
        bHasTireForce);

    TestTrue(
        TEXT("Crash scenario watches cooling consequence"),
        bHasCooling);

    TestTrue(
        TEXT("Crash scenario watches steering-rack functional damage"),
        bHasSteeringDamage);

    TestTrue(
        TEXT("Crash scenario watches wheel-hub functional damage"),
        bHasHubDamage);

    return true;
}

#endif
