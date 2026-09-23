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
        case ETATelemetryMetric::WheelCamberRad:
        case ETATelemetryMetric::WheelToeRad:
        case ETATelemetryMetric::TireRadialDeflectionM:
        case ETATelemetryMetric::WheelSlipRatio:
        case ETATelemetryMetric::TireLongitudinalForceN:
        case ETATelemetryMetric::TireLateralForceN:
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

        case ETATelemetryMetric::CoolingEfficiency01:
            OutValue =
                Sample.CoolingEfficiency01;
            break;

        case ETATelemetryMetric::CoolantTemperatureC:
            OutValue =
                Sample.CoolantTemperatureC;
            break;

        case ETATelemetryMetric::SteeringRackDisplacementM:
            OutValue =
                Sample.SteeringRackDisplacementM;
            break;

        case ETATelemetryMetric::WheelVerticalLoadN:
            OutValue =
                Sample.WheelVerticalLoadN[WheelIndex];
            break;

        case ETATelemetryMetric::SuspensionTravelM:
            OutValue =
                Sample.SuspensionTravelM[WheelIndex];
            break;

        case ETATelemetryMetric::WheelCamberRad:
            OutValue =
                Sample.WheelCamberRad[WheelIndex];
            break;

        case ETATelemetryMetric::WheelToeRad:
            OutValue =
                Sample.WheelToeRad[WheelIndex];
            break;

        case ETATelemetryMetric::TireRadialDeflectionM:
            OutValue =
                Sample.TireRadialDeflectionM[WheelIndex];
            break;

        case ETATelemetryMetric::WheelSlipRatio:
            OutValue =
                Sample.WheelSlipRatio[WheelIndex];
            break;

        case ETATelemetryMetric::TireLongitudinalForceN:
            OutValue =
                Sample.TireLongitudinalForceN[WheelIndex];
            break;

        case ETATelemetryMetric::TireLateralForceN:
            OutValue =
                Sample.TireLateralForceN[WheelIndex];
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

            AbsoluteMaximumValue =
                FMath::Max(
                    AbsoluteMaximumValue,
                    FMath::Abs(Value));

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
        case ETATelemetryMetric::CoolingEfficiency01:
            return TEXT("CoolingEfficiency01");
        case ETATelemetryMetric::CoolantTemperatureC:
            return TEXT("CoolantTemperatureC");
        case ETATelemetryMetric::SteeringRackDisplacementM:
            return TEXT("SteeringRackDisplacementM");
        case ETATelemetryMetric::WheelVerticalLoadN:
            return TEXT("WheelVerticalLoadN");
        case ETATelemetryMetric::SuspensionTravelM:
            return TEXT("SuspensionTravelM");
        case ETATelemetryMetric::WheelCamberRad:
            return TEXT("WheelCamberRad");
        case ETATelemetryMetric::WheelToeRad:
            return TEXT("WheelToeRad");
        case ETATelemetryMetric::TireRadialDeflectionM:
            return TEXT("TireRadialDeflectionM");
        case ETATelemetryMetric::WheelSlipRatio:
            return TEXT("WheelSlipRatio");
        case ETATelemetryMetric::TireLongitudinalForceN:
            return TEXT("TireLongitudinalForceN");
        case ETATelemetryMetric::TireLateralForceN:
            return TEXT("TireLateralForceN");
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

    FString EscapeJsonString(
        const FString& Value)
    {
        FString Result;
        Result.Reserve(Value.Len() + 8);

        for (const TCHAR Character : Value)
        {
            switch (Character)
            {
            case TEXT('\\'):
                Result += TEXT("\\\\");
                break;
            case TEXT('"'):
                Result += TEXT("\\"");
                break;
            case TEXT('\n'):
                Result += TEXT("\\n");
                break;
            case TEXT('\r'):
                Result += TEXT("\\r");
                break;
            case TEXT('\t'):
                Result += TEXT("\\t");
                break;
            default:
                Result.AppendChar(Character);
                break;
            }
        }

        return Result;
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

    OutResult.ScenarioId =
        Config.ScenarioId;

    OutResult.ExpectedPhysicsConfigHash =
        Config.ExpectedPhysicsConfigHash;

    if (!ValidateConfig(Config))
    {
        OutResult.Failures.Add(
            TEXT("Invalid regression configuration."));

        return false;
    }

    OutResult.SampleCount =
        Buffer.Num();

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

    OutResult.ObservedPhysicsConfigHash =
        ObservedHash;

    OutResult.bPhysicsConfigHashConsistent =
        bHashConsistent;

    OutResult.bPhysicsConfigHashMatched =
        bHashConsistent
        && (Config.ExpectedPhysicsConfigHash == 0
            || ObservedHash
                == Config.ExpectedPhysicsConfigHash);

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

        EnvelopeResult.Metric =
            Envelope.Metric;

        EnvelopeResult.Statistic =
            Envelope.Statistic;

        EnvelopeResult.WheelIndex =
            Envelope.WheelIndex;

        EnvelopeResult.StartFraction01 =
            Envelope.StartFraction01;

        EnvelopeResult.EndFraction01 =
            Envelope.EndFraction01;

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


FString TATelemetryRegression::ExportCsv(
    const FTATelemetryRegressionResult& Result)
{
    FString Csv;

    Csv += TEXT(
        "scenario_id,expected_physics_config_hash,observed_physics_config_hash,"
        "hash_consistent,hash_matched,sample_count,"
        "envelope_index,metric,statistic,wheel_index,"
        "window_start,window_end,observed_value,"
        "minimum_allowed,maximum_allowed,pass\n");

    const FString Scenario =
        Result.ScenarioId.ToString()
            .Replace(TEXT("\""), TEXT("\"\""));

    for (const FTATelemetryEnvelopeResult& Envelope :
         Result.EnvelopeResults)
    {
        Csv.Appendf(
            TEXT(
                "\"%s\",%u,%u,%d,%d,%d,"
                "%d,%s,%s,%d,"
                "%.9g,%.9g,%.9g,"
                "%.9g,%.9g,%d\n"),
            *Scenario,
            Result.ExpectedPhysicsConfigHash,
            Result.ObservedPhysicsConfigHash,
            Result.bPhysicsConfigHashConsistent ? 1 : 0,
            Result.bPhysicsConfigHashMatched ? 1 : 0,
            Result.SampleCount,
            Envelope.EnvelopeIndex,
            *MetricToString(Envelope.Metric),
            *StatisticToString(Envelope.Statistic),
            Envelope.WheelIndex,
            Envelope.StartFraction01,
            Envelope.EndFraction01,
            Envelope.ObservedValue,
            Envelope.MinimumAllowed,
            Envelope.MaximumAllowed,
            Envelope.bPassed ? 1 : 0);
    }

    return Csv;
}

FString TATelemetryRegression::ExportJsonLines(
    const FTATelemetryRegressionResult& Result)
{
    FString JsonLines;

    const FString Scenario =
        EscapeJsonString(
            Result.ScenarioId.ToString());

    for (const FTATelemetryEnvelopeResult& Envelope :
         Result.EnvelopeResults)
    {
        JsonLines.Appendf(
            TEXT(
                "{\"scenario_id\":\"%s\","
                "\"expected_physics_config_hash\":%u,"
                "\"observed_physics_config_hash\":%u,"
                "\"hash_consistent\":%s,"
                "\"hash_matched\":%s,"
                "\"sample_count\":%d,"
                "\"envelope_index\":%d,"
                "\"metric\":\"%s\","
                "\"statistic\":\"%s\","
                "\"wheel_index\":%d,"
                "\"window_start\":%.9g,"
                "\"window_end\":%.9g,"
                "\"observed_value\":%.9g,"
                "\"minimum_allowed\":%.9g,"
                "\"maximum_allowed\":%.9g,"
                "\"pass\":%s}\n"),
            *Scenario,
            Result.ExpectedPhysicsConfigHash,
            Result.ObservedPhysicsConfigHash,
            Result.bPhysicsConfigHashConsistent
                ? TEXT("true")
                : TEXT("false"),
            Result.bPhysicsConfigHashMatched
                ? TEXT("true")
                : TEXT("false"),
            Result.SampleCount,
            Envelope.EnvelopeIndex,
            *EscapeJsonString(
                MetricToString(
                    Envelope.Metric)),
            *EscapeJsonString(
                StatisticToString(
                    Envelope.Statistic)),
            Envelope.WheelIndex,
            Envelope.StartFraction01,
            Envelope.EndFraction01,
            Envelope.ObservedValue,
            Envelope.MinimumAllowed,
            Envelope.MaximumAllowed,
            Envelope.bPassed
                ? TEXT("true")
                : TEXT("false"));
    }

    return JsonLines;
}


bool TATelemetryRegression::ValidateProfileConfig(
    const FTATelemetryScenarioProfileConfig& Config)
{
    if (Config.ScenarioId.IsNone() ||
        Config.BaselineVersion <= 0 ||
        Config.MinimumRequiredSamples <= 0 ||
        Config.Metrics.Num() <= 0)
    {
        return false;
    }

    if (Config.bTrustedBaseline &&
        Config.ExpectedPhysicsConfigHash == 0)
    {
        return false;
    }

    const auto IsValidRange =
        [](const FTARegressionRange& Range)
        {
            return
                FMath::IsFinite(
                    Range.MinInclusive)
                && FMath::IsFinite(
                    Range.MaxInclusive)
                && Range.MinInclusive
                    <= Range.MaxInclusive;
        };

    for (const FTATelemetryMetricProfile& Metric :
         Config.Metrics)
    {
        if (!FMath::IsFinite(
                Metric.StartFraction01) ||
            !FMath::IsFinite(
                Metric.EndFraction01) ||
            Metric.StartFraction01 < 0.0 ||
            Metric.EndFraction01 > 1.0 ||
            Metric.EndFraction01
                <= Metric.StartFraction01 ||
            !FMath::IsFinite(
                Metric.SteadyStateFraction01) ||
            Metric.SteadyStateFraction01 <= 0.0 ||
            Metric.SteadyStateFraction01 > 1.0 ||
            !IsValidRange(
                Metric.MinimumEnvelope) ||
            !IsValidRange(
                Metric.MaximumEnvelope) ||
            !IsValidRange(
                Metric.SteadyStateEnvelope))
        {
            return false;
        }

        if (IsWheelMetric(Metric.Metric) &&
            (Metric.WheelIndex < 0 ||
             Metric.WheelIndex >=
                TAPrototypeTelemetryWheelCount))
        {
            return false;
        }
    }

    return true;
}

bool TATelemetryRegression::EvaluateProfile(
    const FTATelemetryRingBuffer& Buffer,
    const FTATelemetryScenarioProfileConfig& Config,
    FTATelemetryScenarioProfileResult& OutResult)
{
    OutResult =
        FTATelemetryScenarioProfileResult{};

    OutResult.ScenarioId =
        Config.ScenarioId;

    OutResult.BaselineVersion =
        Config.BaselineVersion;

    OutResult.bTrustedBaseline =
        Config.bTrustedBaseline;

    OutResult.ExpectedPhysicsConfigHash =
        Config.ExpectedPhysicsConfigHash;

    OutResult.SampleCount =
        Buffer.Num();

    if (!ValidateProfileConfig(Config))
    {
        OutResult.Failures.Add(
            TEXT(
                "Invalid telemetry scenario profile configuration."));

        return false;
    }

    uint32 ObservedHash = 0;
    bool bHashInitialized = false;
    bool bHashConsistent = true;

    for (int32 SampleIndex = 0;
         SampleIndex < Buffer.Num();
         ++SampleIndex)
    {
        const FTAVehicleTelemetrySample* Sample =
            Buffer.GetChronological(
                SampleIndex);

        if (!Sample)
        {
            OutResult.Failures.Add(
                TEXT(
                    "Telemetry buffer returned an invalid chronological sample."));

            return false;
        }

        if (!bHashInitialized)
        {
            ObservedHash =
                Sample->PhysicsConfigHash;

            bHashInitialized =
                true;
        }
        else if (Sample->PhysicsConfigHash
            != ObservedHash)
        {
            bHashConsistent =
                false;
        }
    }

    OutResult.ObservedPhysicsConfigHash =
        ObservedHash;

    OutResult.bPhysicsConfigHashConsistent =
        bHashConsistent;

    OutResult.bPhysicsConfigHashMatched =
        bHashConsistent
        && (Config.ExpectedPhysicsConfigHash == 0
            || ObservedHash
                == Config.ExpectedPhysicsConfigHash);

    if (!bHashConsistent)
    {
        OutResult.Failures.Add(
            TEXT(
                "Telemetry trace contains more than one physics config hash."));
    }
    else if (Config.ExpectedPhysicsConfigHash != 0 &&
             ObservedHash
                != Config.ExpectedPhysicsConfigHash)
    {
        OutResult.Failures.Add(
            FString::Printf(
                TEXT(
                    "Physics config hash mismatch: expected %u, observed %u."),
                Config.ExpectedPhysicsConfigHash,
                ObservedHash));
    }

    if (Buffer.Num() <
        Config.MinimumRequiredSamples)
    {
        OutResult.Failures.Add(
            FString::Printf(
                TEXT(
                    "Scenario %s requires at least %d samples; got %d."),
                *Config.ScenarioId.ToString(),
                Config.MinimumRequiredSamples,
                Buffer.Num()));

        return true;
    }

    OutResult.MetricResults.Reserve(
        Config.Metrics.Num());

    for (const FTATelemetryMetricProfile& Metric :
         Config.Metrics)
    {
        FTATelemetryMetricEnvelope Window;
        Window.Metric =
            Metric.Metric;

        Window.WheelIndex =
            Metric.WheelIndex;

        Window.StartFraction01 =
            Metric.StartFraction01;

        Window.EndFraction01 =
            Metric.EndFraction01;

        int32 StartIndex = 0;
        int32 EndExclusive = 0;

        CalculateWindowIndices(
            Buffer.Num(),
            Window,
            StartIndex,
            EndExclusive);

        TArray<double> Samples;
        Samples.Reserve(
            EndExclusive - StartIndex);

        bool bMetricReadSucceeded =
            true;

        for (int32 SampleIndex = StartIndex;
             SampleIndex < EndExclusive;
             ++SampleIndex)
        {
            const FTAVehicleTelemetrySample* Sample =
                Buffer.GetChronological(
                    SampleIndex);

            double Value = 0.0;

            if (!Sample ||
                !ReadMetricValue(
                    *Sample,
                    Window,
                    Value))
            {
                bMetricReadSucceeded =
                    false;

                break;
            }

            Samples.Add(
                Value);
        }

        FTATelemetryMetricProfileResult MetricResult;

        MetricResult.Metric =
            Metric.Metric;

        MetricResult.WheelIndex =
            Metric.WheelIndex;

        MetricResult.StartFraction01 =
            Metric.StartFraction01;

        MetricResult.EndFraction01 =
            Metric.EndFraction01;

        MetricResult.MinimumEnvelope =
            Metric.MinimumEnvelope;

        MetricResult.MaximumEnvelope =
            Metric.MaximumEnvelope;

        MetricResult.SteadyStateEnvelope =
            Metric.SteadyStateEnvelope;

        if (bMetricReadSucceeded)
        {
            MetricResult.Evaluation =
                TARegressionEnvelope::Evaluate(
                    Samples,
                    Metric.SteadyStateFraction01,
                    Metric.MinimumEnvelope,
                    Metric.MaximumEnvelope,
                    Metric.SteadyStateEnvelope);
        }

        if (!bMetricReadSucceeded ||
            !MetricResult.Evaluation.bPassed)
        {
            OutResult.Failures.Add(
                FString::Printf(
                    TEXT(
                        "%s wheel=%d failed: observed min=%.9g max=%.9g steady=%.9g."),
                    *MetricToString(
                        Metric.Metric),
                    Metric.WheelIndex,
                    MetricResult.Evaluation.Summary.MinValue,
                    MetricResult.Evaluation.Summary.MaxValue,
                    MetricResult.Evaluation.Summary.SteadyStateMean));
        }

        OutResult.MetricResults.Add(
            MoveTemp(MetricResult));
    }

    bool bAllMetricsPassed =
        OutResult.MetricResults.Num()
        == Config.Metrics.Num();

    for (const FTATelemetryMetricProfileResult& Metric :
         OutResult.MetricResults)
    {
        bAllMetricsPassed &=
            Metric.Evaluation.bPassed;
    }

    OutResult.bPassed =
        OutResult.bPhysicsConfigHashMatched
        && bAllMetricsPassed;

    return true;
}

FString TATelemetryRegression::ExportProfileCsv(
    const FTATelemetryScenarioProfileResult& Result)
{
    FString Csv;

    Csv += TEXT(
        "scenario_id,baseline_version,trusted_baseline,"
        "expected_physics_config_hash,observed_physics_config_hash,"
        "hash_consistent,hash_matched,sample_count,"
        "metric,wheel_index,window_start,window_end,"
        "observed_min,observed_max,observed_mean,observed_steady_state,"
        "expected_min_low,expected_min_high,"
        "expected_max_low,expected_max_high,"
        "expected_steady_low,expected_steady_high,pass\n");

    const FString Scenario =
        Result.ScenarioId.ToString()
            .Replace(
                TEXT("\""),
                TEXT("\"\""));

    for (const FTATelemetryMetricProfileResult& Metric :
         Result.MetricResults)
    {
        Csv.Appendf(
            TEXT(
                "\"%s\",%d,%d,%u,%u,%d,%d,%d,"
                "%s,%d,%.9g,%.9g,"
                "%.9g,%.9g,%.9g,%.9g,"
                "%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%d\n"),
            *Scenario,
            Result.BaselineVersion,
            Result.bTrustedBaseline ? 1 : 0,
            Result.ExpectedPhysicsConfigHash,
            Result.ObservedPhysicsConfigHash,
            Result.bPhysicsConfigHashConsistent ? 1 : 0,
            Result.bPhysicsConfigHashMatched ? 1 : 0,
            Result.SampleCount,
            *MetricToString(
                Metric.Metric),
            Metric.WheelIndex,
            Metric.StartFraction01,
            Metric.EndFraction01,
            Metric.Evaluation.Summary.MinValue,
            Metric.Evaluation.Summary.MaxValue,
            Metric.Evaluation.Summary.MeanValue,
            Metric.Evaluation.Summary.SteadyStateMean,
            Metric.MinimumEnvelope.MinInclusive,
            Metric.MinimumEnvelope.MaxInclusive,
            Metric.MaximumEnvelope.MinInclusive,
            Metric.MaximumEnvelope.MaxInclusive,
            Metric.SteadyStateEnvelope.MinInclusive,
            Metric.SteadyStateEnvelope.MaxInclusive,
            Metric.Evaluation.bPassed ? 1 : 0);
    }

    return Csv;
}

FString TATelemetryRegression::ExportProfileJsonLines(
    const FTATelemetryScenarioProfileResult& Result)
{
    FString JsonLines;

    const FString Scenario =
        EscapeJsonString(
            Result.ScenarioId.ToString());

    for (const FTATelemetryMetricProfileResult& Metric :
         Result.MetricResults)
    {
        JsonLines.Appendf(
            TEXT(
                "{\"scenario_id\":\"%s\","
                "\"baseline_version\":%d,"
                "\"trusted_baseline\":%s,"
                "\"expected_physics_config_hash\":%u,"
                "\"observed_physics_config_hash\":%u,"
                "\"hash_consistent\":%s,"
                "\"hash_matched\":%s,"
                "\"sample_count\":%d,"
                "\"metric\":\"%s\","
                "\"wheel_index\":%d,"
                "\"window_start\":%.9g,"
                "\"window_end\":%.9g,"
                "\"observed_min\":%.9g,"
                "\"observed_max\":%.9g,"
                "\"observed_mean\":%.9g,"
                "\"observed_steady_state\":%.9g,"
                "\"expected_min\":[%.9g,%.9g],"
                "\"expected_max\":[%.9g,%.9g],"
                "\"expected_steady_state\":[%.9g,%.9g],"
                "\"pass\":%s}\n"),
            *Scenario,
            Result.BaselineVersion,
            Result.bTrustedBaseline
                ? TEXT("true")
                : TEXT("false"),
            Result.ExpectedPhysicsConfigHash,
            Result.ObservedPhysicsConfigHash,
            Result.bPhysicsConfigHashConsistent
                ? TEXT("true")
                : TEXT("false"),
            Result.bPhysicsConfigHashMatched
                ? TEXT("true")
                : TEXT("false"),
            Result.SampleCount,
            *EscapeJsonString(
                MetricToString(
                    Metric.Metric)),
            Metric.WheelIndex,
            Metric.StartFraction01,
            Metric.EndFraction01,
            Metric.Evaluation.Summary.MinValue,
            Metric.Evaluation.Summary.MaxValue,
            Metric.Evaluation.Summary.MeanValue,
            Metric.Evaluation.Summary.SteadyStateMean,
            Metric.MinimumEnvelope.MinInclusive,
            Metric.MinimumEnvelope.MaxInclusive,
            Metric.MaximumEnvelope.MinInclusive,
            Metric.MaximumEnvelope.MaxInclusive,
            Metric.SteadyStateEnvelope.MinInclusive,
            Metric.SteadyStateEnvelope.MaxInclusive,
            Metric.Evaluation.bPassed
                ? TEXT("true")
                : TEXT("false"));
    }

    return JsonLines;
}
