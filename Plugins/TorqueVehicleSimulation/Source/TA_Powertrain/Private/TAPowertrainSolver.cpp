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

double TAPowertrainSolver::EvaluateTorqueCurveNm(
    const FTAEngineRuntimeConfig& Config,
    const double RPM)
{
    if (Config.TorqueCurve.Num() == 0)
    {
        return 0.0;
    }

    if (Config.TorqueCurve.Num() == 1)
    {
        return Config.TorqueCurve[0].TorqueNm;
    }

    if (RPM <= Config.TorqueCurve[0].RPM)
    {
        return Config.TorqueCurve[0].TorqueNm;
    }

    for (int32 Index = 1; Index < Config.TorqueCurve.Num(); ++Index)
    {
        const FTAEngineTorquePoint& Lower = Config.TorqueCurve[Index - 1];
        const FTAEngineTorquePoint& Upper = Config.TorqueCurve[Index];

        if (RPM <= Upper.RPM)
        {
            const double Range = FMath::Max(1.0, Upper.RPM - Lower.RPM);
            const double Alpha = FMath::Clamp((RPM - Lower.RPM) / Range, 0.0, 1.0);
            return FMath::Lerp(Lower.TorqueNm, Upper.TorqueNm, Alpha);
        }
    }

    return Config.TorqueCurve.Last().TorqueNm;
}

double TAPowertrainSolver::CalculateCombustionTorqueNm(
    const FTAEngineRuntimeConfig& Config,
    const FTAEngineRuntimeState& State,
    const double Throttle01)
{
    if (State.RunState != ETAEngineRunState::Running)
    {
        return 0.0;
    }

    const double RPM =
        State.AngularSpeedRadPerSec * 60.0 / (2.0 * UE_DOUBLE_PI);

    if (RPM >= Config.LimiterRPM)
    {
        return 0.0;
    }

    const double Throttle = FMath::Clamp(Throttle01, 0.0, 1.0);
    const double BaseTorqueNm = EvaluateTorqueCurveNm(Config, RPM);

    double LimiterFactor = 1.0;
    if (RPM > Config.RedlineRPM)
    {
        const double Range = FMath::Max(1.0, Config.LimiterRPM - Config.RedlineRPM);
        LimiterFactor = 1.0 - FMath::Clamp((RPM - Config.RedlineRPM) / Range, 0.0, 1.0);
    }

    const double IdleErrorRPM = FMath::Max(0.0, Config.IdleRPM - RPM);
    const double IdleTorqueNm =
        FMath::Min(
            FMath::Max(0.0, Config.MaxIdleControlTorqueNm),
            IdleErrorRPM * FMath::Max(0.0, Config.IdleControlGainNmPerRPM));

    const double StateFactor =
        FMath::Clamp(State.ThermalTorqueFactor, 0.0, 1.0)
        * FMath::Clamp(State.DamageTorqueFactor, 0.0, 1.0);

    return FMath::Max(
        0.0,
        (BaseTorqueNm * Throttle * LimiterFactor + IdleTorqueNm)
        * StateFactor);
}

void TAPowertrainSolver::InitializeEngineThermalState(
    const FTAEngineThermalConfig& Config,
    FTAEngineThermalState& OutState)
{
    OutState = FTAEngineThermalState{};
    OutState.CoolantTemperatureC =
        FMath::Max(Config.AmbientTemperatureC, Config.InitialCoolantTemperatureC);
}

void TAPowertrainSolver::UpdateEngineThermalState(
    const FTAEngineThermalConfig& Config,
    const double Throttle01,
    const double EngineRPM,
    const double CoolingEfficiency01,
    const double DeltaTimeSeconds,
    FTAEngineThermalState& InOutThermalState,
    FTAEngineRuntimeState& InOutEngineState)
{
    if (DeltaTimeSeconds <= 0.0)
    {
        return;
    }

    const double Throttle = FMath::Clamp(Throttle01, 0.0, 1.0);
    const double CoolingEfficiency =
        FMath::Clamp(CoolingEfficiency01, 0.0, 1.0);

    const double RPMFactor =
        FMath::Clamp(
            EngineRPM / FMath::Max(1.0, InOutEngineState.RunState == ETAEngineRunState::Running ? 7000.0 : 1.0),
            0.0,
            1.5);

    const double GeneratedHeatW =
        FMath::Max(0.0, Config.BaseHeatGenerationW)
        + FMath::Max(0.0, Config.FullLoadAdditionalHeatW)
        * Throttle
        * FMath::Max(0.25, RPMFactor);

    const double TemperatureDeltaFromAmbient =
        FMath::Max(
            0.0,
            InOutThermalState.CoolantTemperatureC - Config.AmbientTemperatureC);

    const double RejectedHeatW =
        FMath::Max(0.0, Config.CoolingCapacityWPerC)
        * CoolingEfficiency
        * TemperatureDeltaFromAmbient;

    if (Config.EffectiveThermalMassJPerC > UE_DOUBLE_SMALL_NUMBER)
    {
        InOutThermalState.CoolantTemperatureC +=
            ((GeneratedHeatW - RejectedHeatW) * DeltaTimeSeconds)
            / Config.EffectiveThermalMassJPerC;
    }

    InOutThermalState.CoolantTemperatureC =
        FMath::Max(
            Config.AmbientTemperatureC,
            InOutThermalState.CoolantTemperatureC);

    const double DerateRange =
        FMath::Max(
            1.0,
            Config.DerateFullTemperatureC - Config.DerateStartTemperatureC);

    const double DerateAlpha =
        FMath::Clamp(
            (InOutThermalState.CoolantTemperatureC - Config.DerateStartTemperatureC)
            / DerateRange,
            0.0,
            1.0);

    InOutEngineState.ThermalTorqueFactor =
        FMath::Lerp(
            1.0,
            FMath::Clamp(Config.MinimumThermalTorqueFactor, 0.0, 1.0),
            DerateAlpha);

    if (InOutThermalState.CoolantTemperatureC > Config.DamageStartTemperatureC)
    {
        const double Severity =
            FMath::Clamp(
                (InOutThermalState.CoolantTemperatureC - Config.DamageStartTemperatureC)
                / FMath::Max(1.0, 150.0 - Config.DamageStartTemperatureC),
                0.0,
                2.0);

        InOutThermalState.ThermalDamage01 =
            FMath::Clamp(
                InOutThermalState.ThermalDamage01
                + Severity
                * FMath::Max(0.0, Config.DamageRatePerSecondAt150C)
                * DeltaTimeSeconds,
                0.0,
                1.0);

        InOutEngineState.DamageTorqueFactor =
            FMath::Clamp(
                1.0 - 0.85 * InOutThermalState.ThermalDamage01,
                0.10,
                1.0);
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
