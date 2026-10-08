#include "TAStarterMotor.h"

bool TAStarterMotor::ValidateConfig(
    const FTAStarterMotorConfig& Config)
{
    return
        FMath::IsFinite(Config.WindingResistanceOhm)
        && Config.WindingResistanceOhm > UE_DOUBLE_SMALL_NUMBER
        && FMath::IsFinite(Config.TorqueConstantNmPerA)
        && Config.TorqueConstantNmPerA >= 0.0
        && FMath::IsFinite(Config.BackEmfConstantVPerRadPerSec)
        && Config.BackEmfConstantVPerRadPerSec >= 0.0
        && FMath::IsFinite(Config.PinionToCrankRatio)
        && Config.PinionToCrankRatio > UE_DOUBLE_SMALL_NUMBER
        && FMath::IsFinite(Config.GearEfficiency01)
        && Config.GearEfficiency01 >= 0.0
        && Config.GearEfficiency01 <= 1.0
        && FMath::IsFinite(Config.MaxCurrentA)
        && Config.MaxCurrentA >= 0.0;
}

bool TAStarterMotor::Calculate(
    const FTAStarterMotorConfig& Config,
    const FTAStarterMotorInput& Input,
    FTAStarterMotorOutput& OutOutput)
{
    OutOutput =
        FTAStarterMotorOutput{};

    if (!ValidateConfig(Config)
        || !FMath::IsFinite(Input.BusVoltageV)
        || Input.BusVoltageV < 0.0
        || !FMath::IsFinite(Input.EngineAngularSpeedRadPerSec)
        || Input.EngineAngularSpeedRadPerSec < 0.0
        || !FMath::IsFinite(Input.Health01))
    {
        return false;
    }

    if (!Input.bEngaged)
    {
        return true;
    }

    const double Health01 =
        FMath::Clamp(
            Input.Health01,
            0.0,
            1.0);

    OutOutput.MotorAngularSpeedRadPerSec =
        Input.EngineAngularSpeedRadPerSec
        * Config.PinionToCrankRatio;

    OutOutput.BackEmfVoltageV =
        OutOutput.MotorAngularSpeedRadPerSec
        * Config.BackEmfConstantVPerRadPerSec;

    const double AvailableWindingVoltageV =
        FMath::Max(
            0.0,
            Input.BusVoltageV
            - OutOutput.BackEmfVoltageV);

    OutOutput.CurrentA =
        FMath::Min(
            AvailableWindingVoltageV
                / Config.WindingResistanceOhm,
            Config.MaxCurrentA)
        * Health01;

    OutOutput.MotorTorqueNm =
        Config.TorqueConstantNmPerA
        * OutOutput.CurrentA;

    OutOutput.CrankshaftTorqueNm =
        OutOutput.MotorTorqueNm
        * Config.PinionToCrankRatio
        * Config.GearEfficiency01;

    OutOutput.ElectricalPowerW =
        Input.BusVoltageV
        * OutOutput.CurrentA;

    OutOutput.MechanicalPowerW =
        OutOutput.CrankshaftTorqueNm
        * Input.EngineAngularSpeedRadPerSec;

    return true;
}
