#include "TAPowertrainSolver.h"

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
