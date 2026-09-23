#pragma once

#include "CoreMinimal.h"

struct TA_STRUCTURE_API FTAStructureNode
{
    FVector3d PositionM = FVector3d::ZeroVector;
    FVector3d PreviousPositionM = FVector3d::ZeroVector;
    FVector3d VelocityMps = FVector3d::ZeroVector;

    double InverseMassPerKg = 1.0;

    bool bPinned = false;
};

struct TA_STRUCTURE_API FTADistanceConstraint
{
    int32 NodeA = INDEX_NONE;
    int32 NodeB = INDEX_NONE;

    double RestLengthM = 1.0;

    // XPBD compliance. Zero approaches a rigid constraint.
    double Compliance = 0.0;

    double YieldStrain = 0.08;
    double FractureStrain = 0.30;
    double PlasticFlowRate01 = 0.25;

    double AccumulatedLambda = 0.0;
    double AccumulatedPlasticStrain = 0.0;

    bool bBroken = false;
};

struct TA_STRUCTURE_API FTAStructureSolverConfig
{
    FVector3d GravityMps2 = FVector3d(0.0, 0.0, -9.80665);

    int32 ConstraintIterations = 8;

    double MaxPositionCorrectionM = 0.10;
    double MaxPlasticRestChangeFractionPerStep = 0.03;
};

namespace TAStructureSolver
{
    TA_STRUCTURE_API void PredictPositions(
        const FTAStructureSolverConfig& Config,
        double DeltaTimeSeconds,
        TArray<FTAStructureNode>& Nodes);

    TA_STRUCTURE_API void SolveDistanceConstraints(
        const FTAStructureSolverConfig& Config,
        double DeltaTimeSeconds,
        TArray<FTAStructureNode>& Nodes,
        TArray<FTADistanceConstraint>& Constraints);

    TA_STRUCTURE_API void UpdatePlasticityAndFracture(
        const FTAStructureSolverConfig& Config,
        TArray<FTAStructureNode>& Nodes,
        TArray<FTADistanceConstraint>& Constraints);

    TA_STRUCTURE_API void FinalizeVelocities(
        double DeltaTimeSeconds,
        TArray<FTAStructureNode>& Nodes);

    TA_STRUCTURE_API void Step(
        const FTAStructureSolverConfig& Config,
        double DeltaTimeSeconds,
        TArray<FTAStructureNode>& Nodes,
        TArray<FTADistanceConstraint>& Constraints);
}
