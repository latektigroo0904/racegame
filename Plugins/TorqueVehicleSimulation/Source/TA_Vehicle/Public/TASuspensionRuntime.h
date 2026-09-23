#pragma once

#include "CoreMinimal.h"

struct TA_VEHICLE_API FTASuspensionKinematicSample
{
    double TravelM = 0.0;

    FVector3d WheelCenterOffsetM = FVector3d::ZeroVector;

    double CamberRad = 0.0;
    double ToeRad = 0.0;
    double MotionRatio = 1.0;
};

struct TA_VEHICLE_API FTASuspensionRuntimeConfig
{
    double SpringRateNPerM = 45000.0;

    double BumpDampingNsPerM = 3500.0;
    double ReboundDampingNsPerM = 5200.0;

    double BumpStopTravelM = 0.09;
    double DroopStopTravelM = -0.07;

    double BumpStopRateNPerM = 180000.0;
    double DroopStopRateNPerM = 120000.0;

    TArray<FTASuspensionKinematicSample> KinematicSamples;
};

struct TA_VEHICLE_API FTASuspensionRuntimeState
{
    double TravelM = 0.0;
    double TravelVelocityMps = 0.0;

    FVector3d WheelCenterOffsetM = FVector3d::ZeroVector;

    double CamberRad = 0.0;
    double ToeRad = 0.0;
    double MotionRatio = 1.0;

    FVector3d StructuralPickupOffsetM = FVector3d::ZeroVector;

    bool bKinematicCacheValid = true;
};

struct TA_VEHICLE_API FTASuspensionForceOutput
{
    double SpringForceN = 0.0;
    double DamperForceN = 0.0;
    double StopForceN = 0.0;

    double TotalForceN = 0.0;
};

namespace TASuspensionRuntime
{
    TA_VEHICLE_API bool EvaluateKinematicCache(
        const FTASuspensionRuntimeConfig& Config,
        double TravelM,
        FTASuspensionRuntimeState& InOutState);

    TA_VEHICLE_API FTASuspensionForceOutput CalculateForce(
        const FTASuspensionRuntimeConfig& Config,
        const FTASuspensionRuntimeState& State);
}
