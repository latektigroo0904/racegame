#include "TAActiveAeroActuator.h"

bool TAActiveAeroActuator::ValidateConfig(
    const FTAActiveAeroActuatorConfig& Config)
{
    return
        FMath::IsFinite(Config.MinimumPosition01)
        && FMath::IsFinite(Config.MaximumPosition01)
        && Config.MinimumPosition01 >= 0.0
        && Config.MaximumPosition01 <= 1.0
        && Config.MaximumPosition01 >= Config.MinimumPosition01
        && FMath::IsFinite(Config.ExtendRate01PerSec)
        && Config.ExtendRate01PerSec >= 0.0
        && FMath::IsFinite(Config.RetractRate01PerSec)
        && Config.RetractRate01PerSec >= 0.0
        && FMath::IsFinite(Config.FailSafePosition01)
        && Config.FailSafePosition01 >= Config.MinimumPosition01
        && Config.FailSafePosition01 <= Config.MaximumPosition01;
}

void TAActiveAeroActuator::InitializeState(
    const FTAActiveAeroActuatorConfig& Config,
    FTAActiveAeroActuatorState& OutState)
{
    OutState = FTAActiveAeroActuatorState{};
    OutState.Position01 =
        FMath::Clamp(
            Config.FailSafePosition01,
            Config.MinimumPosition01,
            Config.MaximumPosition01);
}

bool TAActiveAeroActuator::Step(
    const FTAActiveAeroActuatorConfig& Config,
    const FTAActiveAeroActuatorInput& Input,
    FTAActiveAeroActuatorState& InOutState,
    FTAActiveAeroActuatorOutput& OutOutput)
{
    OutOutput = FTAActiveAeroActuatorOutput{};

    if (!ValidateConfig(Config)
        || !FMath::IsFinite(Input.TargetPosition01)
        || !FMath::IsFinite(Input.Health01)
        || !FMath::IsFinite(Input.DeltaTimeSeconds)
        || Input.DeltaTimeSeconds <= 0.0
        || !FMath::IsFinite(InOutState.Position01))
    {
        return false;
    }

    InOutState.Position01 =
        FMath::Clamp(
            InOutState.Position01,
            Config.MinimumPosition01,
            Config.MaximumPosition01);

    OutOutput.RequestedTargetPosition01 =
        FMath::Clamp(
            Input.TargetPosition01,
            Config.MinimumPosition01,
            Config.MaximumPosition01);

    OutOutput.EffectiveTargetPosition01 =
        OutOutput.RequestedTargetPosition01;

    if (!Input.bPowered
        && Config.bReturnToFailSafeWithoutPower)
    {
        OutOutput.EffectiveTargetPosition01 =
            Config.FailSafePosition01;

        OutOutput.bFailSafeCommanded = true;
    }

    if (Input.bMechanicallyStuck)
    {
        OutOutput.Position01 =
            InOutState.Position01;

        OutOutput.bStuck = true;
        return true;
    }

    const double Health01 =
        FMath::Clamp(Input.Health01, 0.0, 1.0);

    const double Difference =
        OutOutput.EffectiveTargetPosition01
        - InOutState.Position01;

    const double Rate01PerSec =
        Difference >= 0.0
        ? Config.ExtendRate01PerSec
        : Config.RetractRate01PerSec;

    const double MaxDelta01 =
        Rate01PerSec
        * Health01
        * Input.DeltaTimeSeconds;

    const double AppliedDelta01 =
        FMath::Clamp(
            Difference,
            -MaxDelta01,
            MaxDelta01);

    OutOutput.bRateLimited =
        FMath::Abs(AppliedDelta01 - Difference)
        > 1.0e-12;

    InOutState.Position01 =
        FMath::Clamp(
            InOutState.Position01 + AppliedDelta01,
            Config.MinimumPosition01,
            Config.MaximumPosition01);

    OutOutput.Position01 =
        InOutState.Position01;

    return true;
}
