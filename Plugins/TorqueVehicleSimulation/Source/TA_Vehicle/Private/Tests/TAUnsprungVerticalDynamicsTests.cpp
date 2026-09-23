#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TAUnsprungVerticalDynamics.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAUnsprungBalancedForceTest,
    "TorqueAtlas.Suspension.Unsprung.BalancedForce",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAUnsprungBalancedForceTest::RunTest(const FString& Parameters)
{
    FTAUnsprungVerticalConfig Config;
    FTAUnsprungVerticalState State;
    FTAUnsprungVerticalInput Input;

    Input.TireNormalForceN = 3500.0;
    Input.SuspensionReactionForceN = 3500.0;
    Input.GravityAlongTravelAxisMps2 = 0.0;

    FTAUnsprungVerticalOutput Output;

    TestTrue(
        TEXT("Balanced unsprung step integrates"),
        TAUnsprungVerticalDynamics::Integrate(
            Config,
            Input,
            1.0 / 240.0,
            State,
            Output));

    TestTrue(
        TEXT("Balanced forces produce near-zero generalized acceleration"),
        FMath::IsNearlyZero(
            Output.TravelAccelerationMps2,
            1.0e-9));

    TestTrue(
        TEXT("Balanced forces preserve travel"),
        FMath::IsNearlyZero(
            State.TravelM,
            1.0e-9));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAUnsprungForceDirectionTest,
    "TorqueAtlas.Suspension.Unsprung.ForceDirection",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAUnsprungForceDirectionTest::RunTest(const FString& Parameters)
{
    FTAUnsprungVerticalConfig Config;
    FTAUnsprungVerticalState State;
    FTAUnsprungVerticalInput Input;

    Input.TireNormalForceN = 5000.0;
    Input.SuspensionReactionForceN = 3000.0;
    Input.GravityAlongTravelAxisMps2 = 0.0;

    FTAUnsprungVerticalOutput Output;

    TestTrue(
        TEXT("Positive net tire force integrates"),
        TAUnsprungVerticalDynamics::Integrate(
            Config,
            Input,
            0.01,
            State,
            Output));

    TestTrue(
        TEXT("Excess tire force accelerates toward bump"),
        Output.TravelAccelerationMps2 > 0.0
        && State.TravelVelocityMps > 0.0
        && State.TravelM > 0.0);

    State = FTAUnsprungVerticalState{};
    Input.TireNormalForceN = 2000.0;
    Input.SuspensionReactionForceN = 4000.0;

    TestTrue(
        TEXT("Negative net force integrates"),
        TAUnsprungVerticalDynamics::Integrate(
            Config,
            Input,
            0.01,
            State,
            Output));

    TestTrue(
        TEXT("Excess suspension reaction accelerates toward droop"),
        Output.TravelAccelerationMps2 < 0.0
        && State.TravelVelocityMps < 0.0
        && State.TravelM < 0.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAUnsprungRelativeFrameTest,
    "TorqueAtlas.Suspension.Unsprung.ChassisAcceleration",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAUnsprungRelativeFrameTest::RunTest(const FString& Parameters)
{
    FTAUnsprungVerticalConfig Config;
    FTAUnsprungVerticalState State;
    FTAUnsprungVerticalInput Input;

    Input.TireNormalForceN = 3500.0;
    Input.SuspensionReactionForceN = 3500.0;
    Input.GravityAlongTravelAxisMps2 = 0.0;
    Input.ChassisAccelerationAlongTravelAxisMps2 = 3.0;

    FTAUnsprungVerticalOutput Output;

    TestTrue(
        TEXT("Accelerating reference-frame step integrates"),
        TAUnsprungVerticalDynamics::Integrate(
            Config,
            Input,
            0.01,
            State,
            Output));

    TestTrue(
        TEXT("Upward chassis acceleration creates relative droop tendency"),
        Output.TravelAccelerationMps2 < 0.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAUnsprungTravelLimitTest,
    "TorqueAtlas.Suspension.Unsprung.TravelLimit",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAUnsprungTravelLimitTest::RunTest(const FString& Parameters)
{
    FTAUnsprungVerticalConfig Config;
    Config.MinTravelM = -0.05;
    Config.MaxTravelM = 0.05;
    Config.TravelLimitRestitution01 = 0.0;

    FTAUnsprungVerticalState State;
    State.TravelM = 0.049;
    State.TravelVelocityMps = 5.0;

    FTAUnsprungVerticalInput Input;
    Input.GravityAlongTravelAxisMps2 = 0.0;

    FTAUnsprungVerticalOutput Output;

    TestTrue(
        TEXT("Limit impact integrates"),
        TAUnsprungVerticalDynamics::Integrate(
            Config,
            Input,
            0.01,
            State,
            Output));

    TestTrue(
        TEXT("Bump limit reports hit"),
        Output.bHitBumpLimit);

    TestTrue(
        TEXT("Travel clamps to bump limit"),
        FMath::IsNearlyEqual(
            State.TravelM,
            Config.MaxTravelM,
            1.0e-9));

    TestTrue(
        TEXT("Zero-restitution limit removes outward velocity"),
        FMath::IsNearlyZero(
            State.TravelVelocityMps,
            1.0e-9));

    TestTrue(
        TEXT("Limit impact speed is reported"),
        Output.LimitImpactSpeedMps > 0.0);

    return true;
}

#endif
