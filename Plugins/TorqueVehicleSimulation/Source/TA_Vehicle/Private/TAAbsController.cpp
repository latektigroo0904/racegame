#include "TAAbsController.h"

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

bool TAAbsController::ValidateConfig(
    const FTAAbsConfig& Config)
{
    return
        FMath::IsFinite(Config.MinimumVehicleSpeedMps)
        && Config.MinimumVehicleSpeedMps >= 0.0
        && FMath::IsFinite(Config.MinimumBrakeRequest01)
        && Config.MinimumBrakeRequest01 >= 0.0
        && Config.MinimumBrakeRequest01 <= 1.0
        && FMath::IsFinite(Config.ReleaseSlipMagnitude)
        && Config.ReleaseSlipMagnitude > 0.0
        && FMath::IsFinite(Config.BuildSlipMagnitude)
        && Config.BuildSlipMagnitude >= 0.0
        && Config.BuildSlipMagnitude
            < Config.ReleaseSlipMagnitude
        && FMath::IsFinite(
            Config.ReleaseWheelAngularDecelRadPerSec2)
        && Config.ReleaseWheelAngularDecelRadPerSec2 >= 0.0
        && FMath::IsFinite(Config.PressureBuildRate01PerSec)
        && Config.PressureBuildRate01PerSec > 0.0
        && FMath::IsFinite(Config.PressureReleaseRate01PerSec)
        && Config.PressureReleaseRate01PerSec > 0.0
        && FMath::IsFinite(Config.MinimumPressureModulation01)
        && Config.MinimumPressureModulation01 >= 0.0
        && Config.MinimumPressureModulation01 <= 1.0;
}

void TAAbsController::InitializeState(
    FTAAbsState& OutState)
{
    OutState =
        FTAAbsState{};
}

bool TAAbsController::Step(
    const FTAAbsConfig& Config,
    const FTAAbsInput& Input,
    FTAAbsState& InOutState,
    FTAAbsOutput& OutOutput)
{
    OutOutput =
        FTAAbsOutput{};

    if (!ValidateConfig(Config)
        || !FMath::IsFinite(Input.BrakeRequest01)
        || !FMath::IsFinite(Input.VehicleSpeedMps)
        || !FMath::IsFinite(Input.SlipRatio)
        || !FMath::IsFinite(
            Input.WheelAngularDecelerationRadPerSec2)
        || !FMath::IsFinite(Input.DeltaTimeSeconds)
        || Input.DeltaTimeSeconds <= 0.0
        || !FMath::IsFinite(
            InOutState.PressureModulation01))
    {
        return false;
    }

    const double BrakeRequest01 =
        FMath::Clamp(
            Input.BrakeRequest01,
            0.0,
            1.0);

    InOutState.PressureModulation01 =
        FMath::Clamp(
            InOutState.PressureModulation01,
            Config.MinimumPressureModulation01,
            1.0);

    const bool bControllerActive =
        Input.bContactValid
        && Input.VehicleSpeedMps
            >= Config.MinimumVehicleSpeedMps
        && BrakeRequest01
            >= Config.MinimumBrakeRequest01;

    OutOutput.BrakeSlipMagnitude =
        FMath::Max(
            0.0,
            -Input.SlipRatio);

    if (!bControllerActive)
    {
        InOutState.Mode =
            ETAAbsMode::Inactive;

        InOutState.PressureModulation01 =
            MoveToward(
                InOutState.PressureModulation01,
                1.0,
                Config.PressureBuildRate01PerSec,
                Input.DeltaTimeSeconds);

        OutOutput.Mode =
            InOutState.Mode;

        OutOutput.PressureModulation01 =
            InOutState.PressureModulation01;

        return true;
    }

    OutOutput.bReleaseTriggeredBySlip =
        OutOutput.BrakeSlipMagnitude
            >= Config.ReleaseSlipMagnitude;

    OutOutput.bReleaseTriggeredByWheelDeceleration =
        Input.WheelAngularDecelerationRadPerSec2
            >= Config.ReleaseWheelAngularDecelRadPerSec2;

    if (OutOutput.bReleaseTriggeredBySlip
        || OutOutput.bReleaseTriggeredByWheelDeceleration)
    {
        InOutState.Mode =
            ETAAbsMode::Release;
    }
    else if (OutOutput.BrakeSlipMagnitude
        <= Config.BuildSlipMagnitude)
    {
        InOutState.Mode =
            ETAAbsMode::Build;
    }
    else
    {
        InOutState.Mode =
            ETAAbsMode::Hold;
    }

    switch (InOutState.Mode)
    {
    case ETAAbsMode::Build:
        InOutState.PressureModulation01 =
            MoveToward(
                InOutState.PressureModulation01,
                1.0,
                Config.PressureBuildRate01PerSec,
                Input.DeltaTimeSeconds);
        break;

    case ETAAbsMode::Release:
        InOutState.PressureModulation01 =
            MoveToward(
                InOutState.PressureModulation01,
                Config.MinimumPressureModulation01,
                Config.PressureReleaseRate01PerSec,
                Input.DeltaTimeSeconds);
        break;

    case ETAAbsMode::Hold:
    case ETAAbsMode::Inactive:
    default:
        break;
    }

    OutOutput.Mode =
        InOutState.Mode;

    OutOutput.PressureModulation01 =
        InOutState.PressureModulation01;

    return true;
}
