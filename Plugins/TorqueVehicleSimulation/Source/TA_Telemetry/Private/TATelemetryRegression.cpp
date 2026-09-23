#include "TATelemetryRegression.h"

namespace
{
    bool IsWheelMetric(
        const ETATelemetryMetric Metric)
    {
        switch (Metric)
        {
        case ETATelemetryMetric::WheelVerticalLoadN:
        case ETATelemetryMetric::SuspensionTravelM:
        case ETATelemetryMetric::TireRadialDeflectionM:
        case ETATelemetryMetric::WheelSlipRatio:
        case ETATelemetryMetric::TireSurfaceTemperatureC:
            return true;

        default:
            return false;
        }
    }

    bool IsFiniteEnvelope(
        const FTATelemetryMetricEnvelope& Envelope)
    {
        return
            FMath::IsFinite(Envelope.MinimumAllowed)
            && FMath::IsFinite(Envelope.MaximumAllowed)
            && Envelope.MinimumAllowed
                <= Envelope.MaximumAllowed
            && FMath::IsFinite(Envelope.StartFraction01)
            && FMath::IsFinite(Envelope.EndFraction01)
            && Envelope.StartFraction01 >= 0.0
            && Envelope.EndFraction01 <= 1.0
            && Envelope.EndFraction01
                > Envelope.StartFraction01;
    }

    bool ReadMetricValue(
        const FTAVehicleTelemetrySample& Sample,
        const FTATelemetryMetricEnvelope& Envelope,
        double& OutValue)
    {
        const int32 WheelIndex =
            Envelope.WheelIndex;

        switch (Envelope.Metric)
        {
        case ETATelemetryMetric::EngineRPM:
            OutValue =
                Sample.EngineRPM;
            break;

        case ETATelemetryMetric::ChassisSpeedMps:
            OutValue =
                Sample.ChassisLinearVelocityWorldMps.Length();
            break;

        case ETATelemetryMetric::ChassisForwardSpeedMps:
            OutValue =
                Sample.ChassisLinearVelocityWorldMps.X;
            break;

        case ETATelemetryMetric::ChassisVerticalSpeedMps:
            OutValue =
                Sample.ChassisLinearVelocityWorldMps.Z;
            break;

        case ETATelemetryMetric::ChassisYawRateRadPerSec:
            OutValue =
                Sample.ChassisAngularVelocityWorldRadPerSec.Z;
            break;

        case ETATelemetryMetric::TotalLongitudinalForceN:
            OutValue =
                Sample.TotalLongitudinalForceN;
            break;

        case ETATelemetryMetric::TotalLateralForceN:
            OutValue =
                Sample.TotalLateralForceN;
            break;

        case ETATelemetryMetric::WheelVerticalLoadN:
            OutValue =
                Sample.WheelVerticalLoadN[WheelIndex];
            break;

        case ETATelemetryMetric::SuspensionTravelM:
            OutValue =
                Sample.SuspensionTravelM[WheelIndex];
            break;

        case ETATelemetryMetric::TireRadialDeflectionM:
            OutValue =
                Sample.TireRadialDeflectionM[WheelIndex];
            break;

        case ETATelemetryMetric::WheelSlipRatio:
            OutValue =
                Sample.WheelSlipRatio[WheelIndex];
            break;

        case ETATelemetryMetric::TireSurfaceTemperatureC:
            OutValue =
                Sample.TireSurfaceTemperatureC[WheelIndex];
            break;

        default:
            return false;
        }

        return FMath::IsFinite(OutValue);
    }

    void CalculateWindowIndices(
        const int32 SampleCount,
        const FTATelemetryMetricEnvelope& Envelope,
        int32& OutStartIndex,
        int32& OutEndExclusive)
    {
        OutStartIndex =
            FMath::Clamp(
                FMath::FloorToInt(
                    static_cast<double>(SampleCount)
                    * Envelope.StartFraction01),
                0,
                FMath::Max(0, SampleCount - 1));

        OutEndExclusive =
            FMath::Clamp(
                FMath::CeilToInt(
                    static_cast<double>(SampleCount)
                    * Envelope.EndFraction01),
                OutStartIndex + 1,
                SampleCount);
    }

    bool EvaluateStatistic(
        const FTATelemetryRingBuffer& Buffer,
        const FTATelemetryMetricEnvelope& Envelope,
        double& OutObservedValue)
    {
        if (Buffer.Num() <= 0)
        {
            return false;
        }

        int32 StartIndex = 0;
        int32 EndExclusive = 0;

        CalculateWindowIndices(
            Buffer.Num(),
            Envelope,
            StartIndex,
            EndExclusive);

        bool bHasValue = false;
        double MinValue =
            TNumericLimits<double>::Max();

        double MaxValue =
            -TNumericLimits<double>::Max();

        double AbsoluteMaximumValue = 0.0;
        double Sum = 0.0;
        double FinalValue = 0.0;
        int32 Count = 0;

        for (int32 SampleIndex = StartIndex;
             SampleIndex < EndExclusive;
             ++SampleIndex)
        {
            const FTAVehicleTelemetrySample* Sample =
                Buffer.GetChronological(
                    SampleIndex);

            if (!Sample)
            {
                return false;
            }

            double Value = 0.0;

            if (!ReadMetricValue(
                    *Sample,
                    Envelope,
                    Value))
            {
                return false;
            }

            bHasValue = true;
            ++Count;

            MinValue =
                FMath::Min(
                    MinValue,
                    Value);

            MaxValue =
                FMath::Max(
                    MaxValue,
                    Value);

            if (FMath::Abs(Value)
                > FMath::Abs(AbsoluteMaximumValue))
            {
                AbsoluteMaximumValue =
                    Value;
            }

            Sum +=
                Value;

            FinalValue =
                Value;
        }

        if (!bHasValue ||
            Count <= 0)
        {
            return false;
        }

        switch (Envelope.Statistic)
        {
        case ETATelemetryStatistic::Minimum:
            OutObservedValue =
                MinValue;
            break;

        case ETATelemetryStatistic::Maximum:
            OutObservedValue =
                MaxValue;
            break;

        case ETATelemetryStatistic::Mean:
            OutObservedValue =
                Sum / static_cast<double>(Count);
            break;

        case ETATelemetryStatistic::Final:
            OutObservedValue =
                FinalValue;
            break;

        case ETATelemetryStatistic::AbsoluteMaximum:
            OutObservedValue =
                AbsoluteMaximumValue;
            break;

        default:
            return false;
        }

        return
            FMath::IsFinite(
                OutObservedValue);
    }

    FString MetricToString(
        const ETATelemetryMetric Metric)
    {
        switch (Metric)
        {
        case ETATelemetryMetric::EngineRPM:
            return TEXT("EngineRPM");
        case ETATelemetryMetric::ChassisSpeedMps:
            return TEXT("ChassisSpeedMps");
        case ETATelemetryMetric::ChassisForwardSpeedMps:
            return TEXT("ChassisForwardSpeedMps");
        case ETATelemetryMetric::ChassisVerticalSpeedMps:
            return TEXT("ChassisVerticalSpeedMps");
        case ETATelemetryMetric::ChassisYawRateRadPerSec:
            return TEXT("ChassisYawRateRadPerSec");
        case ETATelemetryMetric::TotalLongitudinalForceN:
            return TEXT("TotalLongitudinalForceN");
        case ETATelemetryMetric::TotalLateralForceN:
            return TEXT("TotalLateralForceN");
        case ETATelemetryMetric::WheelVerticalLoadN:
            return TEXT("WheelVerticalLoadN");
        case ETATelemetryMetric::SuspensionTravelM:
            return TEXT("SuspensionTravelM");
        case ETATelemetryMetric::TireRadialDeflectionM:
            return TEXT("TireRadialDeflectionM");
        case ETATelemetryMetric::WheelSlipRatio:
            return TEXT("WheelSlipRatio");
        case ETATelemetryMetric::TireSurfaceTemperatureC:
            return TEXT("TireSurfaceTemperatureC");
        default:
            return TEXT("UnknownMetric");
        }
    }

    FString StatisticToString(
        const ETATelemetryStatistic Statistic)
    {
        switch (Statistic)
        {
        case ETATelemetryStatistic::Minimum:
            return TEXT("Minimum");
        case ETATelemetryStatistic::Maximum:
            return TEXT("Maximum");
        case ETATelemetryStatistic::Mean:
            return TEXT("Mean");
        case ETATelemetryStatistic::Final:
            return TEXT("Final");
        case ETATelemetryStatistic::AbsoluteMaximum:
            return TEXT("AbsoluteMaximum");
        default:
            return TEXT("UnknownStatistic");
        }
    }
}

bool TATelemetryRegression::ValidateConfig(
    const FTATelemetryRegressionConfig& Config)
{
    if (Config.ScenarioId.IsNone() ||
        Config.MinimumRequiredSamples <= 0 ||
        Config.Envelopes.Num() <= 0)
    {
        return false;
    }

    for (const FTATelemetryMetricEnvelope& Envelope :
         Config.Envelopes)
    {
        if (!IsFiniteEnvelope(Envelope))
        {
            return false;
        }

        if (IsWheelMetric(Envelope.Metric) &&
            (Envelope.WheelIndex < 0 ||
             Envelope.WheelIndex >=
                TAPrototypeTelemetryWheelCount))
        {
            return false;
        }
    }

    return true;
}

bool TATelemetryRegression::Evaluate(
    const FTATelemetryRingBuffer& Buffer,
    const FTATelemetryRegressionConfig& Config,
    FTATelemetryRegressionResult& OutResult)
{
    OutResult =
        FTATelemetryRegressionResult{};

    if (!ValidateConfig(Config))
    {
        OutResult.Failures.Add(
            TEXT("Invalid regression configuration."));

        return false;
    }

    OutResult.SampleCount =
        Buffer.Num();

    if (Buffer.Num() <
        Config.MinimumRequiredSamples)
    {
        OutResult.Failures.Add(
            FString::Printf(
                TEXT("Scenario %s requires at least %d samples; got %d."),
                *Config.ScenarioId.ToString(),
                Config.MinimumRequiredSamples,
                Buffer.Num()));

        return true;
    }

    uint32 ObservedHash = 0;
    bool bObservedHashInitialized = false;
    bool bHashConsistent = true;

    for (int32 Index = 0;
         Index < Buffer.Num();
         ++Index)
    {
        const FTAVehicleTelemetrySample* Sample =
            Buffer.GetChronological(Index);

        if (!Sample)
        {
            OutResult.Failures.Add(
                TEXT("Telemetry buffer returned an invalid chronological sample."));

            return false;
        }

        if (!bObservedHashInitialized)
        {
            ObservedHash =
                Sample->PhysicsConfigHash;

            bObservedHashInitialized =
                true;
        }
        else if (Sample->PhysicsConfigHash
            != ObservedHash)
        {
            bHashConsistent =
                false;
        }
    }

    OutResult.bPhysicsConfigHashMatched =
        bHashConsistent
        && (Config.ExpectedPhysicsConfigHash == 0
            || ObservedHash
                == Config.ExpectedPhysicsConfigHash);

    if (!bHashConsistent)
    {
        OutResult.Failures.Add(
            TEXT("Telemetry trace contains more than one physics config hash."));
    }
    else if (Config.ExpectedPhysicsConfigHash != 0 &&
             ObservedHash != Config.ExpectedPhysicsConfigHash)
    {
        OutResult.Failures.Add(
            FString::Printf(
                TEXT("Physics config hash mismatch: expected %u, observed %u."),
                Config.ExpectedPhysicsConfigHash,
                ObservedHash));
    }

    OutResult.EnvelopeResults.Reserve(
        Config.Envelopes.Num());

    for (int32 EnvelopeIndex = 0;
         EnvelopeIndex < Config.Envelopes.Num();
         ++EnvelopeIndex)
    {
        const FTATelemetryMetricEnvelope& Envelope =
            Config.Envelopes[EnvelopeIndex];

        FTATelemetryEnvelopeResult EnvelopeResult;
        EnvelopeResult.EnvelopeIndex =
            EnvelopeIndex;

        EnvelopeResult.MinimumAllowed =
            Envelope.MinimumAllowed;

        EnvelopeResult.MaximumAllowed =
            Envelope.MaximumAllowed;

        double ObservedValue = 0.0;

        const bool bMetricValid =
            EvaluateStatistic(
                Buffer,
                Envelope,
                ObservedValue);

        EnvelopeResult.ObservedValue =
            ObservedValue;

        EnvelopeResult.bPassed =
            bMetricValid
            && ObservedValue >=
                Envelope.MinimumAllowed
            && ObservedValue <=
                Envelope.MaximumAllowed;

        if (EnvelopeResult.bPassed)
        {
            ++OutResult.PassedEnvelopeCount;
        }
        else
        {
            ++OutResult.FailedEnvelopeCount;

            OutResult.Failures.Add(
                FString::Printf(
                    TEXT(
                        "Envelope %d failed: %s/%s wheel=%d observed=%.9g expected=[%.9g, %.9g]."),
                    EnvelopeIndex,
                    *MetricToString(
                        Envelope.Metric),
                    *StatisticToString(
                        Envelope.Statistic),
                    Envelope.WheelIndex,
                    ObservedValue,
                    Envelope.MinimumAllowed,
                    Envelope.MaximumAllowed));
        }

        OutResult.EnvelopeResults.Add(
            EnvelopeResult);
    }

    OutResult.bPassed =
        OutResult.bPhysicsConfigHashMatched
        && OutResult.FailedEnvelopeCount == 0;

    return true;
}
