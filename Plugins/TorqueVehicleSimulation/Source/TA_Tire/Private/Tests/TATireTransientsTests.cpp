#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TATireTransients.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTATireTransientStepResponseTest,
    "TorqueAtlas.Tire.Transients.StepResponseIsMonotonic",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTATireTransientStepResponseTest::RunTest(
    const FString& Parameters)
{
    FTATireTransientConfig Config;

    FTATireTransientState State;
    TATireTransients::InitializeState(State);

    FTATireTransientInput Input;
    Input.TargetSlipRatio = 0.20;
    Input.TargetSlipAngleRad = 0.10;
    Input.LongitudinalSpeedMps = 20.0;
    Input.DeltaTimeSeconds = 1.0 / 240.0;

    FTATireTransientOutput Output;

    double PreviousSlipRatio = 0.0;

    for (int32 Index = 0; Index < 20; ++Index)
    {
        TestTrue(
            TEXT("Transient step succeeds"),
            TATireTransients::Step(
                Config,
                Input,
                State,
                Output));

        TestTrue(
            TEXT("Relaxed slip rises monotonically"),
            Output.RelaxedSlipRatio
                >= PreviousSlipRatio - 1.0e-12);

        TestTrue(
            TEXT("Relaxed slip does not overshoot target"),
            Output.RelaxedSlipRatio
                <= Input.TargetSlipRatio + 1.0e-12);

        PreviousSlipRatio =
            Output.RelaxedSlipRatio;
    }

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTATireTransientRelaxationLengthTest,
    "TorqueAtlas.Tire.Transients.LongerRelaxationRespondsSlower",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTATireTransientRelaxationLengthTest::RunTest(
    const FString& Parameters)
{
    FTATireTransientConfig ShortConfig;
    ShortConfig.LongitudinalRelaxationLengthM = 0.20;

    FTATireTransientConfig LongConfig =
        ShortConfig;

    LongConfig.LongitudinalRelaxationLengthM = 0.80;

    FTATireTransientState ShortState;
    FTATireTransientState LongState;
    TATireTransients::InitializeState(ShortState);
    TATireTransients::InitializeState(LongState);

    FTATireTransientInput Input;
    Input.TargetSlipRatio = 0.20;
    Input.LongitudinalSpeedMps = 15.0;
    Input.DeltaTimeSeconds = 0.01;

    FTATireTransientOutput ShortOutput;
    FTATireTransientOutput LongOutput;

    TestTrue(
        TEXT("Short relaxation solve succeeds"),
        TATireTransients::Step(
            ShortConfig,
            Input,
            ShortState,
            ShortOutput));

    TestTrue(
        TEXT("Long relaxation solve succeeds"),
        TATireTransients::Step(
            LongConfig,
            Input,
            LongState,
            LongOutput));

    TestTrue(
        TEXT("Longer relaxation length responds slower"),
        LongOutput.RelaxedSlipRatio
            < ShortOutput.RelaxedSlipRatio);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTATireTransientSpeedTest,
    "TorqueAtlas.Tire.Transients.HigherSpeedRespondsFasterInTime",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTATireTransientSpeedTest::RunTest(
    const FString& Parameters)
{
    FTATireTransientConfig Config;

    FTATireTransientState SlowState;
    FTATireTransientState FastState;
    TATireTransients::InitializeState(SlowState);
    TATireTransients::InitializeState(FastState);

    FTATireTransientInput SlowInput;
    SlowInput.TargetSlipRatio = 0.20;
    SlowInput.LongitudinalSpeedMps = 5.0;
    SlowInput.DeltaTimeSeconds = 0.01;

    FTATireTransientInput FastInput =
        SlowInput;

    FastInput.LongitudinalSpeedMps = 25.0;

    FTATireTransientOutput SlowOutput;
    FTATireTransientOutput FastOutput;

    TestTrue(
        TEXT("Slow-speed transient step succeeds"),
        TATireTransients::Step(
            Config,
            SlowInput,
            SlowState,
            SlowOutput));

    TestTrue(
        TEXT("Fast-speed transient step succeeds"),
        TATireTransients::Step(
            Config,
            FastInput,
            FastState,
            FastOutput));

    TestTrue(
        TEXT("Higher speed produces faster temporal response"),
        FastOutput.RelaxedSlipRatio
            > SlowOutput.RelaxedSlipRatio);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTATireTransientAirborneDecayTest,
    "TorqueAtlas.Tire.Transients.AirborneContactMemoryDecays",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTATireTransientAirborneDecayTest::RunTest(
    const FString& Parameters)
{
    FTATireTransientConfig Config;

    FTATireTransientState State;
    TATireTransients::InitializeState(State);
    State.RelaxedSlipRatio = 0.30;
    State.RelaxedSlipAngleRad = 0.20;
    State.bWasInContact = true;
    State.ContactAgeSeconds = 1.0;

    FTATireTransientInput Input;
    Input.bContactValid = false;
    Input.DeltaTimeSeconds = 0.02;

    FTATireTransientOutput Output;

    TestTrue(
        TEXT("Airborne decay step succeeds"),
        TATireTransients::Step(
            Config,
            Input,
            State,
            Output));

    TestTrue(
        TEXT("Airborne longitudinal memory decays"),
        FMath::Abs(Output.RelaxedSlipRatio)
            < 0.30);

    TestTrue(
        TEXT("Airborne lateral memory decays"),
        FMath::Abs(Output.RelaxedSlipAngleRad)
            < 0.20);

    TestEqual(
        TEXT("Airborne contact age resets"),
        State.ContactAgeSeconds,
        0.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTATireTransientLowSpeedFiniteTest,
    "TorqueAtlas.Tire.Transients.LowSpeedRemainsFinite",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTATireTransientLowSpeedFiniteTest::RunTest(
    const FString& Parameters)
{
    FTATireTransientConfig Config;

    FTATireTransientState State;
    TATireTransients::InitializeState(State);

    FTATireTransientInput Input;
    Input.TargetSlipRatio = 1.0;
    Input.TargetSlipAngleRad = 0.5;
    Input.LongitudinalSpeedMps = 0.0;

    FTATireTransientOutput Output;

    TestTrue(
        TEXT("Zero-speed transient solve succeeds"),
        TATireTransients::Step(
            Config,
            Input,
            State,
            Output));

    TestTrue(
        TEXT("Zero-speed longitudinal time constant is finite"),
        FMath::IsFinite(
            Output.LongitudinalTimeConstantSeconds));

    TestTrue(
        TEXT("Zero-speed lateral time constant is finite"),
        FMath::IsFinite(
            Output.LateralTimeConstantSeconds));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTATireTransientValidationTest,
    "TorqueAtlas.Tire.Transients.ConfigValidation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTATireTransientValidationTest::RunTest(
    const FString& Parameters)
{
    FTATireTransientConfig Config;

    TestTrue(
        TEXT("Default transient config validates"),
        TATireTransients::ValidateConfig(Config));

    Config.LateralRelaxationLengthM = 0.0;

    TestFalse(
        TEXT("Zero lateral relaxation length is rejected"),
        TATireTransients::ValidateConfig(Config));

    return true;
}

#endif
