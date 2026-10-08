#include "TADriverAssistControllers.h"

namespace
{
    double MoveToward(
        const double Current,
        const double Target,
        const double RatePerSec,
        const double DeltaTimeSeconds)
    {
        const double Difference =
            Target - Current;

        if (FMath::IsNearlyZero(Difference))
        {
            return Target;
        }

        const double Delta =
            FMath::Min(
                FMath::Abs(Difference),
                RatePerSec * DeltaTimeSeconds);

        return Current
            + FMath::Sign(Difference) * Delta;
    }
}

bool TADriverAssistControllers::ValidateTractionConfig(
    const FTATractionControlConfig& Config)
{
    return
        FMath::IsFinite(Config.MinimumVehicleSpeedMps)
        && Config.MinimumVehicleSpeedMps >= 0.0
        && FMath::IsFinite(Config.MinimumThrottle01)
        && Config.MinimumThrottle01 >= 0.0
        && Config.MinimumThrottle01 <= 1.0
        && FMath::IsFinite(Config.EnterSlipRatio)
        && Config.EnterSlipRatio > 0.0
        && FMath::IsFinite(Config.RecoverSlipRatio)
        && Config.RecoverSlipRatio >= 0.0
        && Config.RecoverSlipRatio < Config.EnterSlipRatio
        && FMath::IsFinite(Config.TorqueReductionRate01PerSec)
        && Config.TorqueReductionRate01PerSec > 0.0
        && FMath::IsFinite(Config.TorqueRecoveryRate01PerSec)
        && Config.TorqueRecoveryRate01PerSec > 0.0
        && FMath::IsFinite(Config.MinimumEngineTorqueAuthority01)
        && Config.MinimumEngineTorqueAuthority01 >= 0.0
        && Config.MinimumEngineTorqueAuthority01 <= 1.0;
}

void TADriverAssistControllers::InitializeTractionState(
    FTATractionControlState& OutState)
{
    OutState =
        FTATractionControlState{};
}

bool TADriverAssistControllers::StepTractionControl(
    const FTATractionControlConfig& Config,
    const FTATractionControlInput& Input,
    FTATractionControlState& InOutState,
    FTATractionControlOutput& OutOutput)
{
    OutOutput =
        FTATractionControlOutput{};

    if (!ValidateTractionConfig(Config)
        || !FMath::IsFinite(Input.Throttle01)
        || !FMath::IsFinite(Input.VehicleSpeedMps)
        || !FMath::IsFinite(Input.MaxDrivenSlipRatio)
        || !FMath::IsFinite(Input.DeltaTimeSeconds)
        || Input.DeltaTimeSeconds <= 0.0
        || !FMath::IsFinite(InOutState.EngineTorqueAuthority01))
    {
        return false;
    }

    InOutState.EngineTorqueAuthority01 =
        FMath::Clamp(
            InOutState.EngineTorqueAuthority01,
            Config.MinimumEngineTorqueAuthority01,
            1.0);

    const bool bControllerRelevant =
        Input.bDrivenContactValid
        && Input.VehicleSpeedMps
            >= Config.MinimumVehicleSpeedMps
        && FMath::Clamp(Input.Throttle01, 0.0, 1.0)
            >= Config.MinimumThrottle01;

    if (bControllerRelevant
        && Input.MaxDrivenSlipRatio
            >= Config.EnterSlipRatio)
    {
        InOutState.EngineTorqueAuthority01 =
            MoveToward(
                InOutState.EngineTorqueAuthority01,
                Config.MinimumEngineTorqueAuthority01,
                Config.TorqueReductionRate01PerSec,
                Input.DeltaTimeSeconds);

        OutOutput.bReducingTorque =
            true;
    }
    else if (!bControllerRelevant
        || Input.MaxDrivenSlipRatio
            <= Config.RecoverSlipRatio)
    {
        InOutState.EngineTorqueAuthority01 =
            MoveToward(
                InOutState.EngineTorqueAuthority01,
                1.0,
                Config.TorqueRecoveryRate01PerSec,
                Input.DeltaTimeSeconds);

        OutOutput.bRecoveringTorque =
            InOutState.EngineTorqueAuthority01 < 1.0;
    }

    OutOutput.EngineTorqueAuthority01 =
        InOutState.EngineTorqueAuthority01;

    return true;
}

bool TADriverAssistControllers::ValidateStabilityConfig(
    const FTAStabilityControlConfig& Config)
{
    return
        FMath::IsFinite(Config.WheelbaseM)
        && Config.WheelbaseM > UE_DOUBLE_SMALL_NUMBER
        && FMath::IsFinite(Config.MinimumVehicleSpeedMps)
        && Config.MinimumVehicleSpeedMps >= 0.0
        && FMath::IsFinite(Config.MaxReferenceLateralAccelerationMps2)
        && Config.MaxReferenceLateralAccelerationMps2 > 0.0
        && FMath::IsFinite(Config.YawRateDeadbandRadPerSec)
        && Config.YawRateDeadbandRadPerSec >= 0.0
        && FMath::IsFinite(Config.YawMomentGainNmPerRadPerSec)
        && Config.YawMomentGainNmPerRadPerSec >= 0.0
        && FMath::IsFinite(Config.MaxCorrectiveYawMomentNm)
        && Config.MaxCorrectiveYawMomentNm >= 0.0;
}

bool TADriverAssistControllers::CalculateStabilityRequest(
    const FTAStabilityControlConfig& Config,
    const FTAStabilityControlInput& Input,
    FTAStabilityControlOutput& OutOutput)
{
    OutOutput =
        FTAStabilityControlOutput{};

    if (!ValidateStabilityConfig(Config)
        || !FMath::IsFinite(Input.RoadWheelSteerAngleRad)
        || !FMath::IsFinite(Input.VehicleSpeedMps)
        || !FMath::IsFinite(Input.MeasuredYawRateRadPerSec))
    {
        return false;
    }

    const double SpeedMps =
        FMath::Max(
            0.0,
            Input.VehicleSpeedMps);

    if (SpeedMps
        < Config.MinimumVehicleSpeedMps)
    {
        return true;
    }

    const double KinematicReferenceYawRate =
        SpeedMps
        / Config.WheelbaseM
        * FMath::Tan(
            Input.RoadWheelSteerAngleRad);

    const double MaxReferenceYawRate =
        Config.MaxReferenceLateralAccelerationMps2
        / FMath::Max(
            SpeedMps,
            UE_DOUBLE_SMALL_NUMBER);

    OutOutput.ReferenceYawRateRadPerSec =
        FMath::Clamp(
            KinematicReferenceYawRate,
            -MaxReferenceYawRate,
            MaxReferenceYawRate);

    OutOutput.YawRateErrorRadPerSec =
        OutOutput.ReferenceYawRateRadPerSec
        - Input.MeasuredYawRateRadPerSec;

    if (FMath::Abs(
            OutOutput.YawRateErrorRadPerSec)
        <= Config.YawRateDeadbandRadPerSec)
    {
        OutOutput.DesiredCorrectiveYawMomentNm =
            0.0;

        return true;
    }

    OutOutput.DesiredCorrectiveYawMomentNm =
        FMath::Clamp(
            OutOutput.YawRateErrorRadPerSec
                * Config.YawMomentGainNmPerRadPerSec,
            -Config.MaxCorrectiveYawMomentNm,
            Config.MaxCorrectiveYawMomentNm);

    return true;
}
