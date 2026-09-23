#include "TAUnsprungVerticalDynamics.h"

bool TAUnsprungVerticalDynamics::ValidateConfig(
    const FTAUnsprungVerticalConfig& Config)
{
    return
        Config.EffectiveMassKg > UE_DOUBLE_SMALL_NUMBER
        && Config.MaxTravelM > Config.MinTravelM
        && Config.MaxAbsTravelVelocityMps > 0.0
        && Config.TravelLimitRestitution01 >= 0.0
        && Config.TravelLimitRestitution01 <= 1.0;
}

bool TAUnsprungVerticalDynamics::Integrate(
    const FTAUnsprungVerticalConfig& Config,
    const FTAUnsprungVerticalInput& Input,
    const double DeltaTimeSeconds,
    FTAUnsprungVerticalState& InOutState,
    FTAUnsprungVerticalOutput& OutOutput)
{
    OutOutput = FTAUnsprungVerticalOutput{};

    if (!ValidateConfig(Config) ||
        DeltaTimeSeconds <= 0.0)
    {
        return false;
    }

    const double NetForceN =
        Input.TireNormalForceN
        - Input.SuspensionReactionForceN
        + Input.ExternalGeneralizedForceN;

    OutOutput.TravelAccelerationMps2 =
        NetForceN / Config.EffectiveMassKg
        + Input.GravityAlongTravelAxisMps2
        - Input.ChassisAccelerationAlongTravelAxisMps2;

    InOutState.TravelVelocityMps +=
        OutOutput.TravelAccelerationMps2
        * DeltaTimeSeconds;

    InOutState.TravelVelocityMps =
        FMath::Clamp(
            InOutState.TravelVelocityMps,
            -Config.MaxAbsTravelVelocityMps,
            Config.MaxAbsTravelVelocityMps);

    InOutState.TravelM +=
        InOutState.TravelVelocityMps
        * DeltaTimeSeconds;

    const double Restitution =
        FMath::Clamp(
            Config.TravelLimitRestitution01,
            0.0,
            1.0);

    if (InOutState.TravelM < Config.MinTravelM)
    {
        OutOutput.bHitDroopLimit = true;

        OutOutput.LimitImpactSpeedMps =
            FMath::Max(
                OutOutput.LimitImpactSpeedMps,
                FMath::Abs(
                    InOutState.TravelVelocityMps));

        InOutState.TravelM =
            Config.MinTravelM;

        if (InOutState.TravelVelocityMps < 0.0)
        {
            InOutState.TravelVelocityMps =
                -InOutState.TravelVelocityMps
                * Restitution;
        }
    }
    else if (InOutState.TravelM > Config.MaxTravelM)
    {
        OutOutput.bHitBumpLimit = true;

        OutOutput.LimitImpactSpeedMps =
            FMath::Max(
                OutOutput.LimitImpactSpeedMps,
                FMath::Abs(
                    InOutState.TravelVelocityMps));

        InOutState.TravelM =
            Config.MaxTravelM;

        if (InOutState.TravelVelocityMps > 0.0)
        {
            InOutState.TravelVelocityMps =
                -InOutState.TravelVelocityMps
                * Restitution;
        }
    }

    return true;
}
