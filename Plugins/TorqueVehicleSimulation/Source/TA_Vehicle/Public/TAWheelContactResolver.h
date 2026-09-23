#pragma once

#include "CoreMinimal.h"
#include "TAChassisDynamics.h"
#include "TADoubleWishboneSolver.h"
#include "TASuspensionRuntime.h"
#include "TAVehicleSimulation.h"

struct TA_VEHICLE_API FTARoadPlane
{
    FVector3d PointWorldM = FVector3d::ZeroVector;
    FVector3d NormalWorld = FVector3d(0.0, 0.0, 1.0);

    FTASurfaceSample Surface;
};

struct TA_VEHICLE_API FTAResolvedWheelContact
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
    double LongitudinalVelocityMps = 0.0;
    double LateralVelocityMps = 0.0;

    FVector3d SuspensionForceWorldN = FVector3d::ZeroVector;

    FTASurfaceSample Surface;

    FTADoubleWishboneSolveOutput Geometry;
    FTASuspensionForceOutput SuspensionForce;
};

namespace TAWheelContactResolver
{
    TA_VEHICLE_API FVector3d CalculatePointVelocityWorld(
        const FTAChassisState& Chassis,
        const FVector3d& PointWorldM);

    TA_VEHICLE_API bool ResolveDoubleWishboneRoadContact(
        const FTAChassisState& Chassis,
        const FTADoubleWishboneSolverConfig& GeometryConfig,
        const FTASuspensionRuntimeConfig& SuspensionConfig,
        double WheelRadiusM,
        double RackDisplacementM,
        const FTADoubleWishboneDamageOffsets& DamageOffsets,
        const FTARoadPlane& Road,
        double DeltaTimeSeconds,
        FTADoubleWishboneState& InOutGeometryState,
        FTASuspensionRuntimeState& InOutSuspensionState,
        FTAResolvedWheelContact& OutContact);

    TA_VEHICLE_API void ApplyAntiRollBarToPair(
        const FTAAntiRollBarConfig& Config,
        FTAResolvedWheelContact& InOutLeftContact,
        FTAResolvedWheelContact& InOutRightContact);

    TA_VEHICLE_API FTAWheelContactInput BuildVehicleWheelContactInput(
        const FTAResolvedWheelContact& Contact);
}
