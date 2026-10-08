#include "TADynamicSurface.h"

namespace
{
    double FirstOrderBlend(
        const double DeltaTimeSeconds,
        const double TimeConstantSeconds)
    {
        return FMath::Clamp(
            1.0
            - FMath::Exp(
                -DeltaTimeSeconds
                / FMath::Max(
                    TimeConstantSeconds,
                    UE_DOUBLE_SMALL_NUMBER)),
            0.0,
            1.0);
    }
}

bool TADynamicSurface::ValidateConfig(
    const FTADynamicSurfaceConfig& Config)
{
    return
        FMath::IsFinite(Config.MaxWaterDepthMm)
        && Config.MaxWaterDepthMm >= 0.0
        && FMath::IsFinite(Config.WetnessSaturationDepthMm)
        && Config.WetnessSaturationDepthMm > UE_DOUBLE_SMALL_NUMBER
        && FMath::IsFinite(Config.MaxDrainageRateMmPerSec)
        && Config.MaxDrainageRateMmPerSec >= 0.0
        && FMath::IsFinite(Config.DrainageReferenceDepthMm)
        && Config.DrainageReferenceDepthMm > UE_DOUBLE_SMALL_NUMBER
        && FMath::IsFinite(Config.EvaporationRateMmPerSecAt20C)
        && Config.EvaporationRateMmPerSecAt20C >= 0.0
        && FMath::IsFinite(Config.WindEvaporationGainPerMps)
        && Config.WindEvaporationGainPerMps >= 0.0
        && FMath::IsFinite(Config.TemperatureEvaporationGainPerC)
        && Config.TemperatureEvaporationGainPerC >= 0.0
        && FMath::IsFinite(
            Config.SurfaceTemperatureTimeConstantSeconds)
        && Config.SurfaceTemperatureTimeConstantSeconds
            > UE_DOUBLE_SMALL_NUMBER
        && FMath::IsFinite(Config.SolarHeatingCPerSecAtFull)
        && Config.SolarHeatingCPerSecAtFull >= 0.0;
}

void TADynamicSurface::InitializeState(
    const double InitialSurfaceTemperatureC,
    FTADynamicSurfaceState& OutState)
{
    OutState =
        FTADynamicSurfaceState{};

    OutState.SurfaceTemperatureC =
        FMath::IsFinite(InitialSurfaceTemperatureC)
        ? InitialSurfaceTemperatureC
        : 20.0;
}

bool TADynamicSurface::Step(
    const FTADynamicSurfaceConfig& Config,
    const FTADynamicSurfaceInput& Input,
    FTADynamicSurfaceState& InOutState,
    FTADynamicSurfaceOutput& OutOutput)
{
    OutOutput =
        FTADynamicSurfaceOutput{};

    if (!ValidateConfig(Config)
        || !FMath::IsFinite(Input.PrecipitationRateMmPerHour)
        || Input.PrecipitationRateMmPerHour < 0.0
        || !FMath::IsFinite(Input.Drainage01)
        || !FMath::IsFinite(Input.AmbientTemperatureC)
        || !FMath::IsFinite(Input.WindSpeedMps)
        || Input.WindSpeedMps < 0.0
        || !FMath::IsFinite(Input.SolarHeating01)
        || !FMath::IsFinite(Input.DeltaTimeSeconds)
        || Input.DeltaTimeSeconds <= 0.0
        || !FMath::IsFinite(InOutState.WaterDepthMm)
        || !FMath::IsFinite(InOutState.SurfaceTemperatureC))
    {
        return false;
    }

    InOutState.WaterDepthMm =
        FMath::Clamp(
            InOutState.WaterDepthMm,
            0.0,
            Config.MaxWaterDepthMm);

    OutOutput.RainInputRateMmPerSec =
        Input.PrecipitationRateMmPerHour
        / 3600.0;

    const double DrainageDepthFactor01 =
        FMath::Clamp(
            InOutState.WaterDepthMm
            / Config.DrainageReferenceDepthMm,
            0.0,
            1.0);

    OutOutput.DrainageRateMmPerSec =
        Config.MaxDrainageRateMmPerSec
        * FMath::Clamp(
            Input.Drainage01,
            0.0,
            1.0)
        * DrainageDepthFactor01;

    const double TemperatureAbove20C =
        FMath::Max(
            0.0,
            InOutState.SurfaceTemperatureC
            - 20.0);

    const double EvaporationMultiplier =
        1.0
        + Config.WindEvaporationGainPerMps
            * Input.WindSpeedMps
        + Config.TemperatureEvaporationGainPerC
            * TemperatureAbove20C;

    OutOutput.EvaporationRateMmPerSec =
        InOutState.WaterDepthMm > 0.0
        ? Config.EvaporationRateMmPerSecAt20C
            * EvaporationMultiplier
        : 0.0;

    const double NetWaterRateMmPerSec =
        OutOutput.RainInputRateMmPerSec
        - OutOutput.DrainageRateMmPerSec
        - OutOutput.EvaporationRateMmPerSec;

    InOutState.WaterDepthMm =
        FMath::Clamp(
            InOutState.WaterDepthMm
            + NetWaterRateMmPerSec
                * Input.DeltaTimeSeconds,
            0.0,
            Config.MaxWaterDepthMm);

    const double TemperatureBlend01 =
        FirstOrderBlend(
            Input.DeltaTimeSeconds,
            Config.SurfaceTemperatureTimeConstantSeconds);

    InOutState.SurfaceTemperatureC =
        InOutState.SurfaceTemperatureC
        + TemperatureBlend01
            * (Input.AmbientTemperatureC
                - InOutState.SurfaceTemperatureC)
        + Config.SolarHeatingCPerSecAtFull
            * FMath::Clamp(
                Input.SolarHeating01,
                0.0,
                1.0)
            * Input.DeltaTimeSeconds;

    OutOutput.WaterDepthMm =
        InOutState.WaterDepthMm;

    OutOutput.Wetness01 =
        FMath::Clamp(
            InOutState.WaterDepthMm
            / Config.WetnessSaturationDepthMm,
            0.0,
            1.0);

    OutOutput.SurfaceTemperatureC =
        InOutState.SurfaceTemperatureC;

    return true;
}

void TADynamicSurface::ApplyToSurfaceSample(
    const FTADynamicSurfaceOutput& DynamicState,
    FTASurfaceSample& InOutSample)
{
    InOutSample.WaterDepthMm =
        FMath::Max(
            0.0,
            DynamicState.WaterDepthMm);

    InOutSample.Wetness01 =
        FMath::Clamp(
            DynamicState.Wetness01,
            0.0,
            1.0);

    InOutSample.TemperatureC =
        DynamicState.SurfaceTemperatureC;
}
