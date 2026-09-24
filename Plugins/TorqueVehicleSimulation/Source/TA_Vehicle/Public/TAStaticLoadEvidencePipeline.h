#pragma once

#include "CoreMinimal.h"
#include "TAStaticLoadAcceptance.h"
#include "TAStaticLoadEvidenceAdapter.h"
#include "TAStaticMassBalance.h"

struct TA_VEHICLE_API FTAStaticLoadEvidencePipelineConfig
{
    FTAStaticMassBalanceInput OracleInput;
    FTAStaticLoadEvidenceAdapterConfig Adapter;
    FTAStaticLoadSettledConfig Settled;
    FTAStaticLoadAcceptanceEnvelope Acceptance;
};

struct TA_VEHICLE_API FTAStaticLoadEvidencePipelineState
{
    FTAStaticLoadSettledState Settled;
};

struct TA_VEHICLE_API FTAStaticLoadEvidencePipelineResult
{
    bool bQualified = false;
    FTAStaticMassBalanceOutput Expected;
    FTAStaticLoadMeasuredMean Measured;
    FTAStaticLoadComparison Comparison;
    FTAStaticLoadAcceptanceResult Acceptance;
};

namespace TAStaticLoadEvidencePipeline
{
    TA_VEHICLE_API void Reset(FTAStaticLoadEvidencePipelineState& State);

    /**
     * Runs one evidence tick. A true return means the input tick was validly
     * processed; bQualified indicates whether enough consecutive settled evidence
     * exists to expose comparison/acceptance. Invalid adapter evidence resets the
     * qualification window fail-closed.
     */
    TA_VEHICLE_API bool Tick(
        const FTAStaticLoadEvidencePipelineConfig& Config,
        const FTAFourWheelRuntimeState& RuntimeState,
        const FTAFourWheelStepOutput& StepOutput,
        double DeltaTimeSeconds,
        FTAStaticLoadEvidencePipelineState& InOutState,
        FTAStaticLoadEvidencePipelineResult& OutResult);
}
