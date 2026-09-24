#pragma once

#include "CoreMinimal.h"
#include "TAStaticLoadComparison.h"

struct TA_VEHICLE_API FTAStaticLoadAcceptanceEnvelope
{
    double MaxAbsTotalErrorFraction = 0.02;
    double MaxAbsAxleErrorFraction = 0.03;
    double MaxAbsSideErrorFraction = 0.03;
    double MaxAbsCornerErrorFractionOfTotal = 0.02;
};

struct TA_VEHICLE_API FTAStaticLoadAcceptanceResult
{
    bool bTotalPass = false;
    bool bFrontAxlePass = false;
    bool bRearAxlePass = false;
    bool bLeftSidePass = false;
    bool bRightSidePass = false;
    bool bCornerPass = false;
    bool bPass = false;
};

namespace TAStaticLoadAcceptance
{
    /**
     * Evaluates already-qualified static evidence. Threshold equality passes.
     * Defaults are provisional Proof-of-Physics tolerances, not calibration truth.
     */
    TA_VEHICLE_API bool Evaluate(
        const FTAStaticLoadAcceptanceEnvelope& Envelope,
        const FTAStaticLoadComparison& Comparison,
        FTAStaticLoadAcceptanceResult& OutResult);
}
