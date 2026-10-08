#pragma once

#include "CoreMinimal.h"

enum class ETAAbsMode : uint8
{
    Inactive,
    Build,
    Hold,
    Release
};

struct TA_VEHICLE_API FTAAbsConfig
{
    double MinimumVehicleSpeedMps = 2.0;
    double MinimumBrakeRequest01 = 0.05;

    // Current tire convention: braking slip is negative.
    double ReleaseSlipMagnitude = 0.18;
    double BuildSlipMagnitude = 0.10;

    // Positive magnitude of wheel angular deceleration.
    double ReleaseWheelAngularDecelRadPerSec2 = 180.0;

    double PressureBuildRate01PerSec = 5.0;
    double PressureReleaseRate01PerSec = 12.0;

    double MinimumPressureModulation01 = 0.05;
};

struct TA_VEHICLE_API FTAAbsState
{
    ETAAbsMode Mode = ETAAbsMode::Inactive;
    double PressureModulation01 = 1.0;
};

struct TA_VEHICLE_API FTAAbsInput
{
    double BrakeRequest01 = 0.0;
    double VehicleSpeedMps = 0.0;

    double SlipRatio = 0.0;
    double WheelAngularDecelerationRadPerSec2 = 0.0;

    bool bContactValid = true;

    double DeltaTimeSeconds = 1.0 / 240.0;
};

struct TA_VEHICLE_API FTAAbsOutput
{
    ETAAbsMode Mode = ETAAbsMode::Inactive;
    double PressureModulation01 = 1.0;

    double BrakeSlipMagnitude = 0.0;

    bool bReleaseTriggeredBySlip = false;
    bool bReleaseTriggeredByWheelDeceleration = false;
};

namespace TAAbsController
{
    TA_VEHICLE_API bool ValidateConfig(
        const FTAAbsConfig& Config);

    TA_VEHICLE_API void InitializeState(
        FTAAbsState& OutState);

    TA_VEHICLE_API bool Step(
        const FTAAbsConfig& Config,
        const FTAAbsInput& Input,
        FTAAbsState& InOutState,
        FTAAbsOutput& OutOutput);
}
