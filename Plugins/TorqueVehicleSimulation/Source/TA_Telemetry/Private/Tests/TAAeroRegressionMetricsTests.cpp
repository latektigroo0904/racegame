#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TAAeroRegressionMetrics.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAAeroRegressionMetricsExtractionTest,
    "TorqueAtlas.Telemetry.Aerodynamics.RegressionMetrics.ExtractsAppliedScalars",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAAeroRegressionMetricsExtractionTest::RunTest(const FString& Parameters)
{
    FTATelemetrySample Sample;
    Sample.AeroRelativeAirSpeedMps = 40.0;
    Sample.AeroDynamicPressurePa = 980.0;
    Sample.AeroForceWorldN = FVector3d(-1200.0, 0.0, -600.0);
    Sample.AeroTorqueWorldNm = FVector3d(0.0, 75.0, 0.0);

    FTAAeroRegressionMetrics Metrics;
    const bool bValid = TAAeroRegressionMetrics::Extract(
        Sample,
        FVector3d(1.0, 0.0, 0.0),
        FVector3d(0.0, 0.0, 1.0),
        Metrics);

    TestTrue(TEXT("Canonical axes are accepted"), bValid);
    TestEqual(TEXT("Relative air speed propagates"), Metrics.RelativeAirSpeedMps, 40.0);
    TestEqual(TEXT("Dynamic pressure propagates"), Metrics.DynamicPressurePa, 980.0);
    TestEqual(TEXT("Drag scalar is positive when force opposes forward"), Metrics.DragAxisForceN, 1200.0);
    TestEqual(TEXT("Downforce remains negative on the up axis"), Metrics.VerticalForceN, -600.0);
    TestEqual(TEXT("Pitch torque projects onto right axis"), Metrics.PitchTorqueNm, 75.0);
    TestTrue(TEXT("Force magnitude is finite"), FMath::IsFinite(Metrics.ForceMagnitudeN));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAAeroRegressionMetricsRejectsInvalidFrameTest,
    "TorqueAtlas.Telemetry.Aerodynamics.RegressionMetrics.RejectsInvalidFrame",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAAeroRegressionMetricsRejectsInvalidFrameTest::RunTest(const FString& Parameters)
{
    FTATelemetrySample Sample;
    FTAAeroRegressionMetrics Metrics;

    TestFalse(
        TEXT("Parallel forward/up axes are rejected"),
        TAAeroRegressionMetrics::Extract(
            Sample,
            FVector3d(1.0, 0.0, 0.0),
            FVector3d(1.0, 0.0, 0.0),
            Metrics));

    return true;
}

#endif
