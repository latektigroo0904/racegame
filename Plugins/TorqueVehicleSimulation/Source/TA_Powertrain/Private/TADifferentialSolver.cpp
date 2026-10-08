#include "TADifferentialSolver.h"

namespace
{
    double ClampMagnitude(
        const double Value,
        const double MaxMagnitude)
    {
        return FMath::Clamp(
            Value,
            -FMath::Max(0.0, MaxMagnitude),
            FMath::Max(0.0, MaxMagnitude));
    }

    void ApplyCapacityAndBookkeeping(
        const double InputTorqueNm,
        const double Efficiency01,
        const double LeftCapacityNm,
        const double RightCapacityNm,
        double LeftRequestedNm,
        double RightRequestedNm,
        FTADifferentialOutput& OutOutput)
    {
        const double EfficientInputTorqueNm =
            InputTorqueNm
            * FMath::Clamp(
                Efficiency01,
                0.0,
                1.0);

        const double Sign =
            FMath::Sign(EfficientInputTorqueNm);

        const double LeftCapacity =
            FMath::Max(0.0, LeftCapacityNm);

        const double RightCapacity =
            FMath::Max(0.0, RightCapacityNm);

        // Never permit a drive-side allocation to reverse sign merely because
        // an internal bias request exceeded the available input torque.
        if (!FMath::IsNearlyZero(EfficientInputTorqueNm))
        {
            LeftRequestedNm =
                Sign * FMath::Max(
                    0.0,
                    Sign * LeftRequestedNm);

            RightRequestedNm =
                Sign * FMath::Max(
                    0.0,
                    Sign * RightRequestedNm);
        }

        OutOutput.LeftWheelTorqueNm =
            ClampMagnitude(
                LeftRequestedNm,
                LeftCapacity);

        OutOutput.RightWheelTorqueNm =
            ClampMagnitude(
                RightRequestedNm,
                RightCapacity);

        OutOutput.UntransmittedInputTorqueNm =
            EfficientInputTorqueNm
            - OutOutput.LeftWheelTorqueNm
            - OutOutput.RightWheelTorqueNm;
    }

    double CalculateClutchLockCapacityNm(
        const FTADifferentialConfig& Config,
        const FTADifferentialInput& Input)
    {
        const double RampGain =
            Input.bCoast
            ? Config.CoastRampGain
            : Config.DriveRampGain;

        return FMath::Clamp(
            Config.PreloadTorqueNm
            + FMath::Abs(Input.InputTorqueNm)
                * RampGain,
            0.0,
            Config.MaxLockTorqueNm);
    }
}

bool TADifferentialSolver::ValidateConfig(
    const FTADifferentialConfig& Config)
{
    return
        FMath::IsFinite(Config.MechanicalEfficiency01)
        && Config.MechanicalEfficiency01 >= 0.0
        && Config.MechanicalEfficiency01 <= 1.0
        && FMath::IsFinite(Config.PreloadTorqueNm)
        && Config.PreloadTorqueNm >= 0.0
        && FMath::IsFinite(Config.LockStiffnessNmsPerRad)
        && Config.LockStiffnessNmsPerRad >= 0.0
        && FMath::IsFinite(Config.DriveRampGain)
        && Config.DriveRampGain >= 0.0
        && FMath::IsFinite(Config.CoastRampGain)
        && Config.CoastRampGain >= 0.0
        && FMath::IsFinite(Config.MaxLockTorqueNm)
        && Config.MaxLockTorqueNm >= 0.0
        && FMath::IsFinite(Config.TorqueBiasRatio)
        && Config.TorqueBiasRatio >= 1.0
        && FMath::IsFinite(Config.ViscousCoefficientNmsPerRad)
        && Config.ViscousCoefficientNmsPerRad >= 0.0
        && FMath::IsFinite(Config.ViscousMaxTorqueNm)
        && Config.ViscousMaxTorqueNm >= 0.0
        && FMath::IsFinite(Config.ActiveMaxBiasTorqueNm)
        && Config.ActiveMaxBiasTorqueNm >= 0.0;
}

bool TADifferentialSolver::Solve(
    const FTADifferentialConfig& Config,
    const FTADifferentialInput& Input,
    FTADifferentialOutput& OutOutput)
{
    OutOutput =
        FTADifferentialOutput{};

    if (!ValidateConfig(Config)
        || !FMath::IsFinite(Input.InputTorqueNm)
        || !FMath::IsFinite(Input.LeftAngularSpeedRadPerSec)
        || !FMath::IsFinite(Input.RightAngularSpeedRadPerSec)
        || !FMath::IsFinite(Input.LeftReactionCapacityNm)
        || !FMath::IsFinite(Input.RightReactionCapacityNm)
        || Input.LeftReactionCapacityNm < 0.0
        || Input.RightReactionCapacityNm < 0.0
        || !FMath::IsFinite(Input.ActiveBiasRequestNm))
    {
        return false;
    }

    const double EfficientInputTorqueNm =
        Input.InputTorqueNm
        * Config.MechanicalEfficiency01;

    const double HalfTorqueNm =
        0.5 * EfficientInputTorqueNm;

    OutOutput.RelativeAngularSpeedRadPerSec =
        Input.LeftAngularSpeedRadPerSec
        - Input.RightAngularSpeedRadPerSec;

    if (Config.Type == ETADifferentialType::Open)
    {
        const double CommonMagnitudeNm =
            FMath::Min3(
                FMath::Abs(HalfTorqueNm),
                Input.LeftReactionCapacityNm,
                Input.RightReactionCapacityNm);

        const double CommonTorqueNm =
            FMath::Sign(EfficientInputTorqueNm)
            * CommonMagnitudeNm;

        ApplyCapacityAndBookkeeping(
            Input.InputTorqueNm,
            Config.MechanicalEfficiency01,
            Input.LeftReactionCapacityNm,
            Input.RightReactionCapacityNm,
            CommonTorqueNm,
            CommonTorqueNm,
            OutOutput);

        return true;
    }

    double BiasTorqueNm = 0.0;

    switch (Config.Type)
    {
    case ETADifferentialType::Spool:
    {
        const double RawLockTorqueNm =
            Config.LockStiffnessNmsPerRad
            * OutOutput.RelativeAngularSpeedRadPerSec;

        BiasTorqueNm =
            FMath::Clamp(
                RawLockTorqueNm,
                -Config.MaxLockTorqueNm,
                Config.MaxLockTorqueNm);
        break;
    }

    case ETADifferentialType::ClutchLsd:
    {
        const double CapacityNm =
            CalculateClutchLockCapacityNm(
                Config,
                Input);

        const double RawLockTorqueNm =
            Config.LockStiffnessNmsPerRad
            * OutOutput.RelativeAngularSpeedRadPerSec;

        BiasTorqueNm =
            FMath::Clamp(
                RawLockTorqueNm,
                -CapacityNm,
                CapacityNm);
        break;
    }

    case ETADifferentialType::Helical:
    {
        const double Sign =
            FMath::Sign(EfficientInputTorqueNm);

        const double InputMagnitudeNm =
            FMath::Abs(EfficientInputTorqueNm);

        const double LowSideCapacityNm =
            FMath::Min(
                Input.LeftReactionCapacityNm,
                Input.RightReactionCapacityNm);

        const double HighSideCapacityNm =
            FMath::Max(
                Input.LeftReactionCapacityNm,
                Input.RightReactionCapacityNm);

        const double LowSideTorqueNm =
            FMath::Min(
                InputMagnitudeNm
                    / (1.0 + Config.TorqueBiasRatio),
                LowSideCapacityNm);

        const double HighSideTorqueNm =
            FMath::Min3(
                InputMagnitudeNm - LowSideTorqueNm,
                Config.TorqueBiasRatio * LowSideTorqueNm,
                HighSideCapacityNm);

        const bool bRightHasHigherCapacity =
            Input.RightReactionCapacityNm
            >= Input.LeftReactionCapacityNm;

        const double LeftRequestedNm =
            Sign
            * (bRightHasHigherCapacity
                ? LowSideTorqueNm
                : HighSideTorqueNm);

        const double RightRequestedNm =
            Sign
            * (bRightHasHigherCapacity
                ? HighSideTorqueNm
                : LowSideTorqueNm);

        OutOutput.BiasTorqueNm =
            0.5
            * (RightRequestedNm - LeftRequestedNm);

        ApplyCapacityAndBookkeeping(
            Input.InputTorqueNm,
            Config.MechanicalEfficiency01,
            Input.LeftReactionCapacityNm,
            Input.RightReactionCapacityNm,
            LeftRequestedNm,
            RightRequestedNm,
            OutOutput);

        return true;
    }

    case ETADifferentialType::Viscous:
    {
        const double RawBiasTorqueNm =
            Config.ViscousCoefficientNmsPerRad
            * OutOutput.RelativeAngularSpeedRadPerSec;

        BiasTorqueNm =
            FMath::Clamp(
                RawBiasTorqueNm,
                -Config.ViscousMaxTorqueNm,
                Config.ViscousMaxTorqueNm);
        break;
    }

    case ETADifferentialType::Active:
        BiasTorqueNm =
            FMath::Clamp(
                Input.ActiveBiasRequestNm,
                -Config.ActiveMaxBiasTorqueNm,
                Config.ActiveMaxBiasTorqueNm);
        break;

    case ETADifferentialType::Open:
    default:
        break;
    }

    // Positive BiasTorqueNm moves drive torque from left to right.
    const double LeftRequestedNm =
        HalfTorqueNm - BiasTorqueNm;

    const double RightRequestedNm =
        HalfTorqueNm + BiasTorqueNm;

    OutOutput.BiasTorqueNm =
        BiasTorqueNm;

    ApplyCapacityAndBookkeeping(
        Input.InputTorqueNm,
        Config.MechanicalEfficiency01,
        Input.LeftReactionCapacityNm,
        Input.RightReactionCapacityNm,
        LeftRequestedNm,
        RightRequestedNm,
        OutOutput);

    if (Config.Type == ETADifferentialType::ClutchLsd
        || Config.Type == ETADifferentialType::Viscous
        || Config.Type == ETADifferentialType::Spool)
    {
        OutOutput.DissipatedPowerW =
            FMath::Abs(
                BiasTorqueNm
                * OutOutput.RelativeAngularSpeedRadPerSec);
    }

    return true;
}
