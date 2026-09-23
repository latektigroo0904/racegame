#pragma once

#include "CoreMinimal.h"
#include "TAMultiLinkSolver.h"
#include "TASuspensionRuntime.h"
#include "TAWheelContactResolver.h"

struct TA_VEHICLE_API FTAResolvedMultiLinkContact
{
    bool bInContact = false;
    bool bTravelClamped = false;

    double TravelM = 0.0;
    double PenetrationM = 0.0;

    FVector3d WheelCenterWorldM = FVector3d::ZeroVector;
    FVector3d ContactPointWorldM = FVector3d::ZeroVector;

    FVector3d ForwardTangentWorld = FVector3d(1.0, 0.0, 0.0);
    FVector3d RightTangentWorld = FVector3d(0.0, 1.0, 0.0);
    FVector3d RoadNormalWorld = FVector3d(0.0, 0.0, 1.0);

    double VerticalLoadN = 0.0;

    double TireRadialDeflectionM = 0.0;
    bool bTireBottomed = false;

    double LongitudinalVelocityMps = 0.0;
    double LateralVelocityMps = 0.0;

    FVector3d SuspensionForceWorldN = FVector3d::ZeroVector;

    FTASurfaceSample Surface;

    FTAMultiLinkSolveOutput Geometry;
    FTASuspensionForceOutput SuspensionForce;
};

namespace TAMultiLinkContactResolver
{
    TA_VEHICLE_API bool ResolveRoadContact(
        const FTAChassisState& Chassis,
        const FTAMultiLinkSolverConfig& GeometryConfig,
        const FTASuspensionRuntimeConfig& SuspensionConfig,
        double WheelRadiusM,
        const FTAMultiLinkDamageOffsets& DamageOffsets,
        const FTARoadPlane& Road,
        double DeltaTimeSeconds,
        FTAMultiLinkRuntimeState& InOutGeometryState,
        FTASuspensionRuntimeState& InOutSuspensionState,
        FTAResolvedMultiLinkContact& OutContact);

    TA_VEHICLE_API bool ResolveCompliantRoadContact(
        const FTAChassisState& Chassis,
        const FTAMultiLinkSolverConfig& GeometryConfig,
        const FTASuspensionRuntimeConfig& SuspensionConfig,
        const FTATireRuntimeConfig& TireConfig,
        const FTAMultiLinkDamageOffsets& DamageOffsets,
        const FTARoadPlane& Road,
        double DeltaTimeSeconds,
        FTAMultiLinkRuntimeState& InOutGeometryState,
        FTASuspensionRuntimeState& InOutSuspensionState,
        FTATireRuntimeState& InOutTireState,
        FTAResolvedMultiLinkContact& OutContact);

    TA_VEHICLE_API void ApplyAntiRollBarToPair(
        const FTAAntiRollBarConfig& Config,
        FTAResolvedMultiLinkContact& InOutLeftContact,
        FTAResolvedMultiLinkContact& InOutRightContact);

    TA_VEHICLE_API FTAWheelContactInput BuildVehicleWheelContactInput(
        const FTAResolvedMultiLinkContact& Contact);
}
