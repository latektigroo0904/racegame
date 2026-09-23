#include "TASurfaceTypes.h"

double TASurface::CalculateBaselineFrictionMultiplier(
    const FTASurfaceSample& Sample)
{
    const double Wetness = FMath::Clamp(Sample.Wetness01, 0.0, 1.0);
    const double Ice = FMath::Clamp(Sample.IceFraction01, 0.0, 1.0);

    const double DryWetBlend = FMath::Lerp(
        Sample.DryFrictionMultiplier,
        Sample.WetFrictionMultiplier,
        Wetness);

    constexpr double IceReferenceMultiplier = 0.10;

    return FMath::Max(
        0.0,
        FMath::Lerp(DryWetBlend, IceReferenceMultiplier, Ice));
}
