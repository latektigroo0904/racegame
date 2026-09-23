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

    double ThermalMassJPerC = 12000.0;
    double CoolingWPerC = 18.0;
    double AmbientTemperatureC = 20.0;
    double FadeStartTemperatureC = 220.0;
    double FadeEndTemperatureC = 420.0;

    // Effective friction-work budget used as a provisional wear calibration seed.
    double WearEnergyCapacityJ = 1.0e8;
};

struct TA_POWERTRAIN_API FTAClutchRuntimeState
{
    double Engagement = 0.0;

    double TemperatureC = 20.0;
    double Wear01 = 0.0;

    double ThermalCapacityFactor = 1.0;
    double WearCapacityFactor = 1.0;
};

struct TA_POWERTRAIN_API FTAGearboxRuntimeConfig
{
    TArray<double> ForwardGearRatios { 3.45, 2.15, 1.52, 1.16, 0.92, 0.76 };
    double ReverseGearRatio = 3.20;
    double FinalDriveRatio = 3.90;
    double MechanicalEfficiency = 0.96;
};

struct TA_POWERTRAIN_API FTADrivelineComplianceConfig
{
    double TorsionalStiffnessNmPerRad = 2500.0;
    double TorsionalDampingNmsPerRad = 25.0;
};

struct TA_POWERTRAIN_API FTADrivelineComplianceState
{
    double TwistRad = 0.0;
};

struct TA_POWERTRAIN_API FTAOpenDifferentialTorqueOutput
{
    double LeftWheelTorqueNm = 0.0;
    double RightWheelTorqueNm = 0.0;
    double UntransmittedInputTorqueNm = 0.0;
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

    TA_POWERTRAIN_API void UpdateClutchThermalAndWear(
        const FTAClutchRuntimeConfig& Config,
        double ClutchTorqueNm,
        double SlipAngularSpeedRadPerSec,
        double DeltaTimeSeconds,
        FTAClutchRuntimeState& InOutState);

    // Gear convention:
    // -1 = reverse, 0 = neutral, 1..N = forward gears.
    TA_POWERTRAIN_API double GetSelectedGearRatio(
        const FTAGearboxRuntimeConfig& Config,
        int32 SelectedGear);

    // Conventional reduction ratio G = input speed / output speed.
    TA_POWERTRAIN_API double CalculateGearboxOutputSpeedRadPerSec(
        double GearboxInputSpeedRadPerSec,
        double GearRatio);

    TA_POWERTRAIN_API double CalculateGearboxOutputTorqueNm(
        const FTAGearboxRuntimeConfig& Config,
        double GearboxInputTorqueNm,
        double GearRatio);

    TA_POWERTRAIN_API double CalculateFinalDriveOutputSpeedRadPerSec(
        const FTAGearboxRuntimeConfig& Config,
        double GearboxOutputSpeedRadPerSec);

    TA_POWERTRAIN_API double CalculateFinalDriveOutputTorqueNm(
        const FTAGearboxRuntimeConfig& Config,
        double GearboxOutputTorqueNm);

    TA_POWERTRAIN_API double CalculateCompliantShaftTorqueNm(
        const FTADrivelineComplianceConfig& Config,
        const FTADrivelineComplianceState& State,
        double RelativeAngularSpeedRadPerSec);

    TA_POWERTRAIN_API void IntegrateDrivelineTwist(
        double RelativeAngularSpeedRadPerSec,
        double DeltaTimeSeconds,
        FTADrivelineComplianceState& InOutState);

    // Quasi-static v0.1 approximation. Full coupled differential dynamics
    // will later solve carrier/side-gear inertias explicitly.
    TA_POWERTRAIN_API FTAOpenDifferentialTorqueOutput CalculateOpenDifferentialTorque(
        double DifferentialInputTorqueNm,
        double LeftReactionCapacityNm,
        double RightReactionCapacityNm);
}
