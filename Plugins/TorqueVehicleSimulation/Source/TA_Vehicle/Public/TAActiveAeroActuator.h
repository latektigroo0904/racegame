#pragma once

#include "CoreMinimal.h"

struct TA_VEHICLE_API FTAActiveAeroActuatorConfig
{
    double MinimumPosition01 = 0.0;
    double MaximumPosition01 = 1.0;

    double ExtendRate01PerSec = 1.5;
    double RetractRate01PerSec = 2.0;

    double FailSafePosition01 = 0.0;
    bool bReturnToFailSafeWithoutPower = true;
};

struct TA_VEHICLE_API FTAActiveAeroActuatorState
{
    double Position01 = 0.0;
};

struct TA_VEHICLE_API FTAActiveAeroActuatorInput
{
    double TargetPosition01 = 0.0;
    double Health01 = 1.0;

    bool bPowered = true;
    bool bMechanicallyStuck = false;

    double DeltaTimeSeconds = 1.0 / 240.0;
};

struct TA_VEHICLE_API FTAActiveAeroActuatorOutput
{
    double RequestedTargetPosition01 = 0.0;
    double EffectiveTargetPosition01 = 0.0;
    double Position01 = 0.0;

    bool bRateLimited = false;
    bool bFailSafeCommanded = false;
    bool bStuck = false;
};

namespace TAActiveAeroActuator
{
    TA_VEHICLE_API bool ValidateConfig(
        const FTAActiveAeroActuatorConfig& Config);

    TA_VEHICLE_API void InitializeState(
        const FTAActiveAeroActuatorConfig& Config,
        FTAActiveAeroActuatorState& OutState);

    TA_VEHICLE_API bool Step(
        const FTAActiveAeroActuatorConfig& Config,
        const FTAActiveAeroActuatorInput& Input,
        FTAActiveAeroActuatorState& InOutState,
        FTAActiveAeroActuatorOutput& OutOutput);
}
