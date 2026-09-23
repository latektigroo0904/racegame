#include "TAAeroRegressionReport.h"

namespace
{
    FString SummaryJson(const FTARegressionMetricSummary& S)
    {
        return FString::Printf(
            TEXT("{\"min\":%.17g,\"max\":%.17g,\"mean\":%.17g,\"steady_state_mean\":%.17g}"),
            S.MinValue, S.MaxValue, S.MeanValue, S.SteadyStateMean);
    }

    bool SummarizeChannel(
        const TArray<double>& Values,
        const double SteadyStateFraction01,
        FTARegressionMetricSummary& Out)
    {
        return TARegressionEnvelope::Summarize(
            TConstArrayView<double>(Values),
            SteadyStateFraction01,
            Out);
    }
}

bool TAAeroRegressionReport::Build(
    const FTACompactTelemetryRingBuffer& Buffer,
    const FVector3d& ForwardWorld,
    const FVector3d& UpWorld,
    const double SteadyStateFraction01,
    FTAAeroRegressionReport& OutReport)
{
    OutReport = FTAAeroRegressionReport{};

    if (Buffer.Num() <= 0 ||
        !FMath::IsFinite(SteadyStateFraction01) ||
        SteadyStateFraction01 <= 0.0 ||
        SteadyStateFraction01 > 1.0)
    {
        return false;
    }

    TArray<double> RelativeAirSpeed;
    TArray<double> DynamicPressure;
    TArray<double> ForceMagnitude;
    TArray<double> DragAxisForce;
    TArray<double> VerticalForce;
    TArray<double> PitchTorque;

    const int32 Count = Buffer.Num();
    RelativeAirSpeed.Reserve(Count);
    DynamicPressure.Reserve(Count);
    ForceMagnitude.Reserve(Count);
    DragAxisForce.Reserve(Count);
    VerticalForce.Reserve(Count);
    PitchTorque.Reserve(Count);

    for (int32 Index = 0; Index < Count; ++Index)
    {
        FTATelemetrySample Sample;
        if (!Buffer.GetOldest(Index, Sample))
        {
            return false;
        }

        FTAAeroRegressionMetrics Metrics;
        if (!TAAeroRegressionMetrics::Extract(
                Sample,
                ForwardWorld,
                UpWorld,
                Metrics))
        {
            return false;
        }

        RelativeAirSpeed.Add(Metrics.RelativeAirSpeedMps);
        DynamicPressure.Add(Metrics.DynamicPressurePa);
        ForceMagnitude.Add(Metrics.ForceMagnitudeN);
        DragAxisForce.Add(Metrics.DragAxisForceN);
        VerticalForce.Add(Metrics.VerticalForceN);
        PitchTorque.Add(Metrics.PitchTorqueNm);
    }

    if (!SummarizeChannel(RelativeAirSpeed, SteadyStateFraction01, OutReport.RelativeAirSpeedMps) ||
        !SummarizeChannel(DynamicPressure, SteadyStateFraction01, OutReport.DynamicPressurePa) ||
        !SummarizeChannel(ForceMagnitude, SteadyStateFraction01, OutReport.ForceMagnitudeN) ||
        !SummarizeChannel(DragAxisForce, SteadyStateFraction01, OutReport.DragAxisForceN) ||
        !SummarizeChannel(VerticalForce, SteadyStateFraction01, OutReport.VerticalForceN) ||
        !SummarizeChannel(PitchTorque, SteadyStateFraction01, OutReport.PitchTorqueNm))
    {
        OutReport = FTAAeroRegressionReport{};
        return false;
    }

    OutReport.SampleCount = Count;
    return true;
}

FString TAAeroRegressionReport::ExportJsonLine(
    const FTAAeroRegressionReport& Report)
{
    return FString::Printf(
        TEXT("{\"sample_count\":%d,\"relative_air_speed_mps\":%s,\"dynamic_pressure_pa\":%s,\"force_magnitude_n\":%s,\"drag_axis_force_n\":%s,\"vertical_force_n\":%s,\"pitch_torque_nm\":%s}\n"),
        Report.SampleCount,
        *SummaryJson(Report.RelativeAirSpeedMps),
        *SummaryJson(Report.DynamicPressurePa),
        *SummaryJson(Report.ForceMagnitudeN),
        *SummaryJson(Report.DragAxisForceN),
        *SummaryJson(Report.VerticalForceN),
        *SummaryJson(Report.PitchTorqueNm));
}

FString TAAeroRegressionReport::ExportCsv(
    const FTAAeroRegressionReport& Report)
{
    return FString::Printf(
        TEXT("sample_count,airspeed_min,airspeed_max,airspeed_mean,airspeed_steady,dynamic_pressure_min,dynamic_pressure_max,dynamic_pressure_mean,dynamic_pressure_steady,force_min,force_max,force_mean,force_steady,drag_min,drag_max,drag_mean,drag_steady,vertical_min,vertical_max,vertical_mean,vertical_steady,pitch_min,pitch_max,pitch_mean,pitch_steady\n")
        TEXT("%d,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g\n"),
        Report.SampleCount,
        Report.RelativeAirSpeedMps.MinValue, Report.RelativeAirSpeedMps.MaxValue, Report.RelativeAirSpeedMps.MeanValue, Report.RelativeAirSpeedMps.SteadyStateMean,
        Report.DynamicPressurePa.MinValue, Report.DynamicPressurePa.MaxValue, Report.DynamicPressurePa.MeanValue, Report.DynamicPressurePa.SteadyStateMean,
        Report.ForceMagnitudeN.MinValue, Report.ForceMagnitudeN.MaxValue, Report.ForceMagnitudeN.MeanValue, Report.ForceMagnitudeN.SteadyStateMean,
        Report.DragAxisForceN.MinValue, Report.DragAxisForceN.MaxValue, Report.DragAxisForceN.MeanValue, Report.DragAxisForceN.SteadyStateMean,
        Report.VerticalForceN.MinValue, Report.VerticalForceN.MaxValue, Report.VerticalForceN.MeanValue, Report.VerticalForceN.SteadyStateMean,
        Report.PitchTorqueNm.MinValue, Report.PitchTorqueNm.MaxValue, Report.PitchTorqueNm.MeanValue, Report.PitchTorqueNm.SteadyStateMean);
}
