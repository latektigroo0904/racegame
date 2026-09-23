#pragma once

#include "CoreMinimal.h"
#include "TADoubleWishboneSolver.h"
#include "TASuspensionRuntime.h"
#include "TAWheelContactResolver.h"

struct TA_VEHICLE_API FTASteeringRackRuntimeConfig
{
    double MaxRackDisplacementM = 0.035;

    // >1 softens response around center; <1 sharpens it.
    double InputExponent = 1.0;

    // TA-P01 convention: positive driver steering = right turn.
    // The current rear-steer tie-rod seed requires negative rack travel for a right turn.
    double SteeringSign = -1.0;
};

struct TA_VEHICLE_API FTAFrontAxleRuntimeConfig
{
    // Canonical authored side. Left side is mirrored from this at runtime/compile time.
    FTADoubleWishboneSolverConfig RightGeometry;

    FTASuspensionRuntimeConfig LeftSuspension;
    FTASuspensionRuntimeConfig RightSuspension;

    FTAAntiRollBarConfig AntiRollBar;
    FTASteeringRackRuntimeConfig SteeringRack;
};

struct TA_VEHICLE_API FTAFrontAxleRuntimeState
{
    FTADoubleWishboneState LeftGeometry;
    FTADoubleWishboneState RightGeometry;

    FTASuspensionRuntimeState LeftSuspension;
    FTASuspensionRuntimeState RightSuspension;
};

struct TA_VEHICLE_API FTAFrontAxleSolveInput
{
    double Steering01 = 0.0;

    // Functional degradation only; structural geometry damage remains in
    // LeftDamage/RightDamage.
    double SteeringCommandAuthority01 = 1.0;
    double SteeringRackFreePlayM = 0.0;

    FTARoadPlane LeftRoad;
    FTARoadPlane RightRoad;

    FTADoubleWishboneDamageOffsets LeftDamage;
    FTADoubleWishboneDamageOffsets RightDamage;
};

struct TA_VEHICLE_API FTAFrontAxleSolveOutput
{
    bool bSolved = false;

    double RackDisplacementM = 0.0;

    FTAResolvedWheelContact LeftContact;
    FTAResolvedWheelContact RightContact;

    FTAWheelContactInput LeftVehicleContact;
    FTAWheelContactInput RightVehicleContact;

    double LeftSteeringAngleRad = 0.0;
    double RightSteeringAngleRad = 0.0;

    double LeftBumpSteerRad = 0.0;
    double RightBumpSteerRad = 0.0;

    double AckermannDeltaRad = 0.0;
};

namespace TAFrontAxleRuntime
{
    TA_VEHICLE_API double CalculateRackDisplacementM(
        const FTASteeringRackRuntimeConfig& Config,
        double Steering01);

    TA_VEHICLE_API bool Resolve(
        const FTAChassisState& Chassis,
        const FTAFrontAxleRuntimeConfig& Config,
        const FTAFrontAxleSolveInput& Input,
        double LeftWheelRadiusM,
        double RightWheelRadiusM,
        double DeltaTimeSeconds,
        FTAFrontAxleRuntimeState& InOutState,
        FTAFrontAxleSolveOutput& OutOutput);

    TA_VEHICLE_API bool ResolveWithTireCompliance(
        const FTAChassisState& Chassis,
        const FTAFrontAxleRuntimeConfig& Config,
        const FTAFrontAxleSolveInput& Input,
        const FTATireRuntimeConfig& LeftTireConfig,
        const FTATireRuntimeConfig& RightTireConfig,
        double DeltaTimeSeconds,
        FTAFrontAxleRuntimeState& InOutState,
        FTATireRuntimeState& InOutLeftTireState,
        FTATireRuntimeState& InOutRightTireState,
        FTAFrontAxleSolveOutput& OutOutput);
}
