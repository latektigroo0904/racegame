#pragma once

#include "CoreMinimal.h"
#include "TAFourWheelVehicleRuntime.h"
#include "TAStaticLoadSettledSampler.h"

struct TA_VEHICLE_API FTAStaticLoadEvidenceAdapterConfig
{
    // Evidence must be sampled at a fixed physics cadence. The adapter rejects
    // cadence drift instead of silently changing the settled-window duration.
    double ExpectedDeltaTimeSeconds = 1.0 / 120.0;
    double DeltaTimeToleranceSeconds = 1.0e-6;
};

namespace TAStaticLoadEvidenceAdapter
{
    /**
     * Converts canonical four-wheel runtime evidence into the sampler contract.
     * VerticalLoadN is already defined as a non-negative support-load magnitude;
     * no second sign inversion is permitted here.
     */
    TA_VEHICLE_API bool BuildSample(
        const FTAStaticLoadEvidenceAdapterConfig& Config,
        const FTAFourWheelRuntimeState& RuntimeState,
        const FTAFourWheelStepOutput& StepOutput,
        double DeltaTimeSeconds,
        FTAStaticLoadSample& OutSample);
}
