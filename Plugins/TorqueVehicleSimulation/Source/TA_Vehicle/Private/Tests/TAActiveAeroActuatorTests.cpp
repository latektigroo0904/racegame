#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TAActiveAeroActuator.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAActiveAeroRateLimitTest,
    "TorqueAtlas.Vehicle.Aero.ActiveActuator.RateLimit",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAActiveAeroRateLimitTest::RunTest(const FString& Parameters)
{
    FTAActiveAeroActuatorConfig Config;
    Config.ExtendRate01PerSec = 1.0;

    FTAActiveAeroActuatorState State;
    TAActiveAeroActuator::InitializeState(Config, State);

    FTAActiveAeroActuatorInput Input;
    Input.TargetPosition01 = 1.0;
    Input.DeltaTimeSeconds = 0.1;

    FTAActiveAeroActuatorOutput Output;

    TestTrue(TEXT("Actuator step succeeds"),
        TAActiveAeroActuator::Step(Config, Input, State, Output));

    TestTrue(TEXT("Position is rate limited"),
        FMath::IsNearlyEqual(Output.Position01, 0.1, 1.0e-12));

    TestTrue(TEXT("Rate limit is reported"), Output.bRateLimited);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAActiveAeroFailSafeTest,
    "TorqueAtlas.Vehicle.Aero.ActiveActuator.PowerLossUsesFailSafe",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAActiveAeroFailSafeTest::RunTest(const FString& Parameters)
{
    FTAActiveAeroActuatorConfig Config;
    Config.FailSafePosition01 = 0.25;
    Config.RetractRate01PerSec = 10.0;

    FTAActiveAeroActuatorState State;
    State.Position01 = 0.8;

    FTAActiveAeroActuatorInput Input;
    Input.TargetPosition01 = 1.0;
    Input.bPowered = false;
    Input.DeltaTimeSeconds = 1.0;

    FTAActiveAeroActuatorOutput Output;

    TestTrue(TEXT("Power-loss step succeeds"),
        TAActiveAeroActuator::Step(Config, Input, State, Output));

    TestTrue(TEXT("Fail-safe is commanded"), Output.bFailSafeCommanded);
    TestTrue(TEXT("Actuator reaches fail-safe"),
        FMath::IsNearlyEqual(Output.Position01, 0.25, 1.0e-12));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAActiveAeroStuckTest,
    "TorqueAtlas.Vehicle.Aero.ActiveActuator.StuckPreservesPosition",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAActiveAeroStuckTest::RunTest(const FString& Parameters)
{
    FTAActiveAeroActuatorConfig Config;
    FTAActiveAeroActuatorState State;
    State.Position01 = 0.6;

    FTAActiveAeroActuatorInput Input;
    Input.TargetPosition01 = 0.0;
    Input.bMechanicallyStuck = true;

    FTAActiveAeroActuatorOutput Output;

    TestTrue(TEXT("Stuck actuator step succeeds"),
        TAActiveAeroActuator::Step(Config, Input, State, Output));

    TestEqual(TEXT("Stuck position is preserved"), Output.Position01, 0.6);
    return true;
}

#endif
