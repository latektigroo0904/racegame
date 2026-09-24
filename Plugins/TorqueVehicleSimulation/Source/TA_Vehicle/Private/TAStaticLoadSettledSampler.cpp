#include "TAStaticLoadSettledSampler.h"

#include <cmath>

namespace
{
    bool IsFiniteNonNegative(const double Value)
    {
        return std::isfinite(Value) && Value >= 0.0;
    }

    bool IsValidConfig(const FTAStaticLoadSettledConfig& Config)
    {
        return IsFiniteNonNegative(Config.MaxLinearSpeedMps)
            && IsFiniteNonNegative(Config.MaxAngularSpeedRadPerSec)
            && IsFiniteNonNegative(Config.MaxCornerLoadDeltaFractionOfExpectedTotal)
            && Config.MinimumQualifiedSamples > 0;
    }

    bool IsValidSample(const FTAStaticLoadSample& Sample)
    {
        return IsFiniteNonNegative(Sample.ChassisLinearSpeedMps)
            && IsFiniteNonNegative(Sample.ChassisAngularSpeedRadPerSec)
            && IsFiniteNonNegative(Sample.FrontLeftLoadN)
            && IsFiniteNonNegative(Sample.FrontRightLoadN)
            && IsFiniteNonNegative(Sample.RearLeftLoadN)
            && IsFiniteNonNegative(Sample.RearRightLoadN);
    }

    double MaxCornerDeltaN(const FTAStaticLoadSample& A, const FTAStaticLoadSample& B)
    {
        return FMath::Max(
            FMath::Max(FMath::Abs(A.FrontLeftLoadN - B.FrontLeftLoadN), FMath::Abs(A.FrontRightLoadN - B.FrontRightLoadN)),
            FMath::Max(FMath::Abs(A.RearLeftLoadN - B.RearLeftLoadN), FMath::Abs(A.RearRightLoadN - B.RearRightLoadN)));
    }
}

void TAStaticLoadSettledSampler::Reset(FTAStaticLoadSettledState& State)
{
    State = FTAStaticLoadSettledState{};
}

bool TAStaticLoadSettledSampler::PushSample(
    const FTAStaticLoadSettledConfig& Config,
    const double ExpectedTotalLoadN,
    const FTAStaticLoadSample& Sample,
    FTAStaticLoadSettledState& InOutState)
{
    if (!IsValidConfig(Config) || !std::isfinite(ExpectedTotalLoadN) || ExpectedTotalLoadN <= 0.0 || !IsValidSample(Sample))
    {
        Reset(InOutState);
        return false;
    }

    if (Sample.ChassisLinearSpeedMps > Config.MaxLinearSpeedMps
        || Sample.ChassisAngularSpeedRadPerSec > Config.MaxAngularSpeedRadPerSec)
    {
        Reset(InOutState);
        return false;
    }

    if (InOutState.bHasPreviousQualifiedSample)
    {
        const double MaxAllowedDeltaN = Config.MaxCornerLoadDeltaFractionOfExpectedTotal * ExpectedTotalLoadN;
        if (MaxCornerDeltaN(Sample, InOutState.PreviousQualifiedSample) > MaxAllowedDeltaN)
        {
            Reset(InOutState);
            return false;
        }
    }

    InOutState.bHasPreviousQualifiedSample = true;
    InOutState.PreviousQualifiedSample = Sample;
    ++InOutState.QualifiedSampleCount;
    InOutState.SumFrontLeftLoadN += Sample.FrontLeftLoadN;
    InOutState.SumFrontRightLoadN += Sample.FrontRightLoadN;
    InOutState.SumRearLeftLoadN += Sample.RearLeftLoadN;
    InOutState.SumRearRightLoadN += Sample.RearRightLoadN;
    return true;
}

bool TAStaticLoadSettledSampler::TryGetQualifiedMean(
    const FTAStaticLoadSettledConfig& Config,
    const FTAStaticLoadSettledState& State,
    FTAStaticLoadMeasuredMean& OutMean)
{
    OutMean = FTAStaticLoadMeasuredMean{};
    if (!IsValidConfig(Config) || State.QualifiedSampleCount < Config.MinimumQualifiedSamples || State.QualifiedSampleCount <= 0)
    {
        return false;
    }

    const double Divisor = static_cast<double>(State.QualifiedSampleCount);
    OutMean.FrontLeftLoadN = State.SumFrontLeftLoadN / Divisor;
    OutMean.FrontRightLoadN = State.SumFrontRightLoadN / Divisor;
    OutMean.RearLeftLoadN = State.SumRearLeftLoadN / Divisor;
    OutMean.RearRightLoadN = State.SumRearRightLoadN / Divisor;
    return std::isfinite(OutMean.FrontLeftLoadN)
        && std::isfinite(OutMean.FrontRightLoadN)
        && std::isfinite(OutMean.RearLeftLoadN)
        && std::isfinite(OutMean.RearRightLoadN);
}
