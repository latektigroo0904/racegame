#pragma once

#include "CoreMinimal.h"
#include "TAStaticLoadComparison.h"

struct TA_VEHICLE_API FTAStaticLoadSample
{
    double ChassisLinearSpeedMps = 0.0;
    double ChassisAngularSpeedRadPerSec = 0.0;
    double FrontLeftLoadN = 0.0;
    double FrontRightLoadN = 0.0;
    double RearLeftLoadN = 0.0;
    double RearRightLoadN = 0.0;
};

struct TA_VEHICLE_API FTAStaticLoadSettledConfig
{
    double MaxLinearSpeedMps = 0.02;
    double MaxAngularSpeedRadPerSec = 0.01;
    double MaxCornerLoadDeltaFractionOfExpectedTotal = 0.0025;
    int32 MinimumQualifiedSamples = 120;
};

struct TA_VEHICLE_API FTAStaticLoadSettledState
{
    int32 QualifiedSampleCount = 0;
    bool bHasPreviousQualifiedSample = false;
    FTAStaticLoadSample PreviousQualifiedSample;
    double SumFrontLeftLoadN = 0.0;
    double SumFrontRightLoadN = 0.0;
    double SumRearLeftLoadN = 0.0;
    double SumRearRightLoadN = 0.0;
};

namespace TAStaticLoadSettledSampler
{
    /** Reset all qualification history and accumulated evidence. */
    TA_VEHICLE_API void Reset(FTAStaticLoadSettledState& State);

    /**
     * Consumes one world sample. Any invalid sample, excess chassis motion, or
     * corner-load instability resets the consecutive qualification window.
     * Threshold equality qualifies. The first motion-qualified sample seeds the
     * stability reference and is included in the mean.
     */
    TA_VEHICLE_API bool PushSample(
        const FTAStaticLoadSettledConfig& Config,
        double ExpectedTotalLoadN,
        const FTAStaticLoadSample& Sample,
        FTAStaticLoadSettledState& InOutState);

    /** Returns a mean only after the configured consecutive sample count is met. */
    TA_VEHICLE_API bool TryGetQualifiedMean(
        const FTAStaticLoadSettledConfig& Config,
        const FTAStaticLoadSettledState& State,
        FTAStaticLoadMeasuredMean& OutMean);
}
