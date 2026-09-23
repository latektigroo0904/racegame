#pragma once

#include "CoreMinimal.h"

struct TA_VEHICLE_API FTAUnsprungVerticalConfig
{
    double EffectiveMassKg = 42.0;

    double MinTravelM = -0.07;
    double MaxTravelM = 0.09;

    double TravelLimitRestitution01 = 0.0;
    double MaxAbsTravelVelocityMps = 12.0;
};

struct TA_VEHICLE_API FTAUnsprungVerticalState
{
    double TravelM = 0.0;
    double TravelVelocityMps = 0.0;
};

struct TA_VEHICLE_API FTAUnsprungVerticalInput
{
    double TireNormalForceN = 0.0;
    double SuspensionReactionForceN = 0.0;
    double ExternalGeneralizedForceN = 0.0;

    double GravityAlongTravelAxisMps2 = -9.80665;
    double ChassisAccelerationAlongTravelAxisMps2 = 0.0;
};

struct TA_VEHICLE_API FTAUnsprungVerticalOutput
{
    double TravelAccelerationMps2 = 0.0;

    bool bHitDroopLimit = false;
    bool bHitBumpLimit = false;

    double LimitImpactSpeedMps = 0.0;
};

namespace TAUnsprungVerticalDynamics
{
    TA_VEHICLE_API bool ValidateConfig(
        const FTAUnsprungVerticalConfig& Config);

    TA_VEHICLE_API bool Integrate(
        const FTAUnsprungVerticalConfig& Config,
        const FTAUnsprungVerticalInput& Input,
        double DeltaTimeSeconds,
        FTAUnsprungVerticalState& InOutState,
        FTAUnsprungVerticalOutput& OutOutput);
}
