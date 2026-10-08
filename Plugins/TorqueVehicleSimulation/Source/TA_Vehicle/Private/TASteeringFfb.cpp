#include "TASteeringFfb.h"

bool TASteeringFfb::ValidatePhysicsConfig(
    const FTASteeringFfbConfig& Config)
{
    return
        FMath::IsFinite(Config.EffectivePinionRadiusM)
        && Config.EffectivePinionRadiusM >= 0.0
        && FMath::IsFinite(Config.ColumnViscousDampingNmsPerRad)
        && Config.ColumnViscousDampingNmsPerRad >= 0.0
        && FMath::IsFinite(Config.ColumnCoulombFrictionNm)
        && Config.ColumnCoulombFrictionNm >= 0.0
        && FMath::IsFinite(Config.FrictionSmoothingSpeedRadPerSec)
        && Config.FrictionSmoothingSpeedRadPerSec
            > UE_DOUBLE_SMALL_NUMBER;
}

bool TASteeringFfb::CalculatePhysicalTorque(
    const FTASteeringFfbConfig& Config,
    const FTASteeringFfbInput& Input,
    FTASteeringFfbOutput& OutOutput)
{
    OutOutput =
        FTASteeringFfbOutput{};

    if (!ValidatePhysicsConfig(Config)
        || !FMath::IsFinite(Input.RackForceN)
        || !FMath::IsFinite(
            Input.SteeringWheelAngularVelocityRadPerSec)
        || !FMath::IsFinite(Input.AssistTorqueNm))
    {
        return false;
    }

    // Reaction opposes the rack force at the pinion.
    OutOutput.RackReactionTorqueNm =
        -Input.RackForceN
        * Config.EffectivePinionRadiusM;

    OutOutput.DampingTorqueNm =
        -Config.ColumnViscousDampingNmsPerRad
        * Input.SteeringWheelAngularVelocityRadPerSec;

    const double SmoothSign =
        FMath::Tanh(
            Input.SteeringWheelAngularVelocityRadPerSec
            / Config.FrictionSmoothingSpeedRadPerSec);

    OutOutput.FrictionTorqueNm =
        -Config.ColumnCoulombFrictionNm
        * SmoothSign;

    OutOutput.AssistTorqueNm =
        Input.AssistTorqueNm;

    OutOutput.PhysicalSteeringWheelTorqueNm =
        OutOutput.RackReactionTorqueNm
        + OutOutput.DampingTorqueNm
        + OutOutput.FrictionTorqueNm
        + OutOutput.AssistTorqueNm;

    return true;
}

bool TASteeringFfb::ValidateDeviceConfig(
    const FTAFfbDeviceConfig& Config)
{
    return
        FMath::IsFinite(Config.DeviceMaxTorqueNm)
        && Config.DeviceMaxTorqueNm > UE_DOUBLE_SMALL_NUMBER
        && FMath::IsFinite(Config.UserStrength01)
        && Config.UserStrength01 >= 0.0
        && Config.UserStrength01 <= 1.0;
}

bool TASteeringFfb::ScaleForDevice(
    const FTAFfbDeviceConfig& Config,
    const double PhysicalSteeringWheelTorqueNm,
    FTAFfbDeviceOutput& OutOutput)
{
    OutOutput =
        FTAFfbDeviceOutput{};

    if (!ValidateDeviceConfig(Config)
        || !FMath::IsFinite(
            PhysicalSteeringWheelTorqueNm))
    {
        return false;
    }

    OutOutput.RequestedPhysicalTorqueNm =
        PhysicalSteeringWheelTorqueNm;

    const double RequestedDeviceTorqueNm =
        PhysicalSteeringWheelTorqueNm
        * Config.UserStrength01;

    OutOutput.DeviceCommandTorqueNm =
        FMath::Clamp(
            RequestedDeviceTorqueNm,
            -Config.DeviceMaxTorqueNm,
            Config.DeviceMaxTorqueNm);

    OutOutput.bClipped =
        !FMath::IsNearlyEqual(
            OutOutput.DeviceCommandTorqueNm,
            RequestedDeviceTorqueNm,
            1.0e-12);

    return true;
}
