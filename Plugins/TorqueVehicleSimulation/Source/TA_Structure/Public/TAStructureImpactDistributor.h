#pragma once

#include "CoreMinimal.h"
#include "TAStructureSolver.h"

struct TA_STRUCTURE_API FTAStructureImpactConfig
{
    double DistributionRadiusM = 0.75;
    double DistanceFalloffExponent = 2.0;

    // Portion used only to seed internal deformation modes.
    // The external rigid-body collision impulse remains owned by the chassis solver.
    double DeformationImpulseFraction01 = 0.35;

    double MaxNodeDeltaVelocityMps = 35.0;
};

struct TA_STRUCTURE_API FTAStructureImpactInput
{
    FVector3d ContactPointLocalM = FVector3d::ZeroVector;
    FVector3d CollisionImpulseLocalNs = FVector3d::ZeroVector;
};

struct TA_STRUCTURE_API FTAStructureImpactOutput
{
    bool bApplied = false;

    int32 AffectedNodeCount = 0;

    double RequestedDeformationImpulseNs = 0.0;
    double AppliedInternalImpulseL1Ns = 0.0;
    double InjectedDeformationKineticEnergyJ = 0.0;

    FVector3d ResidualLinearImpulseNs = FVector3d::ZeroVector;
    FVector3d ResidualAngularImpulseNms = FVector3d::ZeroVector;
};

struct TA_STRUCTURE_API FTAStructureImpactScratch
{
    TArray<int32> NodeIndices;
    TArray<double> Weights;
    TArray<FVector3d> InternalImpulsesNs;

    void Initialize(const int32 MaxNodes)
    {
        NodeIndices.Reset();
        Weights.Reset();
        InternalImpulsesNs.Reset();

        NodeIndices.Reserve(FMath::Max(0, MaxNodes));
        Weights.Reserve(FMath::Max(0, MaxNodes));
        InternalImpulsesNs.Reserve(FMath::Max(0, MaxNodes));
    }

    void ResetKeepCapacity()
    {
        NodeIndices.Reset();
        Weights.Reset();
        InternalImpulsesNs.Reset();
    }
};

namespace TAStructureImpactDistributor
{
    TA_STRUCTURE_API bool DistributeImpactAsInternalDeformation(
        const FTAStructureImpactConfig& Config,
        const FTAStructureImpactInput& Input,
        TArray<FTAStructureNode>& InOutNodes,
        FTAStructureImpactScratch& InOutScratch,
        FTAStructureImpactOutput& OutOutput);
}
