#include "TATireTransients.h"

namespace
{
    double FirstOrderBlend(
        const double DeltaTimeSeconds,
        const double TimeConstantSeconds)
    {
        if (TimeConstantSeconds <= UE_DOUBLE_SMALL_NUMBER)
        {
            return 1.0;
        }

        return FMath::Clamp(
            1.0 - FMath::Exp(
                -DeltaTimeSeconds
                / TimeConstantSeconds),
            0.0,
            1.0);
    }

    double RelaxToward(
        const double Current,
        const double Target,
        const double Blend01)
    {
        return Current
            + FMath::Clamp(
                Blend01,
                0.0,
                1.0)
            * (Target - Current);
    }
}

bool TATireTransients::ValidateConfig(
    const FTATireTransientConfig& Config)
{
    return
        FMath::IsFinite(
            Config.LongitudinalRelaxationLengthM)
        && Config.LongitudinalRelaxationLengthM
            > UE_DOUBLE_SMALL_NUMBER
        && FMath::IsFinite(
            Config.LateralRelaxationLengthM)
        && Config.LateralRelaxationLengthM
            > UE_DOUBLE_SMALL_NUMBER
        && FMath::IsFinite(
            Config.RelaxationSpeedFloorMps)
        && Config.RelaxationSpeedFloorMps
            > UE_DOUBLE_SMALL_NUMBER
        && FMath::IsFinite(
            Config.AirborneDecayTimeSeconds)
        && Config.AirborneDecayTimeSeconds
            > UE_DOUBLE_SMALL_NUMBER;
}

void TATireTransients::InitializeState(
    FTATireTransientState& OutState)
{
    OutState =
        FTATireTransientState{};
}

bool TATireTransients::Step(
    const FTATireTransientConfig& Config,
    const FTATireTransientInput& Input,
    FTATireTransientState& InOutState,
    FTATireTransientOutput& OutOutput)
{
    OutOutput =
        FTATireTransientOutput{};

    if (!ValidateConfig(Config)
        || !FMath::IsFinite(Input.TargetSlipRatio)
        || !FMath::IsFinite(Input.TargetSlipAngleRad)
        || !FMath::IsFinite(Input.LongitudinalSpeedMps)
        || !FMath::IsFinite(Input.DeltaTimeSeconds)
        || Input.DeltaTimeSeconds <= 0.0
        || !FMath::IsFinite(InOutState.RelaxedSlipRatio)
        || !FMath::IsFinite(InOutState.RelaxedSlipAngleRad)
        || !FMath::IsFinite(InOutState.ContactAgeSeconds))
    {
        return false;
    }

    if (!Input.bContactValid)
    {
        const double DecayBlend01 =
            FirstOrderBlend(
                Input.DeltaTimeSeconds,
                Config.AirborneDecayTimeSeconds);

        InOutState.RelaxedSlipRatio =
            RelaxToward(
                InOutState.RelaxedSlipRatio,
                0.0,
                DecayBlend01);

        InOutState.RelaxedSlipAngleRad =
            RelaxToward(
                InOutState.RelaxedSlipAngleRad,
                0.0,
                DecayBlend01);

        InOutState.ContactAgeSeconds =
            0.0;

        InOutState.bWasInContact =
            false;

        OutOutput.RelaxedSlipRatio =
            InOutState.RelaxedSlipRatio;

        OutOutput.RelaxedSlipAngleRad =
            InOutState.RelaxedSlipAngleRad;

        OutOutput.LongitudinalBlend01 =
            DecayBlend01;

        OutOutput.LateralBlend01 =
            DecayBlend01;

        return true;
    }

    const double EffectiveSpeedMps =
        FMath::Max(
            FMath::Abs(
                Input.LongitudinalSpeedMps),
            Config.RelaxationSpeedFloorMps);

    OutOutput.LongitudinalTimeConstantSeconds =
        Config.LongitudinalRelaxationLengthM
        / EffectiveSpeedMps;

    OutOutput.LateralTimeConstantSeconds =
        Config.LateralRelaxationLengthM
        / EffectiveSpeedMps;

    OutOutput.LongitudinalBlend01 =
        FirstOrderBlend(
            Input.DeltaTimeSeconds,
            OutOutput.LongitudinalTimeConstantSeconds);

    OutOutput.LateralBlend01 =
        FirstOrderBlend(
            Input.DeltaTimeSeconds,
            OutOutput.LateralTimeConstantSeconds);

    InOutState.RelaxedSlipRatio =
        RelaxToward(
            InOutState.RelaxedSlipRatio,
            Input.TargetSlipRatio,
            OutOutput.LongitudinalBlend01);

    InOutState.RelaxedSlipAngleRad =
        RelaxToward(
            InOutState.RelaxedSlipAngleRad,
            Input.TargetSlipAngleRad,
            OutOutput.LateralBlend01);

    InOutState.ContactAgeSeconds +=
        Input.DeltaTimeSeconds;

    InOutState.bWasInContact =
        true;

    OutOutput.RelaxedSlipRatio =
        InOutState.RelaxedSlipRatio;

    OutOutput.RelaxedSlipAngleRad =
        InOutState.RelaxedSlipAngleRad;

    return true;
}
