#pragma once

#include "CoreMinimal.h"

struct TA_VEHICLE_API FTAChassisConfig
{
    double MassKg = 1420.0;

    FVector3d PrincipalInertiaBodyKgm2 =
        FVector3d(650.0, 1800.0, 1900.0);

    FVector3d GravityWorldMps2 =
        FVector3d(0.0, 0.0, -9.80665);
};

struct TA_VEHICLE_API FTAChassisState
{
    FVector3d PositionWorldM = FVector3d::ZeroVector;

    FQuat OrientationWorld = FQuat::Identity;

    FVector3d LinearVelocityWorldMps = FVector3d::ZeroVector;
    FVector3d AngularVelocityWorldRadPerSec = FVector3d::ZeroVector;
};

struct TA_VEHICLE_API FTAChassisForceAccumulator
{
    FVector3d TotalForceWorldN = FVector3d::ZeroVector;
    FVector3d TotalTorqueWorldNm = FVector3d::ZeroVector;

    void Reset()
    {
        TotalForceWorldN = FVector3d::ZeroVector;
        TotalTorqueWorldNm = FVector3d::ZeroVector;
    }
};

namespace TAChassisDynamics
{
    TA_VEHICLE_API bool IsConfigValid(
        const FTAChassisConfig& Config);

    TA_VEHICLE_API void AddForceWorld(
        const FVector3d& ForceWorldN,
        FTAChassisForceAccumulator& InOutAccumulator);

    TA_VEHICLE_API void AddTorqueWorld(
        const FVector3d& TorqueWorldNm,
        FTAChassisForceAccumulator& InOutAccumulator);

    TA_VEHICLE_API void AddForceAtWorldPoint(
        const FTAChassisState& State,
        const FVector3d& ForceWorldN,
        const FVector3d& PointWorldM,
        FTAChassisForceAccumulator& InOutAccumulator);

    TA_VEHICLE_API bool ApplyImpulseAtWorldPoint(
        const FTAChassisConfig& Config,
        const FVector3d& ImpulseWorldNs,
        const FVector3d& PointWorldM,
        FTAChassisState& InOutState);

    TA_VEHICLE_API bool Integrate(
        const FTAChassisConfig& Config,
        const FTAChassisForceAccumulator& Accumulator,
        double DeltaTimeSeconds,
        FTAChassisState& InOutState);
}
