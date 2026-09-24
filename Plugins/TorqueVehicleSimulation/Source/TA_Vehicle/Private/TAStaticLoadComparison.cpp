#include "TAStaticLoadComparison.h"

namespace
{
    bool IsFiniteNonNegative(const double Value)
    {
        return FMath::IsFinite(Value) && Value >= 0.0;
    }

    double RelativeError(const double ErrorN, const double ExpectedN)
    {
        return ErrorN / ExpectedN;
    }
}

bool TAStaticLoadComparison::Calculate(
    const FTAStaticMassBalanceOutput& Expected,
    const FTAStaticLoadMeasuredMean& Measured,
    FTAStaticLoadComparison& OutComparison)
{
    OutComparison = FTAStaticLoadComparison{};

    const bool bExpectedValid =
        FMath::IsFinite(Expected.TotalWeightN) && Expected.TotalWeightN > 0.0 &&
        FMath::IsFinite(Expected.FrontAxleLoadN) && Expected.FrontAxleLoadN > 0.0 &&
        FMath::IsFinite(Expected.RearAxleLoadN) && Expected.RearAxleLoadN > 0.0 &&
        IsFiniteNonNegative(Expected.FrontLeftLoadN) &&
        IsFiniteNonNegative(Expected.FrontRightLoadN) &&
        IsFiniteNonNegative(Expected.RearLeftLoadN) &&
        IsFiniteNonNegative(Expected.RearRightLoadN);

    const bool bMeasuredValid =
        IsFiniteNonNegative(Measured.FrontLeftLoadN) &&
        IsFiniteNonNegative(Measured.FrontRightLoadN) &&
        IsFiniteNonNegative(Measured.RearLeftLoadN) &&
        IsFiniteNonNegative(Measured.RearRightLoadN);

    if (!bExpectedValid || !bMeasuredValid)
    {
        return false;
    }

    const double ExpectedCornerTotal = Expected.FrontLeftLoadN + Expected.FrontRightLoadN
        + Expected.RearLeftLoadN + Expected.RearRightLoadN;
    const double ExpectedAxleTotal = Expected.FrontAxleLoadN + Expected.RearAxleLoadN;
    const double ConsistencyToleranceN = FMath::Max(1.0e-6, Expected.TotalWeightN * 1.0e-9);

    if (FMath::Abs(ExpectedCornerTotal - Expected.TotalWeightN) > ConsistencyToleranceN ||
        FMath::Abs(ExpectedAxleTotal - Expected.TotalWeightN) > ConsistencyToleranceN)
    {
        return false;
    }

    OutComparison.ExpectedTotalN = Expected.TotalWeightN;
    OutComparison.MeasuredTotalN = Measured.FrontLeftLoadN + Measured.FrontRightLoadN
        + Measured.RearLeftLoadN + Measured.RearRightLoadN;
    OutComparison.TotalErrorN = OutComparison.MeasuredTotalN - OutComparison.ExpectedTotalN;
    OutComparison.TotalErrorFraction = RelativeError(OutComparison.TotalErrorN, OutComparison.ExpectedTotalN);

    OutComparison.ExpectedFrontAxleN = Expected.FrontAxleLoadN;
    OutComparison.MeasuredFrontAxleN = Measured.FrontLeftLoadN + Measured.FrontRightLoadN;
    OutComparison.FrontAxleErrorN = OutComparison.MeasuredFrontAxleN - OutComparison.ExpectedFrontAxleN;
    OutComparison.FrontAxleErrorFraction = RelativeError(OutComparison.FrontAxleErrorN, OutComparison.ExpectedFrontAxleN);

    OutComparison.ExpectedRearAxleN = Expected.RearAxleLoadN;
    OutComparison.MeasuredRearAxleN = Measured.RearLeftLoadN + Measured.RearRightLoadN;
    OutComparison.RearAxleErrorN = OutComparison.MeasuredRearAxleN - OutComparison.ExpectedRearAxleN;
    OutComparison.RearAxleErrorFraction = RelativeError(OutComparison.RearAxleErrorN, OutComparison.ExpectedRearAxleN);

    OutComparison.ExpectedLeftSideN = Expected.FrontLeftLoadN + Expected.RearLeftLoadN;
    OutComparison.MeasuredLeftSideN = Measured.FrontLeftLoadN + Measured.RearLeftLoadN;
    OutComparison.LeftSideErrorN = OutComparison.MeasuredLeftSideN - OutComparison.ExpectedLeftSideN;
    OutComparison.LeftSideErrorFraction = RelativeError(OutComparison.LeftSideErrorN, OutComparison.ExpectedLeftSideN);

    OutComparison.ExpectedRightSideN = Expected.FrontRightLoadN + Expected.RearRightLoadN;
    OutComparison.MeasuredRightSideN = Measured.FrontRightLoadN + Measured.RearRightLoadN;
    OutComparison.RightSideErrorN = OutComparison.MeasuredRightSideN - OutComparison.ExpectedRightSideN;
    OutComparison.RightSideErrorFraction = RelativeError(OutComparison.RightSideErrorN, OutComparison.ExpectedRightSideN);

    const double CornerErrors[] = {
        Measured.FrontLeftLoadN - Expected.FrontLeftLoadN,
        Measured.FrontRightLoadN - Expected.FrontRightLoadN,
        Measured.RearLeftLoadN - Expected.RearLeftLoadN,
        Measured.RearRightLoadN - Expected.RearRightLoadN
    };

    for (const double ErrorN : CornerErrors)
    {
        OutComparison.MaxAbsCornerErrorN = FMath::Max(OutComparison.MaxAbsCornerErrorN, FMath::Abs(ErrorN));
    }
    OutComparison.MaxAbsCornerErrorFractionOfTotal =
        OutComparison.MaxAbsCornerErrorN / OutComparison.ExpectedTotalN;

    return true;
}
