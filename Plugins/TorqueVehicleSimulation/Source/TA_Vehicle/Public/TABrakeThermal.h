#pragma once

#include "CoreMinimal.h"

struct TA_VEHICLE_API FTABrakeThermalConfig
{
    double ThermalMassJPerC = 35000.0;
    double CoolingWPerC = 70.0;
    double AmbientTemperatureC = 20.0;

    double HeatFraction01 = 0.95;

    double FadeStartTemperatureC = 450.0;
    double FadeEndTemperatureC = 750.0;
    double MinimumFadeTorqueFactor01 = 0.35;

    double WearEnergyCapacityJ = 4.0e8;
    double WearTorqueLossAtEnd01 = 0.25;
};

struct TA_VEHICLE_API FTABrakeThermalState
{
    double TemperatureC = 20.0;
    double Wear01 = 0.0;

    double ThermalTorqueFactor01 = 1.0;
    double WearTorqueFactor01 = 1.0;
};

struct TA_VEHICLE_API FTABrakeThermalOutput
{
    double FrictionPowerW = 0.0;
    double HeatPowerW = 0.0;
    double CoolingPowerW = 0.0;

    double GeneratedHeatEnergyJ = 0.0;

    double ThermalTorqueFactor01 = 1.0;
    double WearTorqueFactor01 = 1.0;
    double AvailableTorqueFactor01 = 1.0;
};

namespace TABrakeThermal
{
    TA_VEHICLE_API bool ValidateConfig(
        const FTABrakeThermalConfig& Config);

    TA_VEHICLE_API void InitializeState(
        const FTABrakeThermalConfig& Config,
        FTABrakeThermalState& OutState);

    TA_VEHICLE_API double CalculateThermalTorqueFactor01(
        const FTABrakeThermalConfig& Config,
        double TemperatureC);

    TA_VEHICLE_API double CalculateWearTorqueFactor01(
        const FTABrakeThermalConfig& Config,
        double Wear01);

    TA_VEHICLE_API double CalculateAvailableTorqueFactor01(
        const FTABrakeThermalConfig& Config,
        const FTABrakeThermalState& State);

    TA_VEHICLE_API bool Update(
        const FTABrakeThermalConfig& Config,
        double AppliedBrakeTorqueNm,
        double AverageAbsAngularSpeedRadPerSec,
        double DeltaTimeSeconds,
        FTABrakeThermalState& InOutState,
        FTABrakeThermalOutput& OutOutput);
}
