#include "TAStaticLoadEvidencePipeline.h"

void TAStaticLoadEvidencePipeline::Reset(FTAStaticLoadEvidencePipelineState& State)
{
    TAStaticLoadSettledSampler::Reset(State.Settled);
}

bool TAStaticLoadEvidencePipeline::Tick(
    const FTAStaticLoadEvidencePipelineConfig& Config,
    const FTAFourWheelRuntimeState& RuntimeState,
    const FTAFourWheelStepOutput& StepOutput,
    double DeltaTimeSeconds,
    FTAStaticLoadEvidencePipelineState& InOutState,
    FTAStaticLoadEvidencePipelineResult& OutResult)
{
    OutResult = FTAStaticLoadEvidencePipelineResult{};

    if (!TAStaticMassBalance::Calculate(Config.OracleInput, OutResult.Expected))
    {
        Reset(InOutState);
        return false;
    }

    FTAStaticLoadSample Sample;
    if (!TAStaticLoadEvidenceAdapter::BuildSample(
        Config.Adapter, RuntimeState, StepOutput, DeltaTimeSeconds, Sample))
    {
        Reset(InOutState);
        return false;
    }

    TAStaticLoadSettledSampler::PushSample(
        Config.Settled, OutResult.Expected.TotalWeightN, Sample, InOutState.Settled);

    if (!TAStaticLoadSettledSampler::TryGetQualifiedMean(
        Config.Settled, InOutState.Settled, OutResult.Measured))
    {
        return true;
    }

    if (!TAStaticLoadComparison::Calculate(
        OutResult.Expected, OutResult.Measured, OutResult.Comparison) ||
        !TAStaticLoadAcceptance::Evaluate(
            Config.Acceptance, OutResult.Comparison, OutResult.Acceptance))
    {
        Reset(InOutState);
        OutResult = FTAStaticLoadEvidencePipelineResult{};
        return false;
    }

    OutResult.bQualified = true;
    return true;
}
