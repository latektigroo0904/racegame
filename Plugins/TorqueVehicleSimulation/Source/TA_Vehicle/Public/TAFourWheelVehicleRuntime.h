#pragma once

#include "CoreMinimal.h"
#include "TAFrontAxleRuntime.h"
#include "TARearAxleRuntime.h"
#include "TAVehicleSimulation.h"

enum class ETAPrototypeWheelIndex : uint8
{
    FrontLeft = 0,
    FrontRight = 1,
    RearLeft = 2,
    RearRight = 3
};

struct TA_VEHICLE_API FTAFourWheelRuntimeConfig
{
    FTAFrontAxleRuntimeConfig FrontAxle;
    FTARearAxleRuntimeConfig RearAxle;
};

struct TA_VEHICLE_API FTAFourWheelRuntimeState
{
    FTAVehicleRuntimeState Vehicle;

    FTAFrontAxleRuntimeState FrontAxle;
    FTARearAxleRuntimeState RearAxle;
};

struct TA_VEHICLE_API FTAFourWheelStepInput
{
    FTADriverControls Controls;

    FTARoadPlane FrontLeftRoad;
    FTARoadPlane FrontRightRoad;
    FTARoadPlane RearLeftRoad;
    FTARoadPlane RearRightRoad;

    FTADoubleWishboneDamageOffsets FrontLeftDamage;
    FTADoubleWishboneDamageOffsets FrontRightDamage;

    FTAMultiLinkDamageOffsets RearLeftDamage;
    FTAMultiLinkDamageOffsets RearRightDamage;
};

struct TA_VEHICLE_API FTAFourWheelStepOutput
{
    bool bContactsSolved = false;

    FTAFrontAxleSolveOutput FrontAxle;
    FTARearAxleSolveOutput RearAxle;

    FTAVehicleStepOutput Vehicle;
};

namespace TAFourWheelVehicleRuntime
{
    TA_VEHICLE_API bool Initialize(
        const FTAVehicleRuntimeConfig& VehicleConfig,
        FTAFourWheelRuntimeState& OutState);

    TA_VEHICLE_API bool Step(
        const FTAVehicleRuntimeConfig& VehicleConfig,
        const FTAFourWheelRuntimeConfig& RuntimeConfig,
        const FTAFourWheelStepInput& Input,
        double DeltaTimeSeconds,
        FTAFourWheelRuntimeState& InOutState,
        FTAFourWheelStepOutput& OutOutput);
}
