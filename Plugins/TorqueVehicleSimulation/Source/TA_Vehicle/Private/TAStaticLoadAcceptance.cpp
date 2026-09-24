#include "TAStaticLoadAcceptance.h"

#include <cmath>

namespace
{
    bool IsFiniteNonNegative(const double Value)
    {
        return std::isfinite(Value) && Value >= 0.0;
    }

    bool IsFiniteComparison(const FTAStaticLoadComparison& C)
    {
        return std::isfinite(C.TotalErrorFraction)
            && std::isfinite(C.FrontAxleErrorFraction)
            && std::isfinite(C.RearAxleErrorFraction)
            && std::isfinite(C.LeftSideErrorFraction)
            && std::isfinite(C.RightSideErrorFraction)
            && std::isfinite(C.MaxAbsCornerErrorFractionOfTotal);
    }
}

bool TAStaticLoadAcceptance::Evaluate(
    const FTAStaticLoadAcceptanceEnvelope& Envelope,
    const FTAStaticLoadComparison& Comparison,
    FTAStaticLoadAcceptanceResult& OutResult)
{
    OutResult = FTAStaticLoadAcceptanceResult{};
    if (!IsFiniteNonNegative(Envelope.MaxAbsTotalErrorFraction)
        || !IsFiniteNonNegative(Envelope.MaxAbsAxleErrorFraction)
        || !IsFiniteNonNegative(Envelope.MaxAbsSideErrorFraction)
        || !IsFiniteNonNegative(Envelope.MaxAbsCornerErrorFractionOfTotal)
        || !IsFiniteComparison(Comparison))
    {
        return false;
    }

    OutResult.bTotalPass = FMath::Abs(Comparison.TotalErrorFraction) <= Envelope.MaxAbsTotalErrorFraction;
    OutResult.bFrontAxlePass = FMath::Abs(Comparison.FrontAxleErrorFraction) <= Envelope.MaxAbsAxleErrorFraction;
    OutResult.bRearAxlePass = FMath::Abs(Comparison.RearAxleErrorFraction) <= Envelope.MaxAbsAxleErrorFraction;
    OutResult.bLeftSidePass = FMath::Abs(Comparison.LeftSideErrorFraction) <= Envelope.MaxAbsSideErrorFraction;
    OutResult.bRightSidePass = FMath::Abs(Comparison.RightSideErrorFraction) <= Envelope.MaxAbsSideErrorFraction;
    OutResult.bCornerPass = Comparison.MaxAbsCornerErrorFractionOfTotal <= Envelope.MaxAbsCornerErrorFractionOfTotal;
    OutResult.bPass = OutResult.bTotalPass
        && OutResult.bFrontAxlePass
        && OutResult.bRearAxlePass
        && OutResult.bLeftSidePass
        && OutResult.bRightSidePass
        && OutResult.bCornerPass;
    return true;
}
