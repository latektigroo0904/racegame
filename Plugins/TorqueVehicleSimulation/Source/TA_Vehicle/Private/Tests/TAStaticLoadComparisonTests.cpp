#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TAStaticLoadComparison.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAStaticLoadComparisonExactMatchTest,
    "TorqueAtlas.Vehicle.StaticLoadComparison.ExactMatch",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAStaticLoadComparisonExactMatchTest::RunTest(const FString& Parameters)
{
    FTAStaticMassBalanceOutput Expected;
    Expected.TotalWeightN = 10000.0;
    Expected.FrontAxleLoadN = 5500.0;
    Expected.RearAxleLoadN = 4500.0;
    Expected.FrontLeftLoadN = 2750.0;
    Expected.FrontRightLoadN = 2750.0;
    Expected.RearLeftLoadN = 2250.0;
    Expected.RearRightLoadN = 2250.0;

    FTAStaticLoadMeasuredMean Measured;
    Measured.FrontLeftLoadN = 2750.0;
    Measured.FrontRightLoadN = 2750.0;
    Measured.RearLeftLoadN = 2250.0;
    Measured.RearRightLoadN = 2250.0;

    FTAStaticLoadComparison Result;
    TestTrue(TEXT("Comparison succeeds"), TAStaticLoadComparison::Calculate(Expected, Measured, Result));
    TestEqual(TEXT("Total error"), Result.TotalErrorN, 0.0);
    TestEqual(TEXT("Front error"), Result.FrontAxleErrorN, 0.0);
    TestEqual(TEXT("Rear error"), Result.RearAxleErrorN, 0.0);
    TestEqual(TEXT("Left error"), Result.LeftSideErrorN, 0.0);
    TestEqual(TEXT("Right error"), Result.RightSideErrorN, 0.0);
    TestEqual(TEXT("Corner max"), Result.MaxAbsCornerErrorN, 0.0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAStaticLoadComparisonSignedAggregationTest,
    "TorqueAtlas.Vehicle.StaticLoadComparison.SignedAggregation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAStaticLoadComparisonSignedAggregationTest::RunTest(const FString& Parameters)
{
    FTAStaticMassBalanceOutput Expected;
    Expected.TotalWeightN = 10000.0;
    Expected.FrontAxleLoadN = 5500.0;
    Expected.RearAxleLoadN = 4500.0;
    Expected.FrontLeftLoadN = 2750.0;
    Expected.FrontRightLoadN = 2750.0;
    Expected.RearLeftLoadN = 2250.0;
    Expected.RearRightLoadN = 2250.0;

    FTAStaticLoadMeasuredMean Measured;
    Measured.FrontLeftLoadN = 2850.0;
    Measured.FrontRightLoadN = 2800.0;
    Measured.RearLeftLoadN = 2150.0;
    Measured.RearRightLoadN = 2200.0;

    FTAStaticLoadComparison Result;
    TestTrue(TEXT("Comparison succeeds"), TAStaticLoadComparison::Calculate(Expected, Measured, Result));
    TestEqual(TEXT("Total conserved"), Result.TotalErrorN, 0.0);
    TestEqual(TEXT("Front axle +150 N"), Result.FrontAxleErrorN, 150.0);
    TestEqual(TEXT("Rear axle -150 N"), Result.RearAxleErrorN, -150.0);
    TestEqual(TEXT("Left side conserved"), Result.LeftSideErrorN, 0.0);
    TestEqual(TEXT("Right side conserved"), Result.RightSideErrorN, 0.0);
    TestEqual(TEXT("Maximum corner error"), Result.MaxAbsCornerErrorN, 100.0);
    TestTrue(TEXT("Front relative error positive"), Result.FrontAxleErrorFraction > 0.0);
    TestTrue(TEXT("Rear relative error negative"), Result.RearAxleErrorFraction < 0.0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAStaticLoadComparisonRejectsInvalidEvidenceTest,
    "TorqueAtlas.Vehicle.StaticLoadComparison.RejectsInvalidEvidence",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAStaticLoadComparisonRejectsInvalidEvidenceTest::RunTest(const FString& Parameters)
{
    FTAStaticMassBalanceOutput Expected;
    Expected.TotalWeightN = 10000.0;
    Expected.FrontAxleLoadN = 5500.0;
    Expected.RearAxleLoadN = 4500.0;
    Expected.FrontLeftLoadN = 2750.0;
    Expected.FrontRightLoadN = 2750.0;
    Expected.RearLeftLoadN = 2250.0;
    Expected.RearRightLoadN = 2250.0;

    FTAStaticLoadMeasuredMean Measured;
    Measured.FrontLeftLoadN = -1.0;

    FTAStaticLoadComparison Result;
    Result.TotalErrorN = 123.0;
    TestFalse(TEXT("Negative measured support load rejected"), TAStaticLoadComparison::Calculate(Expected, Measured, Result));
    TestEqual(TEXT("Failure zeroes output"), Result.TotalErrorN, 0.0);

    Expected.FrontAxleLoadN = 5000.0; // inconsistent with corner sum / total
    Measured.FrontLeftLoadN = 2750.0;
    Measured.FrontRightLoadN = 2750.0;
    Measured.RearLeftLoadN = 2250.0;
    Measured.RearRightLoadN = 2250.0;
    TestFalse(TEXT("Inconsistent analytical oracle rejected"), TAStaticLoadComparison::Calculate(Expected, Measured, Result));
    return true;
}

#endif
