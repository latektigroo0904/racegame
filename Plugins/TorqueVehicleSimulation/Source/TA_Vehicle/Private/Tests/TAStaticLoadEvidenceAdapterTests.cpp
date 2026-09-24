#include "Misc/AutomationTest.h"
#include "TAStaticLoadEvidenceAdapter.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAStaticLoadEvidenceAdapterTest,
    "TorqueAtlas.Vehicle.StaticLoad.EvidenceAdapter",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAStaticLoadEvidenceAdapterTest::RunTest(const FString& Parameters)
{
    FTAStaticLoadEvidenceAdapterConfig Config;
    FTAFourWheelRuntimeState State;
    FTAFourWheelStepOutput Output;
    Output.bContactsSolved = true;
    Output.FrontAxle.LeftVehicleContact.VerticalLoadN = 4100.0;
    Output.FrontAxle.RightVehicleContact.VerticalLoadN = 4200.0;
    Output.RearAxle.LeftVehicleContact.VerticalLoadN = 3300.0;
    Output.RearAxle.RightVehicleContact.VerticalLoadN = 3400.0;
    State.Vehicle.Chassis.LinearVelocityWorldMps = FVector3d(0.003, 0.004, 0.0);
    State.Vehicle.Chassis.AngularVelocityWorldRadPerSec = FVector3d(0.0, 0.0, 0.002);

    FTAStaticLoadSample Sample;
    TestTrue(TEXT("Canonical evidence converts"), TAStaticLoadEvidenceAdapter::BuildSample(
        Config, State, Output, Config.ExpectedDeltaTimeSeconds, Sample));
    TestEqual(TEXT("FL preserves support magnitude"), Sample.FrontLeftLoadN, 4100.0);
    TestEqual(TEXT("FR preserves support magnitude"), Sample.FrontRightLoadN, 4200.0);
    TestEqual(TEXT("RL preserves support magnitude"), Sample.RearLeftLoadN, 3300.0);
    TestEqual(TEXT("RR preserves support magnitude"), Sample.RearRightLoadN, 3400.0);
    TestTrue(TEXT("Linear speed is vector magnitude"), FMath::IsNearlyEqual(Sample.ChassisLinearSpeedMps, 0.005, 1.0e-12));
    TestEqual(TEXT("Angular speed is vector magnitude"), Sample.ChassisAngularSpeedRadPerSec, 0.002);

    FTAStaticLoadSample Rejected = Sample;
    TestFalse(TEXT("Cadence drift rejects evidence"), TAStaticLoadEvidenceAdapter::BuildSample(
        Config, State, Output, Config.ExpectedDeltaTimeSeconds + 0.001, Rejected));
    TestEqual(TEXT("Rejected evidence clears FL"), Rejected.FrontLeftLoadN, 0.0);

    Output.FrontAxle.LeftVehicleContact.VerticalLoadN = -1.0;
    TestFalse(TEXT("Negative support load rejects evidence"), TAStaticLoadEvidenceAdapter::BuildSample(
        Config, State, Output, Config.ExpectedDeltaTimeSeconds, Rejected));

    Output.FrontAxle.LeftVehicleContact.VerticalLoadN = 4100.0;
    Output.bContactsSolved = false;
    TestFalse(TEXT("Unsolved contacts reject evidence"), TAStaticLoadEvidenceAdapter::BuildSample(
        Config, State, Output, Config.ExpectedDeltaTimeSeconds, Rejected));

    return true;
}

#endif
