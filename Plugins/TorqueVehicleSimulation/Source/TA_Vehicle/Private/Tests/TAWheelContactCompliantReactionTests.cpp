#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TAWheelContactResolver.h"

namespace
{
    FTADoubleWishboneSolverConfig MakeCompliantReactionGeometry()
    {
        FTADoubleWishboneSolverConfig Config;
        FTADoubleWishboneHardpoints& H = Config.Hardpoints;

        H.UpperInnerA = FVector3d(1.48, 0.38, -0.22);
        H.UpperInnerB = FVector3d(1.14, 0.38, -0.22);
        H.LowerInnerA = FVector3d(1.50, 0.35, -0.48);
        H.LowerInnerB = FVector3d(1.12, 0.35, -0.48);
        H.TieRodInner = FVector3d(1.12, 0.35, -0.38);
        H.DamperChassis = FVector3d(1.31, 0.40, -0.05);
        H.DamperLowerArmReference = FVector3d(1.31, 0.52, -0.43);
        H.UpperBallJointReference = FVector3d(1.31, 0.70, -0.25);
        H.LowerBallJointReference = FVector3d(1.31, 0.73, -0.50);
        H.TieRodOuterReference = FVector3d(1.12, 0.71, -0.39);
        H.WheelCenterReference = FVector3d(1.31, 0.775, -0.45);
        H.WheelForwardReference = FVector3d(1.0, 0.0, 0.0);
        H.WheelUpReference = FVector3d(0.0, 0.0, 1.0);
        H.SteeringRackAxisLocal = FVector3d(0.0, 1.0, 0.0);
        H.SideSign = 1.0;

        Config.MinTravelM = -0.07;
        Config.MaxTravelM = 0.09;
        Config.MaxIterations = 80;
        Config.PositionToleranceM = 0.0005;
        return Config;
    }

    FTASuspensionRuntimeConfig MakeCompliantReactionSuspension()
    {
        FTASuspensionRuntimeConfig Config;
        Config.SpringRateNPerM = 95000.0;
        Config.StaticSpringCompressionM = 0.08;
        Config.BumpDampingNsPerM = 4500.0;
        Config.ReboundDampingNsPerM = 6500.0;
        return Config;
    }

    FTARoadPlane MakeCompliantReactionRoad()
    {
        FTARoadPlane Road;
        Road.PointWorldM = FVector3d::ZeroVector;
        Road.NormalWorld = FVector3d(0.0, 0.0, 1.0);
        Road.Surface.Material = ETASurfaceMaterial::FreshAsphalt;
        Road.Surface.DryFrictionMultiplier = 1.0;
        return Road;
    }

    bool ResolveCompliant(
        const double AdditionalReactionN,
        FTAResolvedWheelContact& OutContact)
    {
        FTAChassisState Chassis;
        Chassis.PositionWorldM = FVector3d(0.0, 0.0, 0.777);

        FTADoubleWishboneState GeometryState;
        FTASuspensionRuntimeState SuspensionState;
        FTATireRuntimeState TireState;

        return TAWheelContactResolver::ResolveDoubleWishboneCompliantRoadContact(
            Chassis,
            MakeCompliantReactionGeometry(),
            MakeCompliantReactionSuspension(),
            FTATireRuntimeConfig{},
            0.0,
            AdditionalReactionN,
            FTADoubleWishboneDamageOffsets{},
            MakeCompliantReactionRoad(),
            1.0 / 240.0,
            GeometryState,
            SuspensionState,
            TireState,
            OutContact);
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAWheelContactCompliantZeroReactionDeterminismTest,
    "TorqueAtlas.Suspension.Contact.Compliant.ZeroReactionEquivalent",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAWheelContactCompliantZeroReactionDeterminismTest::RunTest(const FString& Parameters)
{
    FTAResolvedWheelContact A;
    FTAResolvedWheelContact B;

    TestTrue(TEXT("First zero-reaction solve succeeds"), ResolveCompliant(0.0, A));
    TestTrue(TEXT("Second zero-reaction solve succeeds"), ResolveCompliant(0.0, B));

    TestEqual(TEXT("Zero reaction preserves contact state"), A.bInContact, B.bInContact);
    TestTrue(TEXT("Zero reaction preserves solved travel"), FMath::IsNearlyEqual(A.TravelM, B.TravelM, 1.0e-9));
    TestTrue(TEXT("Zero reaction preserves vertical load"), FMath::IsNearlyEqual(A.VerticalLoadN, B.VerticalLoadN, 1.0e-6));
    TestTrue(TEXT("Zero reaction produces finite non-negative load"), FMath::IsFinite(A.VerticalLoadN) && A.VerticalLoadN >= 0.0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAWheelContactCompliantPositiveReactionTest,
    "TorqueAtlas.Suspension.Contact.Compliant.PositiveAdditionalReaction",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAWheelContactCompliantPositiveReactionTest::RunTest(const FString& Parameters)
{
    FTAResolvedWheelContact Baseline;
    FTAResolvedWheelContact Assisted;

    TestTrue(TEXT("Baseline compliant solve succeeds"), ResolveCompliant(0.0, Baseline));
    TestTrue(TEXT("Positive-reaction compliant solve succeeds"), ResolveCompliant(500.0, Assisted));

    TestTrue(TEXT("Baseline is in contact"), Baseline.bInContact);
    TestTrue(TEXT("Positive reaction remains in contact"), Assisted.bInContact);
    TestTrue(TEXT("Positive reaction increases supported load"), Assisted.VerticalLoadN > Baseline.VerticalLoadN + 1.0);
    TestTrue(TEXT("Positive reaction changes equilibrium travel"), FMath::Abs(Assisted.TravelM - Baseline.TravelM) > 1.0e-7);
    TestTrue(TEXT("Assisted load remains finite and non-negative"), FMath::IsFinite(Assisted.VerticalLoadN) && Assisted.VerticalLoadN >= 0.0);
    return true;
}

#endif
