#pragma once

#include "CoreMinimal.h"
#include "TAStaticMassBalance.h"

struct TA_VEHICLE_API FTAStaticLoadMeasuredMean
{
    double FrontLeftLoadN = 0.0;
    double FrontRightLoadN = 0.0;
    double RearLeftLoadN = 0.0;
    double RearRightLoadN = 0.0;
};

struct TA_VEHICLE_API FTAStaticLoadComparison
{
    double ExpectedTotalN = 0.0;
    double MeasuredTotalN = 0.0;
    double TotalErrorN = 0.0;
    double TotalErrorFraction = 0.0;

    double ExpectedFrontAxleN = 0.0;
    double MeasuredFrontAxleN = 0.0;
    double FrontAxleErrorN = 0.0;
    double FrontAxleErrorFraction = 0.0;

    double ExpectedRearAxleN = 0.0;
    double MeasuredRearAxleN = 0.0;
    double RearAxleErrorN = 0.0;
    double RearAxleErrorFraction = 0.0;

    double ExpectedLeftSideN = 0.0;
    double MeasuredLeftSideN = 0.0;
    double LeftSideErrorN = 0.0;
    double LeftSideErrorFraction = 0.0;

    double ExpectedRightSideN = 0.0;
    double MeasuredRightSideN = 0.0;
    double RightSideErrorN = 0.0;
    double RightSideErrorFraction = 0.0;

    double MaxAbsCornerErrorN = 0.0;
    double MaxAbsCornerErrorFractionOfTotal = 0.0;
};

namespace TAStaticLoadComparison
{
    /**
     * Compares measured mean support loads against the analytical static oracle.
     * Signed errors are Measured - Expected. Fractions use the corresponding
     * expected aggregate as denominator; corner max fraction uses expected total.
     * This is diagnostic/regression evidence only and must never feed physics.
     */
    TA_VEHICLE_API bool Calculate(
        const FTAStaticMassBalanceOutput& Expected,
        const FTAStaticLoadMeasuredMean& Measured,
        FTAStaticLoadComparison& OutComparison);
}
