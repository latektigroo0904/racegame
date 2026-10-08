#pragma once

#include "CoreMinimal.h"
#include "TAElectricMotor.h"
#include "TAHvBattery.h"

struct TA_POWERTRAIN_API FTAElectricDriveConfig
{
    FTAHvBatteryConfig Battery;
    FTAElectricMotorConfig Motor;
};

struct TA_POWERTRAIN_API FTAElectricDriveState
{
    FTAHvBatteryState Battery;
};

struct TA_POWERTRAIN_API FTAElectricDriveInput
{
    double RequestedMotorTorqueNm = 0.0;
    double MotorAngularSpeedRadPerSec = 0.0;
    double MotorHealth01 = 1.0;
    double DeltaTimeSeconds = 1.0 / 240.0;
};

struct TA_POWERTRAIN_API FTAElectricDriveOutput
{
    FTAElectricMotorOutput UnconstrainedMotor;
    FTAHvBatteryOutput Battery;

    double BatteryPowerAuthority01 = 1.0;

    double ActualMotorTorqueNm = 0.0;
    double ActualMechanicalPowerW = 0.0;
    double ActualElectricalPowerW = 0.0;
    double MotorLossPowerW = 0.0;

    bool bBatteryLimited = false;
};

namespace TAElectricDrive
{
    TA_POWERTRAIN_API bool ValidateConfig(
        const FTAElectricDriveConfig& Config);

    TA_POWERTRAIN_API void InitializeState(
        FTAElectricDriveState& OutState);

    TA_POWERTRAIN_API bool Step(
        const FTAElectricDriveConfig& Config,
        const FTAElectricDriveInput& Input,
        FTAElectricDriveState& InOutState,
        FTAElectricDriveOutput& OutOutput);
}
