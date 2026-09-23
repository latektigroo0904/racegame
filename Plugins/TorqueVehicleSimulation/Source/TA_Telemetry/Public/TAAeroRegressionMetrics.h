#pragma once

#include "CoreMinimal.h"
#include "TATelemetry.h"

struct TA_TELEMETRY_API FTAAeroRegressionMetrics
{
    double RelativeAirSpeedMps = 0.0;
    double DynamicPressurePa = 0.0;
    double ForceMagnitudeN = 0.0;
    double DragAxisForceN = 0.0;
    double VerticalForceN = 0.0;
    double PitchTorqueNm = 0.0;
};

namespace TAAeroRegressionMetrics
{
    // Extracts stable scalar channels from the exact applied aero telemetry.
    // ForwardWorld and UpWorld define the scenario/report frame; callers should
    // use canonical proving-ground axes rather than reconstructing aero physics.
    TA_TELEMETRY_API bool Extract(
        const FTATelemetrySample& Sample,
        const FVector3d& ForwardWorld,
        const FVector3d& UpWorld,
        FTAAeroRegressionMetrics& OutMetrics);
}
