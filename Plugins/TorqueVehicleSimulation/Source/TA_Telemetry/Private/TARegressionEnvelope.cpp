#include "TARegressionEnvelope.h"

namespace
{
    bool IsInside(const double Value, const FTARegressionRange& Range)
    {
        return FMath::IsFinite(Value)
            && FMath::IsFinite(Range.MinInclusive)
            && FMath::IsFinite(Range.MaxInclusive)
            && Range.MinInclusive <= Range.MaxInclusive
            && Value >= Range.MinInclusive
            && Value <= Range.MaxInclusive;
    }
}

bool TARegressionEnvelope::Summarize(
    const TConstArrayView<double> Samples,
    const double SteadyStateFraction01,
    FTARegressionMetricSummary& OutSummary)
{
    OutSummary = FTARegressionMetricSummary{};

    if (Samples.IsEmpty() ||
        !FMath::IsFinite(SteadyStateFraction01) ||
        SteadyStateFraction01 <= 0.0 ||
        SteadyStateFraction01 > 1.0)
    {
        return false;
    }

    double Sum = 0.0;
    double MinValue = TNumericLimits<double>::Max();
    double MaxValue = -TNumericLimits<double>::Max();

    for (const double Value : Samples)
    {
        if (!FMath::IsFinite(Value))
        {
            return false;
        }

        Sum += Value;
        MinValue = FMath::Min(MinValue, Value);
        MaxValue = FMath::Max(MaxValue, Value);
    }

    const int32 SteadyCount = FMath::Clamp(
        FMath::CeilToInt(static_cast<double>(Samples.Num()) * SteadyStateFraction01),
        1,
        Samples.Num());

    const int32 SteadyStart = Samples.Num() - SteadyCount;
    double SteadySum = 0.0;

    for (int32 Index = SteadyStart; Index < Samples.Num(); ++Index)
    {
        SteadySum += Samples[Index];
    }

    OutSummary.SampleCount = Samples.Num();
    OutSummary.MinValue = MinValue;
    OutSummary.MaxValue = MaxValue;
    OutSummary.MeanValue = Sum / static_cast<double>(Samples.Num());
    OutSummary.SteadyStateMean = SteadySum / static_cast<double>(SteadyCount);
    return true;
}

FTARegressionMetricResult TARegressionEnvelope::Evaluate(
    const TConstArrayView<double> Samples,
    const double SteadyStateFraction01,
    const FTARegressionRange& MinEnvelope,
    const FTARegressionRange& MaxEnvelope,
    const FTARegressionRange& SteadyStateEnvelope)
{
    FTARegressionMetricResult Result;
    Result.bFinite = Summarize(
        Samples,
        SteadyStateFraction01,
        Result.Summary);

    if (!Result.bFinite)
    {
        return Result;
    }

    Result.bMinInRange = IsInside(Result.Summary.MinValue, MinEnvelope);
    Result.bMaxInRange = IsInside(Result.Summary.MaxValue, MaxEnvelope);
    Result.bSteadyStateInRange = IsInside(
        Result.Summary.SteadyStateMean,
        SteadyStateEnvelope);
    Result.bPassed = Result.bMinInRange
        && Result.bMaxInRange
        && Result.bSteadyStateInRange;
    return Result;
}
