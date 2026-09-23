#pragma once

#include "CoreMinimal.h"
#include "TAMultiLinkContactResolver.h"

struct TA_VEHICLE_API FTARearAxleRuntimeConfig
{
    FTAMultiLinkSolverConfig RightGeometry;

    FTASuspensionRuntimeConfig LeftSuspension;
    FTASuspensionRuntimeConfig RightSuspension;

    FTAAntiRollBarConfig AntiRollBar;
};

struct TA_VEHICLE_API FTARearAxleRuntimeState
{
    FTAMultiLinkRuntimeState LeftGeometry;
    FTAMultiLinkRuntimeState RightGeometry;

    FTASuspensionRuntimeState LeftSuspension;
    FTASuspensionRuntimeState RightSuspension;
};

struct TA_VEHICLE_API FTARearAxleSolveInput
{
    FTARoadPlane LeftRoad;
    FTARoadPlane RightRoad;

    FTAMultiLinkDamageOffsets LeftDamage;
    FTAMultiLinkDamageOffsets RightDamage;
};

struct TA_VEHICLE_API FTARearAxleSolveOutput
{
    bool bSolved = false;

    FTAResolvedMultiLinkContact LeftContact;
    FTAResolvedMultiLinkContact RightContact;

    FTAWheelContactInput LeftVehicleContact;
    FTAWheelContactInput RightVehicleContact;

    double LeftToeRad = 0.0;
    double RightToeRad = 0.0;

    double LeftCamberRad = 0.0;
    double RightCamberRad = 0.0;
};

namespace TARearAxleRuntime
{
    TA_VEHICLE_API bool Resolve(
        const FTAChassisState& Chassis,
        const FTARearAxleRuntimeConfig& Config,
        const FTARearAxleSolveInput& Input,
        double LeftWheelRadiusM,
        double RightWheelRadiusM,
        double DeltaTimeSeconds,
        FTARearAxleRuntimeState& InOutState,
        FTARearAxleSolveOutput& OutOutput);

    TA_VEHICLE_API bool ResolveWithTireCompliance(
        const FTAChassisState& Chassis,
        const FTARearAxleRuntimeConfig& Config,
        const FTARearAxleSolveInput& Input,
        const FTATireRuntimeConfig& LeftTireConfig,
        const FTATireRuntimeConfig& RightTireConfig,
        double DeltaTimeSeconds,
        FTARearAxleRuntimeState& InOutState,
        FTATireRuntimeState& InOutLeftTireState,
        FTATireRuntimeState& InOutRightTireState,
        FTARearAxleSolveOutput& OutOutput);
}
