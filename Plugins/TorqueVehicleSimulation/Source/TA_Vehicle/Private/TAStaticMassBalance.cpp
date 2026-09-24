#include "TAStaticMassBalance.h"

namespace
{
    bool IsFinitePositive(const double Value)
    {
        return FMath::IsFinite(Value) && Value > 0.0;
    }
}

bool TAStaticMassBalance::Calculate(
    const FTAStaticMassBalanceInput& Input,
    FTAStaticMassBalanceOutput& OutOutput)
{
    OutOutput = FTAStaticMassBalanceOutput{};

    const bool bBaseValid =
        IsFinitePositive(Input.MassKg)
        && IsFinitePositive(Input.GravityMagnitudeMps2)
        && IsFinitePositive(Input.WheelbaseM)
        && IsFinitePositive(Input.FrontTrackM)
        && IsFinitePositive(Input.RearTrackM)
        && FMath::IsFinite(Input.ComFromRearAxleM)
        && FMath::IsFinite(Input.ComLateralM);

    if (!bBaseValid
        || Input.ComFromRearAxleM < 0.0
        || Input.ComFromRearAxleM > Input.WheelbaseM
        || FMath::Abs(Input.ComLateralM) >= 0.5 * Input.FrontTrackM
        || FMath::Abs(Input.ComLateralM) >= 0.5 * Input.RearTrackM)
    {
        return false;
    }

    const double TotalWeightN =
        Input.MassKg * Input.GravityMagnitudeMps2;

    const double FrontAxleLoadN =
        TotalWeightN
        * Input.ComFromRearAxleM
        / Input.WheelbaseM;

    const double RearAxleLoadN =
        TotalWeightN - FrontAxleLoadN;

    const double FrontRightFraction =
        0.5 + Input.ComLateralM / Input.FrontTrackM;

    const double RearRightFraction =
        0.5 + Input.ComLateralM / Input.RearTrackM;

    OutOutput.TotalWeightN = TotalWeightN;
    OutOutput.FrontAxleLoadN = FrontAxleLoadN;
    OutOutput.RearAxleLoadN = RearAxleLoadN;
    OutOutput.FrontRightLoadN = FrontAxleLoadN * FrontRightFraction;
    OutOutput.FrontLeftLoadN = FrontAxleLoadN - OutOutput.FrontRightLoadN;
    OutOutput.RearRightLoadN = RearAxleLoadN * RearRightFraction;
    OutOutput.RearLeftLoadN = RearAxleLoadN - OutOutput.RearRightLoadN;

    return
        OutOutput.FrontLeftLoadN >= 0.0
        && OutOutput.FrontRightLoadN >= 0.0
        && OutOutput.RearLeftLoadN >= 0.0
        && OutOutput.RearRightLoadN >= 0.0;
}
