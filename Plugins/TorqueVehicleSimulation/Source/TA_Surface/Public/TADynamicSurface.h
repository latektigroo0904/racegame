#pragma once

#include "CoreMinimal.h"
#include "TASurfaceTypes.h"

struct TA_SURFACE_API FTADynamicSurfaceConfig
{
    double MaxWaterDepthMm = 20.0;
    double WetnessSaturationDepthMm = 0.50;

    double MaxDrainageRateMmPerSec = 0.25;
    double DrainageReferenceDepthMm = 2.0;

    double EvaporationRateMmPerSecAt20C = 0.0015;
    double WindEvaporationGainPerMps = 0.05;
    double TemperatureEvaporationGainPerC = 0.025;

    double SurfaceTemperatureTimeConstantSeconds = 300.0;
    double SolarHeatingCPerSecAtFull = 0.010;
};

struct TA_SURFACE_API FTADynamicSurfaceState
{
    double WaterDepthMm = 0.0;
    double SurfaceTemperatureC = 20.0;
};

struct TA_SURFACE_API FTADynamicSurfaceInput
{
    double PrecipitationRateMmPerHour = 0.0;
    double Drainage01 = 0.5;

    double AmbientTemperatureC = 20.0;
    double WindSpeedMps = 0.0;
    double SolarHeating01 = 0.0;

    double DeltaTimeSeconds = 0.1;
};

struct TA_SURFACE_API FTADynamicSurfaceOutput
{
    double RainInputRateMmPerSec = 0.0;
    double DrainageRateMmPerSec = 0.0;
    double EvaporationRateMmPerSec = 0.0;

    double WaterDepthMm = 0.0;
    double Wetness01 = 0.0;
    double SurfaceTemperatureC = 20.0;
};

namespace TADynamicSurface
{
    TA_SURFACE_API bool ValidateConfig(
        const FTADynamicSurfaceConfig& Config);

    TA_SURFACE_API void InitializeState(
        double InitialSurfaceTemperatureC,
        FTADynamicSurfaceState& OutState);

    TA_SURFACE_API bool Step(
        const FTADynamicSurfaceConfig& Config,
        const FTADynamicSurfaceInput& Input,
        FTADynamicSurfaceState& InOutState,
        FTADynamicSurfaceOutput& OutOutput);

    TA_SURFACE_API void ApplyToSurfaceSample(
        const FTADynamicSurfaceOutput& DynamicState,
        FTASurfaceSample& InOutSample);
}
