#pragma once

#include "CoreMinimal.h"
#include "TAChassisDynamics.h"

struct TA_VEHICLE_API FTAAerodynamicsConfig
{
    double ReferenceAreaM2 = 2.10;
    double DragCoefficient = 0.32;
    double LiftCoefficient = -0.12;

    // Vehicle-local, relative to chassis center of mass.
    FVector3d ApplicationPointBodyM = FVector3d(0.0, 0.0, 0.10);
};

struct TA_VEHICLE_API FTAAerodynamicsEnvironment
{
    double AirDensityKgPerM3 = 1.225;
    FVector3d WindVelocityWorldMps = FVector3d::ZeroVector;
};

struct TA_VEHICLE_API FTAAerodynamicsOutput
{
    FVector3d RelativeAirVelocityWorldMps = FVector3d::ZeroVector;
    FVector3d ForceWorldN = FVector3d::ZeroVector;
    FVector3d TorqueWorldNm = FVector3d::ZeroVector;
    FVector3d ApplicationPointWorldM = FVector3d::ZeroVector;

    double DynamicPressurePa = 0.0;
    double DragForceN = 0.0;
    double LiftForceN = 0.0;
};

namespace TAAerodynamics
{
    TA_VEHICLE_API bool ValidateConfig(const FTAAerodynamicsConfig& Config);

    TA_VEHICLE_API bool Calculate(
        const FTAAerodynamicsConfig& Config,
        const FTAAerodynamicsEnvironment& Environment,
        const FTAChassisState& Chassis,
        FTAAerodynamicsOutput& OutOutput);

    TA_VEHICLE_API bool AddToChassis(
        const FTAAerodynamicsConfig& Config,
        const FTAAerodynamicsEnvironment& Environment,
        const FTAChassisState& Chassis,
        FTAChassisForceAccumulator& InOutAccumulator,
        FTAAerodynamicsOutput* OutOutput = nullptr);
}
