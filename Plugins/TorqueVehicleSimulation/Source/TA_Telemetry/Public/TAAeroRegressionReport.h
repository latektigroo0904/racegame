#pragma once

#include "CoreMinimal.h"
#include "TAAeroRegressionMetrics.h"
#include "TARegressionEnvelope.h"
#include "TATelemetry.h"

struct TA_TELEMETRY_API FTAAeroRegressionReport
{
    int32 SampleCount = 0;
    FTARegressionMetricSummary RelativeAirSpeedMps;
    FTARegressionMetricSummary DynamicPressurePa;
    FTARegressionMetricSummary ForceMagnitudeN;
    FTARegressionMetricSummary DragAxisForceN;
    FTARegressionMetricSummary VerticalForceN;
    FTARegressionMetricSummary PitchTorqueNm;
};

namespace TAAeroRegressionReport
{
    /**
     * Builds scalar summaries from the exact applied-aero compact telemetry.
     * ForwardWorld/UpWorld define the scenario frame. No aero physics is
     * recomputed here; extraction delegates to TAAeroRegressionMetrics.
     */
    TA_TELEMETRY_API bool Build(
        const FTACompactTelemetryRingBuffer& Buffer,
        const FVector3d& ForwardWorld,
        const FVector3d& UpWorld,
        double SteadyStateFraction01,
        FTAAeroRegressionReport& OutReport);

    /** One JSON object, suitable for CI artifact capture. */
    TA_TELEMETRY_API FString ExportJsonLine(
        const FTAAeroRegressionReport& Report);

    /** Header + one data row. */
    TA_TELEMETRY_API FString ExportCsv(
        const FTAAeroRegressionReport& Report);
}
