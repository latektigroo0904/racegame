#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TAAerodynamics.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAAeroZeroSpeedTest,
    "TorqueAtlas.Vehicle.Aero.ZeroRelativeSpeed",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAAeroZeroSpeedTest::RunTest(const FString& Parameters)
{
    FTAAerodynamicsConfig Config;
    FTAAerodynamicsEnvironment Environment;
    FTAChassisState Chassis;
    Chassis.LinearVelocityWorldMps = FVector3d(12.0, 0.0, 0.0);
    Environment.WindVelocityWorldMps = Chassis.LinearVelocityWorldMps;

    FTAAerodynamicsOutput Output;
    TestTrue(TEXT("Calculate"), TAAerodynamics::Calculate(Config, Environment, Chassis, Output));
    TestTrue(TEXT("Zero force"), Output.ForceWorldN.IsNearlyZero());
    TestEqual(TEXT("Zero q"), Output.DynamicPressurePa, 0.0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAAeroSpeedSquaredTest,
    "TorqueAtlas.Vehicle.Aero.SpeedSquared",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAAeroSpeedSquaredTest::RunTest(const FString& Parameters)
{
    FTAAerodynamicsConfig Config;
    Config.LiftCoefficient = 0.0;
    FTAAerodynamicsEnvironment Environment;
    FTAChassisState Slow;
    FTAChassisState Fast;
    Slow.LinearVelocityWorldMps = FVector3d(10.0, 0.0, 0.0);
    Fast.LinearVelocityWorldMps = FVector3d(20.0, 0.0, 0.0);

    FTAAerodynamicsOutput A;
    FTAAerodynamicsOutput B;
    TestTrue(TEXT("Slow"), TAAerodynamics::Calculate(Config, Environment, Slow, A));
    TestTrue(TEXT("Fast"), TAAerodynamics::Calculate(Config, Environment, Fast, B));
    TestTrue(TEXT("4x drag"), FMath::IsNearlyEqual(B.DragForceN, 4.0 * A.DragForceN, 1.0e-9));
    TestTrue(TEXT("Opposes travel"), A.ForceWorldN.X < 0.0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAAeroWindAndDownforceTest,
    "TorqueAtlas.Vehicle.Aero.WindAndDownforce",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAAeroWindAndDownforceTest::RunTest(const FString& Parameters)
{
    FTAAerodynamicsConfig Config;
    Config.LiftCoefficient = -0.5;
    FTAAerodynamicsEnvironment Headwind;
    FTAAerodynamicsEnvironment Tailwind;
    Headwind.WindVelocityWorldMps = FVector3d(-10.0, 0.0, 0.0);
    Tailwind.WindVelocityWorldMps = FVector3d(10.0, 0.0, 0.0);

    FTAChassisState Chassis;
    Chassis.LinearVelocityWorldMps = FVector3d(20.0, 0.0, 0.0);

    FTAAerodynamicsOutput Head;
    FTAAerodynamicsOutput Tail;
    TestTrue(TEXT("Head"), TAAerodynamics::Calculate(Config, Headwind, Chassis, Head));
    TestTrue(TEXT("Tail"), TAAerodynamics::Calculate(Config, Tailwind, Chassis, Tail));
    TestTrue(TEXT("Headwind increases q"), Head.DynamicPressurePa > Tail.DynamicPressurePa);
    TestTrue(TEXT("Negative Cl is downforce"), Head.ForceWorldN.Z < 0.0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAAeroApplicationMomentTest,
    "TorqueAtlas.Vehicle.Aero.ApplicationPointMoment",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAAeroApplicationMomentTest::RunTest(const FString& Parameters)
{
    FTAAerodynamicsConfig Config;
    Config.LiftCoefficient = 0.0;
    Config.ApplicationPointBodyM = FVector3d(0.0, 0.0, 0.5);
    FTAAerodynamicsEnvironment Environment;
    FTAChassisState Chassis;
    Chassis.LinearVelocityWorldMps = FVector3d(30.0, 0.0, 0.0);

    FTAAerodynamicsOutput Output;
    TestTrue(TEXT("Calculate"), TAAerodynamics::Calculate(Config, Environment, Chassis, Output));
    TestTrue(TEXT("Offset drag creates pitch moment"), FMath::Abs(Output.TorqueWorldNm.Y) > 1.0);

    FTAChassisForceAccumulator Accumulator;
    TestTrue(TEXT("Add"), TAAerodynamics::AddToChassis(Config, Environment, Chassis, Accumulator));
    TestTrue(TEXT("Accumulator force matches"), Accumulator.TotalForceWorldN.Equals(Output.ForceWorldN, 1.0e-9));
    TestTrue(TEXT("Accumulator torque matches"), Accumulator.TotalTorqueWorldNm.Equals(Output.TorqueWorldNm, 1.0e-9));
    return true;
}

#endif
