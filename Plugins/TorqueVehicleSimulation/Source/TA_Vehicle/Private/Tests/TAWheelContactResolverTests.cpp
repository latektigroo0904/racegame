#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TAWheelContactResolver.h"

namespace
{
    FTADoubleWishboneSolverConfig MakeContactFrontRightGeometry()
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

    FTASuspensionRuntimeConfig MakeContactSuspensionForceConfig()
    {
        FTASuspensionRuntimeConfig Config;

        // With ~0.47 motion ratio this produces roughly one quarter
        // of TA-P01 static vehicle weight at reference ride height.
        Config.SpringRateNPerM = 95000.0;
        Config.StaticSpringCompressionM = 0.08;

        Config.BumpDampingNsPerM = 4500.0;
        Config.ReboundDampingNsPerM = 6500.0;

        return Config;
    }

    FTARoadPlane MakeFlatDryRoad()
    {
        FTARoadPlane Road;
        Road.PointWorldM = FVector3d::ZeroVector;
        Road.NormalWorld = FVector3d(0.0, 0.0, 1.0);
        Road.Surface.Material = ETASurfaceMaterial::FreshAsphalt;
        Road.Surface.DryFrictionMultiplier = 1.0;
        return Road;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAWheelContactReferenceLoadTest,
    "TorqueAtlas.Suspension.Contact.ReferenceRideHeightLoad",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAWheelContactReferenceLoadTest::RunTest(const FString& Parameters)
{
    const FTADoubleWishboneSolverConfig Geometry =
        MakeContactFrontRightGeometry();

    const FTASuspensionRuntimeConfig Suspension =
        MakeContactSuspensionForceConfig();

    const FTARoadPlane Road =
        MakeFlatDryRoad();

    FTAChassisState Chassis;

    // Reference wheel center z = -0.45 m and tire radius = 0.327 m.
    // COM/chassis origin at 0.777 m places the reference tire support point on z=0.
    Chassis.PositionWorldM = FVector3d(0.0, 0.0, 0.777);

    FTADoubleWishboneState GeometryState;
    FTASuspensionRuntimeState SuspensionState;
    FTAResolvedWheelContact Contact;

    TestTrue(
        TEXT("Reference road contact resolves"),
        TAWheelContactResolver::ResolveDoubleWishboneRoadContact(
            Chassis,
            Geometry,
            Suspension,
            0.327,
            0.0,
            FTADoubleWishboneDamageOffsets{},
            Road,
            1.0 / 240.0,
            GeometryState,
            SuspensionState,
            Contact));

    TestTrue(
        TEXT("Reference wheel is in contact"),
        Contact.bInContact);

    TestTrue(
        TEXT("Reference travel remains close to authored zero"),
        FMath::Abs(Contact.TravelM) < 0.002);

    TestTrue(
        TEXT("Suspension creates meaningful positive vertical load"),
        Contact.VerticalLoadN > 2500.0);

    TestTrue(
        TEXT("Contact point lies near road plane"),
        FMath::Abs(Contact.ContactPointWorldM.Z) < 0.002);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAWheelContactVelocityTest,
    "TorqueAtlas.Suspension.Contact.DerivesVelocityFromChassis",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAWheelContactVelocityTest::RunTest(const FString& Parameters)
{
    const FTADoubleWishboneSolverConfig Geometry =
        MakeContactFrontRightGeometry();

    const FTASuspensionRuntimeConfig Suspension =
        MakeContactSuspensionForceConfig();

    const FTARoadPlane Road =
        MakeFlatDryRoad();

    FTAChassisState Chassis;
    Chassis.PositionWorldM = FVector3d(0.0, 0.0, 0.777);
    Chassis.LinearVelocityWorldMps = FVector3d(12.0, 0.0, 0.0);

    FTADoubleWishboneState GeometryState;
    FTASuspensionRuntimeState SuspensionState;
    FTAResolvedWheelContact Contact;

    TestTrue(
        TEXT("Moving road contact resolves"),
        TAWheelContactResolver::ResolveDoubleWishboneRoadContact(
            Chassis,
            Geometry,
            Suspension,
            0.327,
            0.0,
            FTADoubleWishboneDamageOffsets{},
            Road,
            1.0 / 240.0,
            GeometryState,
            SuspensionState,
            Contact));

    TestTrue(
        TEXT("Longitudinal contact speed comes from chassis velocity"),
        FMath::IsNearlyEqual(
            Contact.LongitudinalVelocityMps,
            12.0,
            0.02));

    TestTrue(
        TEXT("Straight chassis motion has negligible lateral speed"),
        FMath::Abs(Contact.LateralVelocityMps) < 0.02);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAWheelContactDroopLossTest,
    "TorqueAtlas.Suspension.Contact.LosesContactAtFullDroop",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAWheelContactDroopLossTest::RunTest(const FString& Parameters)
{
    const FTADoubleWishboneSolverConfig Geometry =
        MakeContactFrontRightGeometry();

    const FTASuspensionRuntimeConfig Suspension =
        MakeContactSuspensionForceConfig();

    const FTARoadPlane Road =
        MakeFlatDryRoad();

    FTAChassisState Chassis;
    Chassis.PositionWorldM = FVector3d(0.0, 0.0, 1.05);

    FTADoubleWishboneState GeometryState;
    FTASuspensionRuntimeState SuspensionState;
    FTAResolvedWheelContact Contact;

    TestTrue(
        TEXT("Raised chassis still resolves suspension geometry"),
        TAWheelContactResolver::ResolveDoubleWishboneRoadContact(
            Chassis,
            Geometry,
            Suspension,
            0.327,
            0.0,
            FTADoubleWishboneDamageOffsets{},
            Road,
            1.0 / 240.0,
            GeometryState,
            SuspensionState,
            Contact));

    TestFalse(
        TEXT("Wheel is off the road beyond droop reach"),
        Contact.bInContact);

    TestTrue(
        TEXT("Airborne wheel has zero road normal load"),
        FMath::IsNearlyZero(Contact.VerticalLoadN));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAWheelContactDamageAlignmentTest,
    "TorqueAtlas.Suspension.Contact.DamagedPickupChangesContactAlignment",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAWheelContactDamageAlignmentTest::RunTest(const FString& Parameters)
{
    const FTADoubleWishboneSolverConfig Geometry =
        MakeContactFrontRightGeometry();

    const FTASuspensionRuntimeConfig Suspension =
        MakeContactSuspensionForceConfig();

    const FTARoadPlane Road =
        MakeFlatDryRoad();

    FTAChassisState Chassis;
    Chassis.PositionWorldM = FVector3d(0.0, 0.0, 0.777);

    FTADoubleWishboneState ReferenceGeometryState;
    FTASuspensionRuntimeState ReferenceSuspensionState;
    FTAResolvedWheelContact ReferenceContact;

    TestTrue(
        TEXT("Reference contact resolves"),
        TAWheelContactResolver::ResolveDoubleWishboneRoadContact(
            Chassis,
            Geometry,
            Suspension,
            0.327,
            0.0,
            FTADoubleWishboneDamageOffsets{},
            Road,
            1.0 / 240.0,
            ReferenceGeometryState,
            ReferenceSuspensionState,
            ReferenceContact));

    FTADoubleWishboneDamageOffsets Damage;
    Damage.UpperInnerA = FVector3d(0.0, -0.02, 0.0);
    Damage.UpperInnerB = FVector3d(0.0, -0.02, 0.0);

    FTADoubleWishboneState DamagedGeometryState;
    FTASuspensionRuntimeState DamagedSuspensionState;
    FTAResolvedWheelContact DamagedContact;

    TestTrue(
        TEXT("Damaged contact resolves"),
        TAWheelContactResolver::ResolveDoubleWishboneRoadContact(
            Chassis,
            Geometry,
            Suspension,
            0.327,
            0.0,
            Damage,
            Road,
            1.0 / 240.0,
            DamagedGeometryState,
            DamagedSuspensionState,
            DamagedContact));

    TestTrue(
        TEXT("Damaged pickup changes solved camber"),
        FMath::Abs(
            DamagedContact.Geometry.CamberRad
            - ReferenceContact.Geometry.CamberRad)
            > FMath::DegreesToRadians(1.0));

    TestFalse(
        TEXT("Damage invalidates undamaged kinematic cache path"),
        DamagedSuspensionState.bKinematicCacheValid);

    return true;
}

#endif
