#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TARegressionEnvelope.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTARegressionEnvelopeSummaryTest,
    "TorqueAtlas.Telemetry.RegressionEnvelope.Summary",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTARegressionEnvelopeSummaryTest::RunTest(const FString& Parameters)
{
    const TArray<double> Samples = { 0.0, 2.0, 4.0, 6.0, 8.0 };
    FTARegressionMetricSummary Summary;

    TestTrue(TEXT("Summary succeeds"),
        TARegressionEnvelope::Summarize(Samples, 0.4, Summary));
    TestEqual(TEXT("Sample count"), Summary.SampleCount, 5);
    TestEqual(TEXT("Minimum"), Summary.MinValue, 0.0);
    TestEqual(TEXT("Maximum"), Summary.MaxValue, 8.0);
    TestEqual(TEXT("Mean"), Summary.MeanValue, 4.0);
    TestEqual(TEXT("Trailing steady-state mean"), Summary.SteadyStateMean, 7.0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTARegressionEnvelopePassFailTest,
    "TorqueAtlas.Telemetry.RegressionEnvelope.PassFail",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTARegressionEnvelopePassFailTest::RunTest(const FString& Parameters)
{
    const TArray<double> Samples = { 1.0, 2.0, 3.0, 4.0 };

    FTARegressionRange MinRange;
    MinRange.MinInclusive = 0.5;
    MinRange.MaxInclusive = 1.5;

    FTARegressionRange MaxRange;
    MaxRange.MinInclusive = 3.5;
    MaxRange.MaxInclusive = 4.5;

    FTARegressionRange SteadyRange;
    SteadyRange.MinInclusive = 3.0;
    SteadyRange.MaxInclusive = 4.0;

    const FTARegressionMetricResult Pass =
        TARegressionEnvelope::Evaluate(
            Samples, 0.5, MinRange, MaxRange, SteadyRange);

    TestTrue(TEXT("Expected envelope passes"), Pass.bPassed);

    SteadyRange.MaxInclusive = 3.25;
    const FTARegressionMetricResult Fail =
        TARegressionEnvelope::Evaluate(
            Samples, 0.5, MinRange, MaxRange, SteadyRange);

    TestFalse(TEXT("Out-of-range steady state fails"), Fail.bPassed);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTARegressionEnvelopeRejectsInvalidTest,
    "TorqueAtlas.Telemetry.RegressionEnvelope.RejectsInvalidInput",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTARegressionEnvelopeRejectsInvalidTest::RunTest(const FString& Parameters)
{
    FTARegressionMetricSummary Summary;
    const TArray<double> Empty;
    TestFalse(TEXT("Empty trace rejected"),
        TARegressionEnvelope::Summarize(Empty, 0.25, Summary));

    const TArray<double> Samples = { 1.0, 2.0 };
    TestFalse(TEXT("Zero steady-state fraction rejected"),
        TARegressionEnvelope::Summarize(Samples, 0.0, Summary));
    TestFalse(TEXT("Steady-state fraction above one rejected"),
        TARegressionEnvelope::Summarize(Samples, 1.01, Summary));
    return true;
}

#endif
