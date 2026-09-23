#pragma once

#include "CoreMinimal.h"

constexpr int32 TARearMultiLinkCount = 5;

struct TA_VEHICLE_API FTAMultiLinkLinkConfig
{
    FVector3d ChassisPickupReference = FVector3d::ZeroVector;
    FVector3d UprightPickupReference = FVector3d::ZeroVector;
};

struct TA_VEHICLE_API FTAMultiLinkSolverConfig
{
    FTAMultiLinkLinkConfig Links[TARearMultiLinkCount];

    FVector3d WheelCenterReference = FVector3d::ZeroVector;

    FVector3d DamperChassisReference = FVector3d::ZeroVector;
    FVector3d DamperUprightReference = FVector3d::ZeroVector;

    FVector3d WheelForwardReference = FVector3d(1.0, 0.0, 0.0);
    FVector3d WheelUpReference = FVector3d(0.0, 0.0, 1.0);

    // +1 for right side, -1 for left side.
    double SideSign = 1.0;

    double MinTravelM = -0.07;
    double MaxTravelM = 0.09;

    int32 MaxIterations = 80;
    double PositionToleranceM = 0.001;
};

struct TA_VEHICLE_API FTAMultiLinkDamageOffsets
{
    FVector3d ChassisPickupOffsets[TARearMultiLinkCount] = {};
    FVector3d DamperChassisOffset = FVector3d::ZeroVector;
};

struct TA_VEHICLE_API FTAMultiLinkSolveInput
{
    double TravelM = 0.0;
    FTAMultiLinkDamageOffsets Damage;
};

struct TA_VEHICLE_API FTAMultiLinkRuntimeState
{
    FVector3d UprightLinkPoints[TARearMultiLinkCount] = {};

    FVector3d WheelCenter = FVector3d::ZeroVector;
    FVector3d DamperUpright = FVector3d::ZeroVector;

    double SolvedTravelM = 0.0;
    bool bHasValidPreviousSolution = false;
};

struct TA_VEHICLE_API FTAMultiLinkSolveOutput
{
    bool bConverged = false;

    int32 IterationsUsed = 0;
    double MaxConstraintResidualM = 0.0;

    FVector3d WheelCenterLocalM = FVector3d::ZeroVector;

    FVector3d WheelForwardLocal = FVector3d(1.0, 0.0, 0.0);
    FVector3d WheelRightLocal = FVector3d(0.0, 1.0, 0.0);
    FVector3d WheelUpLocal = FVector3d(0.0, 0.0, 1.0);

    FVector3d DamperUprightLocalM = FVector3d::ZeroVector;
    double DamperLengthM = 0.0;

    double CamberRad = 0.0;
    double ToeRad = 0.0;
};

namespace TAMultiLinkSolver
{
    TA_VEHICLE_API FTAMultiLinkSolverConfig MirrorAcrossCenterline(
        const FTAMultiLinkSolverConfig& RightSideConfig);

    TA_VEHICLE_API bool ValidateConfig(
        const FTAMultiLinkSolverConfig& Config);

    TA_VEHICLE_API bool Solve(
        const FTAMultiLinkSolverConfig& Config,
        const FTAMultiLinkSolveInput& Input,
        FTAMultiLinkRuntimeState& InOutState,
        FTAMultiLinkSolveOutput& OutOutput);
}
