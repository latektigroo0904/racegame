#pragma once

#include "CoreMinimal.h"

struct TA_VEHICLE_API FTASteeringFfbConfig
{
    // Effective rack-pinion moment arm used to convert rack force into
    // steering-column reaction torque.
    double EffectivePinionRadiusM = 0.012;

    double ColumnViscousDampingNmsPerRad = 0.08;
    double ColumnCoulombFrictionNm = 0.20;

    // Smooth-sign reference speed avoids discontinuity at zero velocity.
    double FrictionSmoothingSpeedRadPerSec = 0.20;
};

struct TA_VEHICLE_API FTASteeringFfbInput
{
    double RackForceN = 0.0;
    double SteeringWheelAngularVelocityRadPerSec = 0.0;

    // Physical steering-assist torque at the column. Positive follows the
    // steering-wheel positive torque convention.
    double AssistTorqueNm = 0.0;
};

struct TA_VEHICLE_API FTASteeringFfbOutput
{
    double RackReactionTorqueNm = 0.0;
    double DampingTorqueNm = 0.0;
    double FrictionTorqueNm = 0.0;
    double AssistTorqueNm = 0.0;

    // Physical steering-wheel torque before device/user scaling.
    double PhysicalSteeringWheelTorqueNm = 0.0;
};

struct TA_VEHICLE_API FTAFfbDeviceConfig
{
    double DeviceMaxTorqueNm = 10.0;
    double UserStrength01 = 1.0;
};

struct TA_VEHICLE_API FTAFfbDeviceOutput
{
    double RequestedPhysicalTorqueNm = 0.0;
    double DeviceCommandTorqueNm = 0.0;
    bool bClipped = false;
};

namespace TASteeringFfb
{
    TA_VEHICLE_API bool ValidatePhysicsConfig(
        const FTASteeringFfbConfig& Config);

    TA_VEHICLE_API bool CalculatePhysicalTorque(
        const FTASteeringFfbConfig& Config,
        const FTASteeringFfbInput& Input,
        FTASteeringFfbOutput& OutOutput);

    TA_VEHICLE_API bool ValidateDeviceConfig(
        const FTAFfbDeviceConfig& Config);

    TA_VEHICLE_API bool ScaleForDevice(
        const FTAFfbDeviceConfig& Config,
        double PhysicalSteeringWheelTorqueNm,
        FTAFfbDeviceOutput& OutOutput);
}
