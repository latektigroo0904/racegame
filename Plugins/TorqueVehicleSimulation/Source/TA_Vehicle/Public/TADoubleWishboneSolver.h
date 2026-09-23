#pragma once

#include "CoreMinimal.h"

struct TA_VEHICLE_API FTADoubleWishboneHardpoints
{
    FVector3d UpperInnerA = FVector3d::ZeroVector;
    FVector3d UpperInnerB = FVector3d::ZeroVector;

    FVector3d LowerInnerA = FVector3d::ZeroVector;
    FVector3d LowerInnerB = FVector3d::ZeroVector;

    FVector3d TieRodInner = FVector3d::ZeroVector;

    FVector3d UpperBallJointReference = FVector3d::ZeroVector;
    FVector3d LowerBallJointReference = FVector3d::ZeroVector;
    FVector3d TieRodOuterReference = FVector3d::ZeroVector;
    FVector3d WheelCenterReference = FVector3d::ZeroVector;

    FVector3d WheelForwardReference = FVector3d(1.0, 0.0, 0.0);
    FVector3d WheelUpReference = FVector3d(0.0, 0.0, 1.0);

    FVector3d SteeringRackAxisLocal = FVector3d(0.0, 1.0, 0.0);

    // +1 for right side, -1 for left side.
    double SideSign = 1.0;
};

struct TA_VEHICLE_API FTADoubleWishboneSolverConfig
{
    FTADoubleWishboneHardpoints Hardpoints;

    double MinTravelM = -0.07;
    double MaxTravelM = 0.09;

    int32 MaxIterations = 40;
    double PositionToleranceM = 0.0005;
};

struct TA_VEHICLE_API FTADoubleWishboneDamageOffsets
{
    FVector3d UpperInnerA = FVector3d::ZeroVector;
    FVector3d UpperInnerB = FVector3d::ZeroVector;

    FVector3d LowerInnerA = FVector3d::ZeroVector;
    FVector3d LowerInnerB = FVector3d::ZeroVector;

    FVector3d TieRodInner = FVector3d::ZeroVector;
};

struct TA_VEHICLE_API FTADoubleWishboneSolveInput
{
    double TravelM = 0.0;
    double RackDisplacementM = 0.0;

    FTADoubleWishboneDamageOffsets Damage;
};

struct TA_VEHICLE_API FTADoubleWishboneState
{
    FVector3d UpperBallJoint = FVector3d::ZeroVector;
    FVector3d LowerBallJoint = FVector3d::ZeroVector;
    FVector3d TieRodOuter = FVector3d::ZeroVector;
    FVector3d WheelCenter = FVector3d::ZeroVector;

    double SolvedTravelM = 0.0;

    bool bHasValidPreviousSolution = false;
};

struct TA_VEHICLE_API FTADoubleWishboneSolveOutput
{
    bool bConverged = false;

    int32 IterationsUsed = 0;
    double MaxConstraintResidualM = 0.0;

    FVector3d WheelCenterLocalM = FVector3d::ZeroVector;

    FVector3d WheelForwardLocal = FVector3d(1.0, 0.0, 0.0);
    FVector3d WheelRightLocal = FVector3d(0.0, 1.0, 0.0);
    FVector3d WheelUpLocal = FVector3d(0.0, 0.0, 1.0);

    double CamberRad = 0.0;
    double ToeRad = 0.0;
};

namespace TADoubleWishboneSolver
{
    TA_VEHICLE_API bool ValidateConfig(
        const FTADoubleWishboneSolverConfig& Config);

    TA_VEHICLE_API bool Solve(
        const FTADoubleWishboneSolverConfig& Config,
        const FTADoubleWishboneSolveInput& Input,
        FTADoubleWishboneState& InOutState,
        FTADoubleWishboneSolveOutput& OutOutput);
}
