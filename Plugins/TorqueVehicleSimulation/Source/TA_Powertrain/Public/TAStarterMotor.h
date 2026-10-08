#pragma once

#include "CoreMinimal.h"

struct TA_POWERTRAIN_API FTAStarterMotorConfig
{
    double WindingResistanceOhm = 0.040;
    double TorqueConstantNmPerA = 0.018;
    double BackEmfConstantVPerRadPerSec = 0.018;

    double PinionToCrankRatio = 10.0;
    double GearEfficiency01 = 0.82;

    double MaxCurrentA = 500.0;
};

struct TA_POWERTRAIN_API FTAStarterMotorInput
{
    double BusVoltageV = 0.0;
    double EngineAngularSpeedRadPerSec = 0.0;

    bool bEngaged = false;
    double Health01 = 1.0;
};

struct TA_POWERTRAIN_API FTAStarterMotorOutput
{
    double MotorAngularSpeedRadPerSec = 0.0;
    double BackEmfVoltageV = 0.0;
    double CurrentA = 0.0;

    double MotorTorqueNm = 0.0;
    double CrankshaftTorqueNm = 0.0;

    double ElectricalPowerW = 0.0;
    double MechanicalPowerW = 0.0;
};

namespace TAStarterMotor
{
    TA_POWERTRAIN_API bool ValidateConfig(
        const FTAStarterMotorConfig& Config);

    TA_POWERTRAIN_API bool Calculate(
        const FTAStarterMotorConfig& Config,
        const FTAStarterMotorInput& Input,
        FTAStarterMotorOutput& OutOutput);
}
