#pragma once

#include "CoreMinimal.h"

struct TA_POWERTRAIN_API FTAElectricMotorConfig
{
    double MaxMotoringTorqueNm = 450.0;
    double MaxRegenTorqueNm = 250.0;

    double MaxMotoringMechanicalPowerW = 220000.0;
    double MaxRegenMechanicalPowerW = 120000.0;

    double MaximumAngularSpeedRadPerSec = 1600.0;

    double MotoringEfficiency01 = 0.94;
    double RegenEfficiency01 = 0.90;
};

struct TA_POWERTRAIN_API FTAElectricMotorInput
{
    double RequestedTorqueNm = 0.0;
    double AngularSpeedRadPerSec = 0.0;
    double Health01 = 1.0;
};

struct TA_POWERTRAIN_API FTAElectricMotorOutput
{
    double ActualTorqueNm = 0.0;
    double MechanicalPowerW = 0.0;

    // Positive draws from battery, negative charges battery.
    double ElectricalPowerW = 0.0;

    double LossPowerW = 0.0;
    bool bTorqueLimited = false;
};

namespace TAElectricMotor
{
    TA_POWERTRAIN_API bool ValidateConfig(
        const FTAElectricMotorConfig& Config);

    TA_POWERTRAIN_API bool Calculate(
        const FTAElectricMotorConfig& Config,
        const FTAElectricMotorInput& Input,
        FTAElectricMotorOutput& OutOutput);
}
