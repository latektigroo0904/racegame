#pragma once

#include "CoreMinimal.h"
#include "TAUnsprungVerticalDynamics.h"
#include "TAWheelContactResolver.h"

struct TA_VEHICLE_API FTAExperimentalUnsprungCornerConfig
{
    FTADoubleWishboneSolverConfig Geometry;
    FTASuspensionRuntimeConfig Suspension;
    FTATireRuntimeConfig Tire;
    FTAUnsprungVerticalConfig Unsprung;

    FVector3d GravityWorldMps2 =
        FVector3d(0.0, 0.0, -9.80665);

    // Experimental local integration refinement only.
    int32 InternalSubsteps = 1;
};

struct TA_VEHICLE_API FTAExperimentalUnsprungCornerInput
{
    FTAChassisState Chassis;
    FTARoadPlane Road;

    double RackDisplacementM = 0.0;
    double AdditionalSuspensionReactionN = 0.0;

    FTADoubleWishboneDamageOffsets Damage;

    // Acceleration of the chassis reference frame used by the relative
    // unsprung coordinate.
    FVector3d ChassisLinearAccelerationWorldMps2 =
        FVector3d::ZeroVector;
};

struct TA_VEHICLE_API FTAExperimentalUnsprungCornerState
{
    FTADoubleWishboneState Geometry;
    FTASuspensionRuntimeState Suspension;
    FTATireRuntimeState Tire;
    FTAUnsprungVerticalState Unsprung;

    bool bInitialized = false;
};

struct TA_VEHICLE_API FTAExperimentalUnsprungCornerOutput
{
    bool bSolved = false;
    bool bInContact = false;

    double TireNormalForceN = 0.0;
    double ChassisSuspensionReactionN = 0.0;

    double GeneralizedTireForceN = 0.0;
    double GeneralizedGravityForceN = 0.0;
    double GeneralizedChassisInertialForceN = 0.0;

    // Positive means net force toward bump along the unsprung travel axis.
    double GeneralizedForceBalanceN = 0.0;

    double RequestedTireDeflectionM = 0.0;
    double TireRadialDeflectionM = 0.0;

    double TravelM = 0.0;
    double TravelVelocityMps = 0.0;

    FVector3d RoadNormalWorld =
        FVector3d(0.0, 0.0, 1.0);

    FVector3d ChassisSuspensionForceWorldN =
        FVector3d::ZeroVector;

    FVector3d ChassisSuspensionApplicationPointWorldM =
        FVector3d::ZeroVector;

    FTADoubleWishboneSolveOutput Geometry;
    FTASuspensionForceOutput SuspensionForce;
    FTAUnsprungVerticalOutput Unsprung;

    // VerticalLoadN is tire-road normal load.
    // SuspensionForceWorldN is the force transmitted to the chassis.
    // These are intentionally not forced equal in the dynamic-unsprung path.
    FTAWheelContactInput VehicleContact;
};

namespace TAExperimentalUnsprungCorner
{
    TA_VEHICLE_API bool ValidateConfig(
        const FTAExperimentalUnsprungCornerConfig& Config);

    // Seeds geometry, suspension and tire state from the current canonical
    // quasi-static contact equilibrium. Dynamic unsprung state starts with
    // zero relative travel velocity.
    TA_VEHICLE_API bool InitializeFromQuasiStatic(
        const FTAExperimentalUnsprungCornerConfig& Config,
        const FTAExperimentalUnsprungCornerInput& Input,
        double DeltaTimeSeconds,
        FTAExperimentalUnsprungCornerState& OutState,
        FTAExperimentalUnsprungCornerOutput& OutOutput);

    TA_VEHICLE_API bool Step(
        const FTAExperimentalUnsprungCornerConfig& Config,
        const FTAExperimentalUnsprungCornerInput& Input,
        double DeltaTimeSeconds,
        FTAExperimentalUnsprungCornerState& InOutState,
        FTAExperimentalUnsprungCornerOutput& OutOutput);
}
