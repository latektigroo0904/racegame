#include "Misc/AutomationTest.h"
#include "TAStaticLoadSettledSampler.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
    constexpr double ExpectedTotalN = 10000.0;

    FTAStaticLoadSettledConfig TestConfig()
    {
        FTAStaticLoadSettledConfig Config;
        Config.MaxLinearSpeedMps = 0.02;
        Config.MaxAngularSpeedRadPerSec = 0.01;
        Config.MaxCornerLoadDeltaFractionOfExpectedTotal = 0.001; // 10 N
        Config.MinimumQualifiedSamples = 3;
        return Config;
    }

    FTAStaticLoadSample Sample(const double FL = 2500.0, const double FR = 2500.0, const double RL = 2500.0, const double RR = 2500.0)
    {
        FTAStaticLoadSample S;
        S.FrontLeftLoadN = FL;
        S.FrontRightLoadN = FR;
        S.RearLeftLoadN = RL;
        S.RearRightLoadN = RR;
        return S;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTAStaticLoadSettledWarmupTest,
    "TorqueAtlas.Vehicle.StaticLoad.Settled.WarmupAndMean",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAStaticLoadSettledWarmupTest::RunTest(const FString& Parameters)
{
    const auto Config = TestConfig();
    FTAStaticLoadSettledState State;
    FTAStaticLoadMeasuredMean Mean;

    TestTrue(TEXT("sample 1 qualifies"), TAStaticLoadSettledSampler::PushSample(Config, ExpectedTotalN, Sample(2498, 2502, 2501, 2499), State));
    TestFalse(TEXT("one sample is insufficient"), TAStaticLoadSettledSampler::TryGetQualifiedMean(Config, State, Mean));
    TestTrue(TEXT("sample 2 qualifies"), TAStaticLoadSettledSampler::PushSample(Config, ExpectedTotalN, Sample(2500, 2500, 2500, 2500), State));
    TestFalse(TEXT("two samples are insufficient"), TAStaticLoadSettledSampler::TryGetQualifiedMean(Config, State, Mean));
    TestTrue(TEXT("sample 3 qualifies"), TAStaticLoadSettledSampler::PushSample(Config, ExpectedTotalN, Sample(2502, 2498, 2499, 2501), State));
    TestTrue(TEXT("minimum window yields mean"), TAStaticLoadSettledSampler::TryGetQualifiedMean(Config, State, Mean));
    TestEqual(TEXT("FL mean"), Mean.FrontLeftLoadN, 2500.0);
    TestEqual(TEXT("FR mean"), Mean.FrontRightLoadN, 2500.0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTAStaticLoadSettledMotionResetTest,
    "TorqueAtlas.Vehicle.StaticLoad.Settled.MotionReset",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAStaticLoadSettledMotionResetTest::RunTest(const FString& Parameters)
{
    const auto Config = TestConfig();
    FTAStaticLoadSettledState State;
    TAStaticLoadSettledSampler::PushSample(Config, ExpectedTotalN, Sample(), State);
    FTAStaticLoadSample Moving = Sample();
    Moving.ChassisLinearSpeedMps = Config.MaxLinearSpeedMps + 0.0001;
    TestFalse(TEXT("motion rejects sample"), TAStaticLoadSettledSampler::PushSample(Config, ExpectedTotalN, Moving, State));
    TestEqual(TEXT("motion clears window"), State.QualifiedSampleCount, 0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTAStaticLoadSettledInstabilityResetTest,
    "TorqueAtlas.Vehicle.StaticLoad.Settled.LoadInstabilityReset",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAStaticLoadSettledInstabilityResetTest::RunTest(const FString& Parameters)
{
    const auto Config = TestConfig();
    FTAStaticLoadSettledState State;
    TAStaticLoadSettledSampler::PushSample(Config, ExpectedTotalN, Sample(), State);
    TestFalse(TEXT("delta above 10 N resets"), TAStaticLoadSettledSampler::PushSample(Config, ExpectedTotalN, Sample(2510.01, 2500, 2500, 2500), State));
    TestEqual(TEXT("instability clears window"), State.QualifiedSampleCount, 0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTAStaticLoadSettledBoundaryTest,
    "TorqueAtlas.Vehicle.StaticLoad.Settled.ThresholdBoundaries",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAStaticLoadSettledBoundaryTest::RunTest(const FString& Parameters)
{
    const auto Config = TestConfig();
    FTAStaticLoadSettledState State;
    FTAStaticLoadSample First = Sample();
    First.ChassisLinearSpeedMps = Config.MaxLinearSpeedMps;
    First.ChassisAngularSpeedRadPerSec = Config.MaxAngularSpeedRadPerSec;
    TestTrue(TEXT("motion threshold equality qualifies"), TAStaticLoadSettledSampler::PushSample(Config, ExpectedTotalN, First, State));
    TestTrue(TEXT("load-delta threshold equality qualifies"), TAStaticLoadSettledSampler::PushSample(Config, ExpectedTotalN, Sample(2510, 2490, 2500, 2500), State));
    TestEqual(TEXT("boundary samples remain consecutive"), State.QualifiedSampleCount, 2);
    return true;
}

#endif
