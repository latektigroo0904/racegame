#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TAWheelContactResolver.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAWheelContactLoadBridgeTest,
    "TorqueAtlas.Suspension.Contact.VerticalLoadBridgePreservesSupportMagnitude",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAWheelContactLoadBridgeTest::RunTest(const FString& Parameters)
{
    FTAResolvedWheelContact Contact;
    Contact.VerticalLoadN = 4321.25;
    Contact.LongitudinalVelocityMps = 12.5;
    Contact.LateralVelocityMps = -0.75;
    Contact.SuspensionForceWorldN = FVector3d(0.0, 0.0, 4321.25);

    const FTAWheelContactInput Input =
        TAWheelContactResolver::BuildVehicleWheelContactInput(Contact);

    TestEqual(
        TEXT("Vertical support magnitude crosses bridge unchanged"),
        Input.VerticalLoadN,
        Contact.VerticalLoadN);

    TestEqual(
        TEXT("Suspension world force crosses bridge unchanged"),
        Input.SuspensionForceWorldN,
        Contact.SuspensionForceWorldN);

    TestTrue(
        TEXT("Bridge never negates positive support load"),
        Input.VerticalLoadN > 0.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAWheelContactAntiRollConservationTest,
    "TorqueAtlas.Suspension.Contact.AntiRollConservesPairLoadBeforeClamp",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAWheelContactAntiRollConservationTest::RunTest(const FString& Parameters)
{
    FTAAntiRollBarConfig Config;
    Config.CouplingRateNPerM = 12000.0;
    Config.MaxTransferForceN = 3500.0;

    FTAResolvedWheelContact Left;
    Left.bInContact = true;
    Left.TravelM = 0.04;
    Left.VerticalLoadN = 5000.0;
    Left.RoadNormalWorld = FVector3d(0.0, 0.0, 1.0);

    FTAResolvedWheelContact Right;
    Right.bInContact = true;
    Right.TravelM = -0.02;
    Right.VerticalLoadN = 5000.0;
    Right.RoadNormalWorld = FVector3d(0.0, 0.0, 1.0);

    const double BeforeN = Left.VerticalLoadN + Right.VerticalLoadN;

    TAWheelContactResolver::ApplyAntiRollBarToPair(
        Config,
        Left,
        Right);

    const double AfterN = Left.VerticalLoadN + Right.VerticalLoadN;

    TestTrue(
        TEXT("Anti-roll transfer changes the individual wheel loads"),
        !FMath::IsNearlyEqual(Left.VerticalLoadN, Right.VerticalLoadN, 1.0e-9));

    TestTrue(
        TEXT("Unclamped anti-roll transfer conserves pair support load"),
        FMath::IsNearlyEqual(BeforeN, AfterN, 1.0e-9));

    TestTrue(
        TEXT("Left world force remains aligned with road normal"),
        FVector3d::DotProduct(
            Left.SuspensionForceWorldN,
            Left.RoadNormalWorld) > 0.0);

    TestTrue(
        TEXT("Right world force remains aligned with road normal"),
        FVector3d::DotProduct(
            Right.SuspensionForceWorldN,
            Right.RoadNormalWorld) > 0.0);

    TestTrue(
        TEXT("Left force magnitude equals positive support load"),
        FMath::IsNearlyEqual(
            Left.SuspensionForceWorldN.Length(),
            Left.VerticalLoadN,
            1.0e-9));

    TestTrue(
        TEXT("Right force magnitude equals positive support load"),
        FMath::IsNearlyEqual(
            Right.SuspensionForceWorldN.Length(),
            Right.VerticalLoadN,
            1.0e-9));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAWheelContactAntiRollAirborneZeroTest,
    "TorqueAtlas.Suspension.Contact.AntiRollDoesNotCreateAirborneSupport",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAWheelContactAntiRollAirborneZeroTest::RunTest(const FString& Parameters)
{
    FTAAntiRollBarConfig Config;

    FTAResolvedWheelContact Left;
    Left.bInContact = false;
    Left.TravelM = 0.05;
    Left.VerticalLoadN = 1234.0;
    Left.SuspensionForceWorldN = FVector3d(0.0, 0.0, 1234.0);

    FTAResolvedWheelContact Right;
    Right.bInContact = true;
    Right.TravelM = -0.01;
    Right.VerticalLoadN = 5000.0;
    Right.RoadNormalWorld = FVector3d(0.0, 0.0, 1.0);

    TAWheelContactResolver::ApplyAntiRollBarToPair(
        Config,
        Left,
        Right);

    TestTrue(
        TEXT("Airborne wheel support load is forced to zero"),
        FMath::IsNearlyZero(Left.VerticalLoadN));

    TestTrue(
        TEXT("Airborne wheel world force is forced to zero"),
        Left.SuspensionForceWorldN.IsNearlyZero());

    return true;
}

#endif
