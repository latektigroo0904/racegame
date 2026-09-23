#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TAChassisDynamics.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAChassisCenteredForceTest,
    "TorqueAtlas.Chassis.Dynamics.CenteredForce",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAChassisCenteredForceTest::RunTest(const FString& Parameters)
{
    FTAChassisConfig Config;
    Config.MassKg = 1000.0;
    Config.GravityWorldMps2 = FVector3d::ZeroVector;

    FTAChassisState State;

    FTAChassisForceAccumulator Accumulator;
    TAChassisDynamics::AddForceAtWorldPoint(
        State,
        FVector3d(1000.0, 0.0, 0.0),
        State.PositionWorldM,
        Accumulator);

    TestTrue(
        TEXT("Force through COM creates no torque"),
        Accumulator.TotalTorqueWorldNm.IsNearlyZero());

    TestTrue(
        TEXT("Chassis integrates"),
        TAChassisDynamics::Integrate(
            Config,
            Accumulator,
            0.1,
            State));

    TestTrue(
        TEXT("Centered forward force increases forward velocity"),
        State.LinearVelocityWorldMps.X > 0.0);

    TestTrue(
        TEXT("Centered force leaves angular velocity near zero"),
        State.AngularVelocityWorldRadPerSec.IsNearlyZero());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAChassisOffCenterForceTest,
    "TorqueAtlas.Chassis.Dynamics.OffCenterForce",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAChassisOffCenterForceTest::RunTest(const FString& Parameters)
{
    FTAChassisConfig Config;
    Config.MassKg = 1000.0;
    Config.GravityWorldMps2 = FVector3d::ZeroVector;
    Config.PrincipalInertiaBodyKgm2 = FVector3d(500.0, 800.0, 900.0);

    FTAChassisState State;

    FTAChassisForceAccumulator Accumulator;

    TAChassisDynamics::AddForceAtWorldPoint(
        State,
        FVector3d(1000.0, 0.0, 0.0),
        FVector3d(0.0, 1.0, 0.0),
        Accumulator);

    TestTrue(
        TEXT("Off-center force creates non-zero torque"),
        FMath::Abs(Accumulator.TotalTorqueWorldNm.Z) > 0.0);

    TestTrue(
        TEXT("Chassis integrates"),
        TAChassisDynamics::Integrate(
            Config,
            Accumulator,
            0.1,
            State));

    TestTrue(
        TEXT("Off-center force creates angular velocity"),
        FMath::Abs(State.AngularVelocityWorldRadPerSec.Z) > 0.0);

    TestTrue(
        TEXT("Orientation remains normalized"),
        FMath::IsNearlyEqual(
            State.OrientationWorld.SizeSquared(),
            1.0,
            1.0e-6));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAChassisPureTorqueTest,
    "TorqueAtlas.Chassis.Dynamics.PureTorque",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAChassisPureTorqueTest::RunTest(const FString& Parameters)
{
    FTAChassisConfig Config;
    Config.GravityWorldMps2 = FVector3d::ZeroVector;

    FTAChassisState State;
    FTAChassisForceAccumulator Accumulator;

    TAChassisDynamics::AddTorqueWorld(
        FVector3d(0.0, 0.0, 1000.0),
        Accumulator);

    TestTrue(
        TEXT("Chassis integrates pure torque"),
        TAChassisDynamics::Integrate(
            Config,
            Accumulator,
            0.1,
            State));

    TestTrue(
        TEXT("Pure torque produces angular velocity"),
        State.AngularVelocityWorldRadPerSec.Z > 0.0);

    TestTrue(
        TEXT("Pure torque creates no linear velocity without gravity"),
        State.LinearVelocityWorldMps.IsNearlyZero());

    return true;
}

#endif
