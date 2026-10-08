#pragma once

#include "CoreMinimal.h"
#include "TAChassisDynamics.h"

struct TA_VEHICLE_API FTAAerodynamicsConfig
{
    double ReferenceAreaM2 = 2.10;
    double DragCoefficient = 0.32;

    // Negative coefficients produce downforce in the current convention.
    // Front + rear coefficients form the total lift/downforce coefficient.
    double FrontLiftCoefficient = -0.05;
    double RearLiftCoefficient = -0.07;

    // Vehicle-body-local, relative to chassis center of mass.
    FVector3d DragApplicationPointBodyM =
        FVector3d(0.0, 0.0, 0.10);

    FVector3d FrontLiftApplicationPointBodyM =
        FVector3d(1.10, 0.0, 0.0);

    FVector3d RearLiftApplicationPointBodyM =
        FVector3d(-1.10, 0.0, 0.0);
};

struct TA_VEHICLE_API FTAAerodynamicsEnvironment
{
    double AirDensityKgPerM3 = 1.225;
    FVector3d WindVelocityWorldMps = FVector3d::ZeroVector;
};

struct TA_VEHICLE_API FTAAerodynamicsOutput
{
    FVector3d RelativeAirVelocityWorldMps = FVector3d::ZeroVector;

    FVector3d DragForceWorldN = FVector3d::ZeroVector;
    FVector3d FrontLiftForceWorldN = FVector3d::ZeroVector;
    FVector3d RearLiftForceWorldN = FVector3d::ZeroVector;

    FVector3d TotalForceWorldN = FVector3d::ZeroVector;
    FVector3d TotalTorqueWorldNm = FVector3d::ZeroVector;

    FVector3d DragApplicationPointWorldM = FVector3d::ZeroVector;
    FVector3d FrontLiftApplicationPointWorldM = FVector3d::ZeroVector;
    FVector3d RearLiftApplicationPointWorldM = FVector3d::ZeroVector;

    double DynamicPressurePa = 0.0;
    double DragForceN = 0.0;
    double FrontLiftForceN = 0.0;
    double RearLiftForceN = 0.0;

    double TotalLiftForceN() const
    {
        return FrontLiftForceN + RearLiftForceN;
    }
};

namespace TAAerodynamics
{
    TA_VEHICLE_API bool ValidateConfig(
        const FTAAerodynamicsConfig& Config);

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
