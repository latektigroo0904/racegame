#include "Misc/AutomationTest.h"
#include "TAAeroRegressionReport.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAAeroRegressionReportBuildTest,
    "TorqueAtlas.Telemetry.Aerodynamics.Report.BuildsAppliedMetrics",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAAeroRegressionReportBuildTest::RunTest(const FString& Parameters)
{
    FTACompactTelemetryRingBuffer Buffer;
    Buffer.Initialize(4);

    for (int32 Index = 0; Index < 4; ++Index)
    {
        FTATelemetrySample Sample;
        Sample.AeroRelativeAirSpeedMps = 10.0 + Index;
        Sample.AeroDynamicPressurePa = 60.0 + 10.0 * Index;
        Sample.AeroForceWorldN = FVector3d(-100.0 - 10.0 * Index, 0.0, -200.0 - 20.0 * Index);
        Sample.AeroTorqueWorldNm = FVector3d(0.0, 5.0 + Index, 0.0);
        Buffer.Push(Sample);
    }

    FTAAeroRegressionReport Report;
    const bool bBuilt = TAAeroRegressionReport::Build(
        Buffer,
        FVector3d(1.0, 0.0, 0.0),
        FVector3d(0.0, 0.0, 1.0),
        0.5,
        Report);

    TestTrue(TEXT("Report builds"), bBuilt);
    TestEqual(TEXT("Sample count"), Report.SampleCount, 4);
    TestEqual(TEXT("Air-speed min"), Report.RelativeAirSpeedMps.MinValue, 10.0);
    TestEqual(TEXT("Air-speed max"), Report.RelativeAirSpeedMps.MaxValue, 13.0);
    TestEqual(TEXT("Air-speed mean"), Report.RelativeAirSpeedMps.MeanValue, 11.5);
    TestEqual(TEXT("Air-speed trailing mean"), Report.RelativeAirSpeedMps.SteadyStateMean, 12.5);
    TestEqual(TEXT("Drag is positive against forward axis"), Report.DragAxisForceN.MeanValue, 115.0);
    TestEqual(TEXT("Downforce remains negative on +up"), Report.VerticalForceN.MeanValue, -230.0);
    TestEqual(TEXT("Pitch torque follows right axis"), Report.PitchTorqueNm.MeanValue, 6.5);

    const FString Json = TAAeroRegressionReport::ExportJsonLine(Report);
    TestTrue(TEXT("JSON contains sample count"), Json.Contains(TEXT("\"sample_count\":4")));
    TestTrue(TEXT("JSON contains drag channel"), Json.Contains(TEXT("\"drag_axis_force_n\"")));

    const FString Csv = TAAeroRegressionReport::ExportCsv(Report);
    TestTrue(TEXT("CSV contains airspeed header"), Csv.Contains(TEXT("airspeed_mean")));
    TestTrue(TEXT("CSV contains pitch header"), Csv.Contains(TEXT("pitch_steady")));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAAeroRegressionReportRejectsInvalidFrameTest,
    "TorqueAtlas.Telemetry.Aerodynamics.Report.RejectsInvalidScenarioFrame",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAAeroRegressionReportRejectsInvalidFrameTest::RunTest(const FString& Parameters)
{
    FTACompactTelemetryRingBuffer Buffer;
    Buffer.Initialize(1);
    Buffer.Push(FTATelemetrySample{});

    FTAAeroRegressionReport Report;
    TestFalse(
        TEXT("Parallel forward/up axes are rejected by metric extraction"),
        TAAeroRegressionReport::Build(
            Buffer,
            FVector3d(1.0, 0.0, 0.0),
            FVector3d(1.0, 0.0, 0.0),
            1.0,
            Report));

    return true;
}

#endif
