#include "TAElectricDrive.h"

bool TAElectricDrive::ValidateConfig(
    const FTAElectricDriveConfig& Config)
{
    return
        TAHvBattery::ValidateConfig(Config.Battery)
        && TAElectricMotor::ValidateConfig(Config.Motor);
}

void TAElectricDrive::InitializeState(
    FTAElectricDriveState& OutState)
{
    OutState =
        FTAElectricDriveState{};
}

bool TAElectricDrive::Step(
    const FTAElectricDriveConfig& Config,
    const FTAElectricDriveInput& Input,
    FTAElectricDriveState& InOutState,
    FTAElectricDriveOutput& OutOutput)
{
    OutOutput =
        FTAElectricDriveOutput{};

    if (!ValidateConfig(Config)
        || !FMath::IsFinite(Input.RequestedMotorTorqueNm)
        || !FMath::IsFinite(Input.MotorAngularSpeedRadPerSec)
        || !FMath::IsFinite(Input.MotorHealth01)
        || !FMath::IsFinite(Input.DeltaTimeSeconds)
        || Input.DeltaTimeSeconds <= 0.0)
    {
        return false;
    }

    FTAElectricMotorInput MotorInput;
    MotorInput.RequestedTorqueNm =
        Input.RequestedMotorTorqueNm;

    MotorInput.AngularSpeedRadPerSec =
        Input.MotorAngularSpeedRadPerSec;

    MotorInput.Health01 =
        Input.MotorHealth01;

    if (!TAElectricMotor::Calculate(
            Config.Motor,
            MotorInput,
            OutOutput.UnconstrainedMotor))
    {
        return false;
    }

    FTAHvBatteryInput BatteryInput;
    BatteryInput.RequestedTerminalPowerW =
        OutOutput.UnconstrainedMotor.ElectricalPowerW;

    BatteryInput.DeltaTimeSeconds =
        Input.DeltaTimeSeconds;

    if (!TAHvBattery::Step(
            Config.Battery,
            BatteryInput,
            InOutState.Battery,
            OutOutput.Battery))
    {
        return false;
    }

    const double RequestedElectricalPowerW =
        OutOutput.UnconstrainedMotor.ElectricalPowerW;

    if (FMath::Abs(RequestedElectricalPowerW)
        > UE_DOUBLE_SMALL_NUMBER)
    {
        OutOutput.BatteryPowerAuthority01 =
            FMath::Clamp(
                FMath::Abs(
                    OutOutput.Battery.ActualTerminalPowerW)
                / FMath::Abs(RequestedElectricalPowerW),
                0.0,
                1.0);
    }
    else
    {
        OutOutput.BatteryPowerAuthority01 =
            1.0;
    }

    OutOutput.bBatteryLimited =
        OutOutput.Battery.bPowerLimited
        || OutOutput.BatteryPowerAuthority01
            < 1.0 - 1.0e-9;

    OutOutput.ActualMotorTorqueNm =
        OutOutput.UnconstrainedMotor.ActualTorqueNm
        * OutOutput.BatteryPowerAuthority01;

    OutOutput.ActualMechanicalPowerW =
        OutOutput.ActualMotorTorqueNm
        * Input.MotorAngularSpeedRadPerSec;

    OutOutput.ActualElectricalPowerW =
        OutOutput.Battery.ActualTerminalPowerW;

    OutOutput.MotorLossPowerW =
        FMath::Abs(
            OutOutput.ActualElectricalPowerW
            - OutOutput.ActualMechanicalPowerW);

    return true;
}
