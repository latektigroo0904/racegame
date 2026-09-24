#include "Misc/AutomationTest.h"
#include "TAStaticLoadAcceptance.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTAStaticLoadAcceptancePassTest,
    "TorqueAtlas.Vehicle.StaticLoad.Acceptance.PassAndBoundary",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAStaticLoadAcceptancePassTest::RunTest(const FString& Parameters)
{
    FTAStaticLoadAcceptanceEnvelope Envelope;
    FTAStaticLoadComparison C;
    C.TotalErrorFraction = -Envelope.MaxAbsTotalErrorFraction;
    C.FrontAxleErrorFraction = Envelope.MaxAbsAxleErrorFraction;
    C.RearAxleErrorFraction = -Envelope.MaxAbsAxleErrorFraction;
    C.LeftSideErrorFraction = Envelope.MaxAbsSideErrorFraction;
    C.RightSideErrorFraction = -Envelope.MaxAbsSideErrorFraction;
    C.MaxAbsCornerErrorFractionOfTotal = Envelope.MaxAbsCornerErrorFractionOfTotal;

    FTAStaticLoadAcceptanceResult Result;
    TestTrue(TEXT("valid evidence evaluates"), TAStaticLoadAcceptance::Evaluate(Envelope, C, Result));
    TestTrue(TEXT("threshold equality passes"), Result.bPass);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTAStaticLoadAcceptanceFailureTest,
    "TorqueAtlas.Vehicle.StaticLoad.Acceptance.ComponentFailure",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAStaticLoadAcceptanceFailureTest::RunTest(const FString& Parameters)
{
    FTAStaticLoadAcceptanceEnvelope Envelope;
    FTAStaticLoadComparison C;
    C.FrontAxleErrorFraction = Envelope.MaxAbsAxleErrorFraction + 0.0001;

    FTAStaticLoadAcceptanceResult Result;
    TestTrue(TEXT("valid evidence evaluates"), TAStaticLoadAcceptance::Evaluate(Envelope, C, Result));
    TestFalse(TEXT("front axle fails"), Result.bFrontAxlePass);
    TestFalse(TEXT("aggregate fails"), Result.bPass);
    TestTrue(TEXT("unrelated total passes"), Result.bTotalPass);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTAStaticLoadAcceptanceInvalidTest,
    "TorqueAtlas.Vehicle.StaticLoad.Acceptance.InvalidInput",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAStaticLoadAcceptanceInvalidTest::RunTest(const FString& Parameters)
{
    FTAStaticLoadAcceptanceEnvelope Envelope;
    Envelope.MaxAbsTotalErrorFraction = -1.0;
    FTAStaticLoadComparison C;
    FTAStaticLoadAcceptanceResult Result;
    Result.bPass = true;
    TestFalse(TEXT("negative threshold rejected"), TAStaticLoadAcceptance::Evaluate(Envelope, C, Result));
    TestFalse(TEXT("failure clears output"), Result.bPass);
    return true;
}

#endif
