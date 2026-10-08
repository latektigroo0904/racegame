#pragma once

#include "CoreMinimal.h"

struct TA_TIRE_API FTATireTransientConfig
{
    double LongitudinalRelaxationLengthM = 0.35;
    double LateralRelaxationLengthM = 0.45;
    double RelaxationSpeedFloorMps = 1.0;

    // When contact is lost, stale contact-patch shear memory decays toward zero.
    double AirborneDecayTimeSeconds = 0.08;
};

struct TA_TIRE_API FTATireTransientState
{
    double RelaxedSlipRatio = 0.0;
    double RelaxedSlipAngleRad = 0.0;

    double ContactAgeSeconds = 0.0;
    bool bWasInContact = false;
};

struct TA_TIRE_API FTATireTransientInput
{
    double TargetSlipRatio = 0.0;
    double TargetSlipAngleRad = 0.0;

    double LongitudinalSpeedMps = 0.0;
    bool bContactValid = true;

    double DeltaTimeSeconds = 1.0 / 240.0;
};

struct TA_TIRE_API FTATireTransientOutput
{
    double RelaxedSlipRatio = 0.0;
    double RelaxedSlipAngleRad = 0.0;

    double LongitudinalBlend01 = 0.0;
    double LateralBlend01 = 0.0;

    double LongitudinalTimeConstantSeconds = 0.0;
    double LateralTimeConstantSeconds = 0.0;
};

namespace TATireTransients
{
    TA_TIRE_API bool ValidateConfig(
        const FTATireTransientConfig& Config);

    TA_TIRE_API void InitializeState(
        FTATireTransientState& OutState);

    TA_TIRE_API bool Step(
        const FTATireTransientConfig& Config,
        const FTATireTransientInput& Input,
        FTATireTransientState& InOutState,
        FTATireTransientOutput& OutOutput);
}
