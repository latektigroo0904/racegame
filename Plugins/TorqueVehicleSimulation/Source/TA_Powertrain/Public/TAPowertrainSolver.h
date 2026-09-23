#pragma once

#include "CoreMinimal.h"

struct TA_POWERTRAIN_API FTAEngineRuntimeConfig
{
    double CrankInertiaKgm2 = 0.20;
    double FrictionConstantNm = 8.0;
    double FrictionLinearNms = 0.02;
    double FrictionQuadraticNms2 = 0.00002;
};

struct TA_POWERTRAIN_API FTAEngineRuntimeState
{
    double AngularSpeedRadPerSec = 0.0;
};

struct TA_POWERTRAIN_API FTAClutchRuntimeConfig
{
    double MaxTorqueCapacityNm = 500.0;
    double CouplingStiffnessNms = 20.0;
};

struct TA_POWERTRAIN_API FTAClutchRuntimeState
{
    double Engagement = 0.0;
    double ThermalCapacityFactor = 1.0;
    double WearCapacityFactor = 1.0;
};

namespace TAPowertrainSolver
{
    TA_POWERTRAIN_API double CalculateEngineFrictionTorqueNm(
        const FTAEngineRuntimeConfig& Config,
        double AngularSpeedRadPerSec);

    TA_POWERTRAIN_API double IntegrateEngineAngularSpeed(
        const FTAEngineRuntimeConfig& Config,
        double CurrentAngularSpeedRadPerSec,
        double CombustionTorqueNm,
        double ExternalLoadTorqueNm,
        double DeltaTimeSeconds);

    TA_POWERTRAIN_API double CalculateClutchTorqueNm(
        const FTAClutchRuntimeConfig& Config,
        const FTAClutchRuntimeState& State,
        double EngineAngularSpeedRadPerSec,
        double GearboxInputAngularSpeedRadPerSec);
}
