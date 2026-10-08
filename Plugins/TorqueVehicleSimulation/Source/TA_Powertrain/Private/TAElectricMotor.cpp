#include "TAElectricMotor.h"

bool TAElectricMotor::ValidateConfig(
    const FTAElectricMotorConfig& Config)
{
    return
        FMath::IsFinite(Config.MaxMotoringTorqueNm)
        && Config.MaxMotoringTorqueNm >= 0.0
        && FMath::IsFinite(Config.MaxRegenTorqueNm)
        && Config.MaxRegenTorqueNm >= 0.0
        && FMath::IsFinite(Config.MaxMotoringMechanicalPowerW)
        && Config.MaxMotoringMechanicalPowerW >= 0.0
        && FMath::IsFinite(Config.MaxRegenMechanicalPowerW)
        && Config.MaxRegenMechanicalPowerW >= 0.0
        && FMath::IsFinite(Config.MaximumAngularSpeedRadPerSec)
        && Config.MaximumAngularSpeedRadPerSec > 0.0
        && FMath::IsFinite(Config.MotoringEfficiency01)
        && Config.MotoringEfficiency01 > UE_DOUBLE_SMALL_NUMBER
        && Config.MotoringEfficiency01 <= 1.0
        && FMath::IsFinite(Config.RegenEfficiency01)
        && Config.RegenEfficiency01 > UE_DOUBLE_SMALL_NUMBER
        && Config.RegenEfficiency01 <= 1.0;
}

bool TAElectricMotor::Calculate(
    const FTAElectricMotorConfig& Config,
    const FTAElectricMotorInput& Input,
    FTAElectricMotorOutput& OutOutput)
{
    OutOutput = FTAElectricMotorOutput{};

    if (!ValidateConfig(Config)
        || !FMath::IsFinite(Input.RequestedTorqueNm)
        || !FMath::IsFinite(Input.AngularSpeedRadPerSec)
        || !FMath::IsFinite(Input.Health01))
    {
        return false;
    }

    const double AbsSpeed =
        FMath::Abs(Input.AngularSpeedRadPerSec);

    if (AbsSpeed >= Config.MaximumAngularSpeedRadPerSec)
    {
        OutOutput.bTorqueLimited =
            !FMath::IsNearlyZero(Input.RequestedTorqueNm);

        return true;
    }

    const bool bMotoring =
        Input.RequestedTorqueNm
        * Input.AngularSpeedRadPerSec >= 0.0;

    const double TorqueLimitNm =
        bMotoring
        ? Config.MaxMotoringTorqueNm
        : Config.MaxRegenTorqueNm;

    const double PowerLimitW =
        bMotoring
        ? Config.MaxMotoringMechanicalPowerW
        : Config.MaxRegenMechanicalPowerW;

    double EffectiveTorqueLimitNm =
        TorqueLimitNm;

    if (AbsSpeed > UE_DOUBLE_SMALL_NUMBER
        && PowerLimitW > 0.0)
    {
        EffectiveTorqueLimitNm =
            FMath::Min(
                EffectiveTorqueLimitNm,
                PowerLimitW / AbsSpeed);
    }

    EffectiveTorqueLimitNm *=
        FMath::Clamp(Input.Health01, 0.0, 1.0);

    OutOutput.ActualTorqueNm =
        FMath::Clamp(
            Input.RequestedTorqueNm,
            -EffectiveTorqueLimitNm,
            EffectiveTorqueLimitNm);

    OutOutput.bTorqueLimited =
        !FMath::IsNearlyEqual(
            OutOutput.ActualTorqueNm,
            Input.RequestedTorqueNm,
            1.0e-9);

    OutOutput.MechanicalPowerW =
        OutOutput.ActualTorqueNm
        * Input.AngularSpeedRadPerSec;

    if (OutOutput.MechanicalPowerW >= 0.0)
    {
        OutOutput.ElectricalPowerW =
            OutOutput.MechanicalPowerW
            / Config.MotoringEfficiency01;
    }
    else
    {
        OutOutput.ElectricalPowerW =
            OutOutput.MechanicalPowerW
            * Config.RegenEfficiency01;
    }

    OutOutput.LossPowerW =
        FMath::Abs(
            OutOutput.ElectricalPowerW
            - OutOutput.MechanicalPowerW);

    return true;
}
