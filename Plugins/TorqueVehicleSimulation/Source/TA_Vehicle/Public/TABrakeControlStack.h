#pragma once

#include "CoreMinimal.h"
#include "TAAbsController.h"
#include "TABrakeHydraulics.h"

struct TA_VEHICLE_API FTABrakeControlStackConfig
{
    FTABrakeHydraulicConfig Hydraulics;
    FTAAbsConfig Abs;
};

struct TA_VEHICLE_API FTABrakeControlStackState
{
    FTABrakeHydraulicState Hydraulics;
    FTAAbsState Abs[TABrakeCornerCount];
};

struct TA_VEHICLE_API FTABrakeControlStackInput
{
    double BrakePedal01 = 0.0;
    double VehicleSpeedMps = 0.0;

    double SlipRatio[TABrakeCornerCount] =
        { 0.0, 0.0, 0.0, 0.0 };

    double WheelAngularDecelerationRadPerSec2[TABrakeCornerCount] =
        { 0.0, 0.0, 0.0, 0.0 };

    bool bContactValid[TABrakeCornerCount] =
        { true, true, true, true };

    double DeltaTimeSeconds = 1.0 / 240.0;
};

struct TA_VEHICLE_API FTABrakeControlStackOutput
{
    FTAAbsOutput Abs[TABrakeCornerCount];
    FTABrakeHydraulicOutput Hydraulics;
};

namespace TABrakeControlStack
{
    TA_VEHICLE_API bool ValidateConfig(
        const FTABrakeControlStackConfig& Config);

    TA_VEHICLE_API void InitializeState(
        const FTABrakeControlStackConfig& Config,
        FTABrakeControlStackState& OutState);

    TA_VEHICLE_API bool Step(
        const FTABrakeControlStackConfig& Config,
        const FTABrakeControlStackInput& Input,
        FTABrakeControlStackState& InOutState,
        FTABrakeControlStackOutput& OutOutput);
}
