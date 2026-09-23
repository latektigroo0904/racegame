#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TAMultiLinkContactResolver.h"

namespace
{
    FTAMultiLinkSolverConfig MakeRearGeometry()
    {
        FTAMultiLinkSolverConfig C;
        C.Links[0].ChassisPickupReference={-1.15,0.35,-0.18}; C.Links[0].UprightPickupReference={-1.20,0.68,-0.24};
        C.Links[1].ChassisPickupReference={-1.50,0.36,-0.20}; C.Links[1].UprightPickupReference={-1.42,0.69,-0.25};
        C.Links[2].ChassisPickupReference={-1.10,0.34,-0.46}; C.Links[2].UprightPickupReference={-1.18,0.72,-0.49};
        C.Links[3].ChassisPickupReference={-1.53,0.33,-0.47}; C.Links[3].UprightPickupReference={-1.45,0.73,-0.50};
        C.Links[4].ChassisPickupReference={-1.50,0.38,-0.36}; C.Links[4].UprightPickupReference={-1.48,0.71,-0.38};
        C.WheelCenterReference={-1.31,0.765,-0.45};
        C.DamperChassisReference={-1.31,0.40,-0.05}; C.DamperUprightReference={-1.31,0.64,-0.40};
        C.MaxIterations=180; C.PositionToleranceM=0.001;
        return C;
    }

    FTASuspensionRuntimeConfig MakeSuspension()
    {
        FTASuspensionRuntimeConfig C;
        C.SpringRateNPerM=95000.0; C.StaticSpringCompressionM=0.08;
        C.BumpDampingNsPerM=4500.0; C.ReboundDampingNsPerM=6500.0;
        return C;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTAMultiLinkContactReferenceTest,
    "TorqueAtlas.Suspension.MultiLinkContact.ReferenceLoad",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAMultiLinkContactReferenceTest::RunTest(const FString& Parameters)
{
    FTAChassisState Chassis; Chassis.PositionWorldM={0,0,0.777};
    FTARoadPlane Road; Road.NormalWorld={0,0,1}; Road.Surface.Material=ETASurfaceMaterial::FreshAsphalt;
    FTAMultiLinkRuntimeState GeometryState; FTASuspensionRuntimeState SuspensionState; FTAResolvedMultiLinkContact Contact;
    TestTrue(TEXT("Rear road contact resolves"), TAMultiLinkContactResolver::ResolveRoadContact(
        Chassis, MakeRearGeometry(), MakeSuspension(), 0.327, FTAMultiLinkDamageOffsets{}, Road,
        1.0/240.0, GeometryState, SuspensionState, Contact));
    TestTrue(TEXT("Rear wheel contacts road"), Contact.bInContact);
    TestTrue(TEXT("Rear reference travel remains near zero"), FMath::Abs(Contact.TravelM)<0.003);
    TestTrue(TEXT("Rear suspension derives positive load"), Contact.VerticalLoadN>2000.0);
    TestTrue(TEXT("Rear patch lies on road"), FMath::Abs(Contact.ContactPointWorldM.Z)<0.003);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTAMultiLinkContactVelocityTest,
    "TorqueAtlas.Suspension.MultiLinkContact.DerivesPatchVelocity",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAMultiLinkContactVelocityTest::RunTest(const FString& Parameters)
{
    FTAChassisState Chassis; Chassis.PositionWorldM={0,0,0.777}; Chassis.LinearVelocityWorldMps={15,0,0};
    FTARoadPlane Road; Road.NormalWorld={0,0,1};
    FTAMultiLinkRuntimeState GeometryState; FTASuspensionRuntimeState SuspensionState; FTAResolvedMultiLinkContact Contact;
    TestTrue(TEXT("Moving rear contact resolves"), TAMultiLinkContactResolver::ResolveRoadContact(
        Chassis, MakeRearGeometry(), MakeSuspension(), 0.327, FTAMultiLinkDamageOffsets{}, Road,
        1.0/240.0, GeometryState, SuspensionState, Contact));
    TestTrue(TEXT("Rear longitudinal velocity derives from chassis"), FMath::IsNearlyEqual(Contact.LongitudinalVelocityMps,15.0,0.03));
    TestTrue(TEXT("Rear lateral velocity near zero"), FMath::Abs(Contact.LateralVelocityMps)<0.03);
    return true;
}

#endif
