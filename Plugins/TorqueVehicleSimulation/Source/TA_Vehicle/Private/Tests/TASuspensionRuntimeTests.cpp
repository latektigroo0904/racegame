#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TASuspensionRuntime.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTASuspensionCacheInterpolationTest,
    "TorqueAtlas.Suspension.Runtime.CacheInterpolation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTASuspensionCacheInterpolationTest::RunTest(const FString& Parameters)
{
    FTASuspensionRuntimeConfig Config;

    FTASuspensionKinematicSample Droop;
    Droop.TravelM = -0.05;
    Droop.CamberRad = FMath::DegreesToRadians(-0.5);
    Droop.ToeRad = FMath::DegreesToRadians(0.10);
    Droop.MotionRatio = 0.95;

    FTASuspensionKinematicSample Bump;
    Bump.TravelM = 0.05;
    Bump.CamberRad = FMath::DegreesToRadians(-1.5);
    Bump.ToeRad = FMath::DegreesToRadians(0.20);
    Bump.MotionRatio = 1.05;

    Config.KinematicSamples.Add(Droop);
    Config.KinematicSamples.Add(Bump);

    FTASuspensionRuntimeState State;

    TestTrue(
        TEXT("Cache evaluates at midpoint"),
        TASuspensionRuntime::EvaluateKinematicCache(
            Config,
            0.0,
            State));

    TestTrue(
        TEXT("Midpoint camber interpolates between endpoints"),
        FMath::IsNearlyEqual(
            State.CamberRad,
            FMath::DegreesToRadians(-1.0),
            1.0e-6));

    TestTrue(
        TEXT("Motion ratio interpolates"),
        FMath::IsNearlyEqual(State.MotionRatio, 1.0, 1.0e-6));

    State.bKinematicCacheValid = false;

    TestFalse(
        TEXT("Damaged/invalidated cache refuses undamaged lookup"),
        TASuspensionRuntime::EvaluateKinematicCache(
            Config,
            0.0,
            State));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTASuspensionForceTest,
    "TorqueAtlas.Suspension.Runtime.SpringDamperForce",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTASuspensionForceTest::RunTest(const FString& Parameters)
{
    FTASuspensionRuntimeConfig Config;
    Config.SpringRateNPerM = 40000.0;
    Config.StaticSpringCompressionM = 0.08;
    Config.BumpDampingNsPerM = 3000.0;
    Config.ReboundDampingNsPerM = 5000.0;

    FTASuspensionRuntimeState State;
    State.TravelM = 0.0;
    State.TravelVelocityMps = 0.0;
    State.MotionRatio = 1.0;

    const FTASuspensionForceOutput StaticForce =
        TASuspensionRuntime::CalculateForce(Config, State);

    TestTrue(
        TEXT("Reference ride height retains positive spring support force"),
        StaticForce.SpringForceN > 0.0);

    State.TravelM = 0.02;
    State.TravelVelocityMps = 0.50;

    const FTASuspensionForceOutput BumpForce =
        TASuspensionRuntime::CalculateForce(Config, State);

    TestTrue(
        TEXT("Bump increases spring force"),
        BumpForce.SpringForceN > StaticForce.SpringForceN);

    TestTrue(
        TEXT("Bump damping contributes positive supporting force under chosen convention"),
        BumpForce.DamperForceN > 0.0);

    State.TravelVelocityMps = -0.50;

    const FTASuspensionForceOutput ReboundForce =
        TASuspensionRuntime::CalculateForce(Config, State);

    TestTrue(
        TEXT("Rebound damping opposes extension"),
        ReboundForce.DamperForceN < 0.0);

    return true;
}

#endif
