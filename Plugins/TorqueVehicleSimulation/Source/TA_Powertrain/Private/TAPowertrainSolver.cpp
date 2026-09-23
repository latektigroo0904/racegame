#include "TAPowertrainSolver.h"

namespace
{
    double CalculateFadeFactor(
        const FTAClutchRuntimeConfig& Config,
        const double TemperatureC)
    {
        if (TemperatureC <= Config.FadeStartTemperatureC)
        {
            return 1.0;
        }

        if (TemperatureC >= Config.FadeEndTemperatureC)
        {
            return 0.35;
        }

        const double Range =
            FMath::Max(1.0, Config.FadeEndTemperatureC - Config.FadeStartTemperatureC);

        const double T =
            FMath::Clamp(
                (TemperatureC - Config.FadeStartTemperatureC) / Range,
                0.0,
                1.0);

        return FMath::Lerp(1.0, 0.35, T);
    }
}

double TAPowertrainSolver::CalculateEngineFrictionTorqueNm(
    const FTAEngineRuntimeConfig& Config,
    const double AngularSpeedRadPerSec)
{
    const double Omega = FMath::Max(0.0, AngularSpeedRadPerSec);

    return Config.FrictionConstantNm
        + Config.FrictionLinearNms * Omega
        + Config.FrictionQuadraticNms2 * Omega * Omega;
}

double TAPowertrainSolver::IntegrateEngineAngularSpeed(
    const FTAEngineRuntimeConfig& Config,
    const double CurrentAngularSpeedRadPerSec,
    const double CombustionTorqueNm,
    const double ExternalLoadTorqueNm,
    const double DeltaTimeSeconds)
{
    if (Config.CrankInertiaKgm2 <= SMALL_NUMBER || DeltaTimeSeconds <= 0.0)
    {
        return FMath::Max(0.0, CurrentAngularSpeedRadPerSec);
    }

    const double FrictionTorqueNm =
        CalculateEngineFrictionTorqueNm(Config, CurrentAngularSpeedRadPerSec);

    const double NetTorqueNm =
        CombustionTorqueNm - FrictionTorqueNm - ExternalLoadTorqueNm;

    const double AngularAcceleration =
        NetTorqueNm / Config.CrankInertiaKgm2;

    return FMath::Max(
        0.0,
        CurrentAngularSpeedRadPerSec + AngularAcceleration * DeltaTimeSeconds);
}

double TAPowertrainSolver::CalculateClutchTorqueNm(
    const FTAClutchRuntimeConfig& Config,
    const FTAClutchRuntimeState& State,
    const double EngineAngularSpeedRadPerSec,
    const double GearboxInputAngularSpeedRadPerSec)
{
    const double Engagement = FMath::Clamp(State.Engagement, 0.0, 1.0);
    const double ThermalFactor = FMath::Clamp(State.ThermalCapacityFactor, 0.0, 1.0);
    const double WearFactor = FMath::Clamp(State.WearCapacityFactor, 0.0, 1.0);

    const double CapacityNm =
        FMath::Max(0.0, Config.MaxTorqueCapacityNm)
        * Engagement
        * ThermalFactor
        * WearFactor;

    const double SlipRadPerSec =
        EngineAngularSpeedRadPerSec - GearboxInputAngularSpeedRadPerSec;

    const double RequestedTorqueNm =
        Config.CouplingStiffnessNms * SlipRadPerSec;

    return FMath::Clamp(RequestedTorqueNm, -CapacityNm, CapacityNm);
}

void TAPowertrainSolver::UpdateClutchThermalAndWear(
    const FTAClutchRuntimeConfig& Config,
    const double ClutchTorqueNm,
    const double SlipAngularSpeedRadPerSec,
    const double DeltaTimeSeconds,
    FTAClutchRuntimeState& InOutState)
{
    if (DeltaTimeSeconds <= 0.0)
    {
        return;
    }

    const double SlipPowerW =
        FMath::Abs(ClutchTorqueNm * SlipAngularSpeedRadPerSec);

    const double CoolingW =
        FMath::Max(
            0.0,
            InOutState.TemperatureC - Config.AmbientTemperatureC)
        * FMath::Max(0.0, Config.CoolingWPerC);

    const double NetEnergyJ =
        (SlipPowerW - CoolingW) * DeltaTimeSeconds;

    if (Config.ThermalMassJPerC > SMALL_NUMBER)
    {
        InOutState.TemperatureC =
            FMath::Max(
                Config.AmbientTemperatureC,
                InOutState.TemperatureC
                + NetEnergyJ / Config.ThermalMassJPerC);
    }

    if (Config.WearEnergyCapacityJ > SMALL_NUMBER)
    {
        InOutState.Wear01 =
            FMath::Clamp(
                InOutState.Wear01
                + (SlipPowerW * DeltaTimeSeconds) / Config.WearEnergyCapacityJ,
                0.0,
                1.0);
    }

    InOutState.ThermalCapacityFactor =
        CalculateFadeFactor(Config, InOutState.TemperatureC);

    InOutState.WearCapacityFactor =
        FMath::Clamp(1.0 - 0.65 * InOutState.Wear01, 0.20, 1.0);
}

double TAPowertrainSolver::GetSelectedGearRatio(
    const FTAGearboxRuntimeConfig& Config,
    const int32 SelectedGear)
{
    if (SelectedGear == 0)
    {
        return 0.0;
    }

    if (SelectedGear < 0)
    {
        return -FMath::Abs(Config.ReverseGearRatio);
    }

    const int32 Index = SelectedGear - 1;
    if (!Config.ForwardGearRatios.IsValidIndex(Index))
    {
        return 0.0;
    }

    return Config.ForwardGearRatios[Index];
}

double TAPowertrainSolver::CalculateGearboxOutputSpeedRadPerSec(
    const double GearboxInputSpeedRadPerSec,
    const double GearRatio)
{
    if (FMath::Abs(GearRatio) <= SMALL_NUMBER)
    {
        return 0.0;
    }

    return GearboxInputSpeedRadPerSec / GearRatio;
}

double TAPowertrainSolver::CalculateGearboxOutputTorqueNm(
    const FTAGearboxRuntimeConfig& Config,
    const double GearboxInputTorqueNm,
    const double GearRatio)
{
    if (FMath::Abs(GearRatio) <= SMALL_NUMBER)
    {
        return 0.0;
    }

    return GearboxInputTorqueNm
        * GearRatio
        * FMath::Clamp(Config.MechanicalEfficiency, 0.0, 1.0);
}

double TAPowertrainSolver::CalculateFinalDriveOutputSpeedRadPerSec(
    const FTAGearboxRuntimeConfig& Config,
    const double GearboxOutputSpeedRadPerSec)
{
    if (FMath::Abs(Config.FinalDriveRatio) <= SMALL_NUMBER)
    {
        return 0.0;
    }

    return GearboxOutputSpeedRadPerSec / Config.FinalDriveRatio;
}

double TAPowertrainSolver::CalculateFinalDriveOutputTorqueNm(
    const FTAGearboxRuntimeConfig& Config,
    const double GearboxOutputTorqueNm)
{
    if (FMath::Abs(Config.FinalDriveRatio) <= SMALL_NUMBER)
    {
        return 0.0;
    }

    return GearboxOutputTorqueNm
        * Config.FinalDriveRatio
        * FMath::Clamp(Config.MechanicalEfficiency, 0.0, 1.0);
}

double TAPowertrainSolver::CalculateCompliantShaftTorqueNm(
    const FTADrivelineComplianceConfig& Config,
    const FTADrivelineComplianceState& State,
    const double RelativeAngularSpeedRadPerSec)
{
    return Config.TorsionalStiffnessNmPerRad * State.TwistRad
        + Config.TorsionalDampingNmsPerRad * RelativeAngularSpeedRadPerSec;
}

void TAPowertrainSolver::IntegrateDrivelineTwist(
    const double RelativeAngularSpeedRadPerSec,
    const double DeltaTimeSeconds,
    FTADrivelineComplianceState& InOutState)
{
    if (DeltaTimeSeconds <= 0.0)
    {
        return;
    }

    InOutState.TwistRad += RelativeAngularSpeedRadPerSec * DeltaTimeSeconds;
}

FTAOpenDifferentialTorqueOutput TAPowertrainSolver::CalculateOpenDifferentialTorque(
    const double DifferentialInputTorqueNm,
    const double LeftReactionCapacityNm,
    const double RightReactionCapacityNm)
{
    FTAOpenDifferentialTorqueOutput Output;

    const double Sign =
        DifferentialInputTorqueNm >= 0.0 ? 1.0 : -1.0;

    const double RequestedPerWheelNm =
        0.5 * FMath::Abs(DifferentialInputTorqueNm);

    const double TransmittablePerWheelNm =
        FMath::Min3(
            RequestedPerWheelNm,
            FMath::Max(0.0, LeftReactionCapacityNm),
            FMath::Max(0.0, RightReactionCapacityNm));

    Output.LeftWheelTorqueNm = Sign * TransmittablePerWheelNm;
    Output.RightWheelTorqueNm = Sign * TransmittablePerWheelNm;

    const double TransmittedTotalNm =
        2.0 * TransmittablePerWheelNm;

    Output.UntransmittedInputTorqueNm =
        Sign * FMath::Max(
            0.0,
            FMath::Abs(DifferentialInputTorqueNm) - TransmittedTotalNm);

    return Output;
}
