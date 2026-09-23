#pragma once

#include "CoreMinimal.h"

enum class ETASurfaceMaterial : uint8
{
    FreshAsphalt,
    PolishedAsphalt,
    Concrete,
    Cobble,
    CompactGravel,
    LooseGravel,
    Dirt,
    Mud,
    PackedSnow,
    Ice,
    Unknown
};

struct TA_SURFACE_API FTASurfaceSample
{
    ETASurfaceMaterial Material = ETASurfaceMaterial::FreshAsphalt;

    double DryFrictionMultiplier = 1.0;
    double WetFrictionMultiplier = 0.72;

    double Roughness = 0.0;
    double Drainage = 0.5;
    double TemperatureC = 20.0;

    double Wetness01 = 0.0;
    double WaterDepthMm = 0.0;
    double SnowDepthMm = 0.0;
    double IceFraction01 = 0.0;
    double LooseMaterialDepthMm = 0.0;
    double RubberLevel01 = 0.0;
    double DirtContamination01 = 0.0;
};

namespace TASurface
{
    TA_SURFACE_API double CalculateBaselineFrictionMultiplier(
        const FTASurfaceSample& Sample);
}
