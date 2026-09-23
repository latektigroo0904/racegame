#pragma once

#include "CoreMinimal.h"

struct TA_TELEMETRY_API FTARegressionRange
{
    double MinInclusive = -TNumericLimits<double>::Max();
    double MaxInclusive = TNumericLimits<double>::Max();
};

struct TA_TELEMETRY_API FTARegressionMetricSummary
{
    int32 SampleCount = 0;
    double MinValue = 0.0;
    double MaxValue = 0.0;
    double MeanValue = 0.0;
    double SteadyStateMean = 0.0;
};

struct TA_TELEMETRY_API FTARegressionMetricResult
{
    FTARegressionMetricSummary Summary;
    bool bFinite = false;
    bool bMinInRange = false;
    bool bMaxInRange = false;
    bool bSteadyStateInRange = false;
    bool bPassed = false;
};

namespace TARegressionEnvelope
{
    // Summarizes a scalar trace without allocating. SteadyStateFraction01 is
    // the trailing fraction of samples used for the steady-state mean.
    TA_TELEMETRY_API bool Summarize(
        TConstArrayView<double> Samples,
        double SteadyStateFraction01,
        FTARegressionMetricSummary& OutSummary);

    TA_TELEMETRY_API FTARegressionMetricResult Evaluate(
        TConstArrayView<double> Samples,
        double SteadyStateFraction01,
        const FTARegressionRange& MinEnvelope,
        const FTARegressionRange& MaxEnvelope,
        const FTARegressionRange& SteadyStateEnvelope);
}
