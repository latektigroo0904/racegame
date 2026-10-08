#pragma once

#include "CoreMinimal.h"

struct TA_VEHICLE_API FTAFluidLeakConfig
{
    double FluidDensityKgPerM3 = 1000.0;
    double DischargeCoefficient = 0.62;
};

struct TA_VEHICLE_API FTAFluidReservoirState
{
    double MassKg = 0.0;
    double CumulativeLeakedMassKg = 0.0;
};

struct TA_VEHICLE_API FTAFluidLeakOutput
{
    double RequestedMassFlowKgPerSec = 0.0;
    double ActualMassFlowKgPerSec = 0.0;
    double LeakedMassThisStepKg = 0.0;
};

namespace TAFluidPrimitives
{
    TA_VEHICLE_API bool ValidateLeakConfig(
        const FTAFluidLeakConfig& Config);

    TA_VEHICLE_API double CalculateOrificeMassFlowKgPerSec(
        const FTAFluidLeakConfig& Config,
        double LeakAreaM2,
        double UpstreamPressurePa,
        double DownstreamPressurePa);

    TA_VEHICLE_API bool IntegrateReservoirLeak(
        const FTAFluidLeakConfig& Config,
        double LeakAreaM2,
        double UpstreamPressurePa,
        double DownstreamPressurePa,
        double DeltaTimeSeconds,
        FTAFluidReservoirState& InOutReservoir,
        FTAFluidLeakOutput& OutOutput);
}
