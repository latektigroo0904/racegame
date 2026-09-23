#pragma once

#include "CoreMinimal.h"
#include "TADamageTypes.h"
#include "TAStructureImpactDistributor.h"
#include "TAStructureSolver.h"

struct TA_DAMAGE_API FTAStructureMountDamageBinding
{
    int32 TargetComponentIndex = INDEX_NONE;

    TArray<int32> NodeIndices;
    TArray<double> Weights;

    // Must be authored in ascending order.
    TArray<double> DisplacementThresholdsM;
};

struct TA_DAMAGE_API FTAStructureDamageBridgeConfig
{
    // Optional per-constraint target component. INDEX_NONE means generic structure.
    TArray<int32> ConstraintTargetComponentIndices;

    TArray<FTAStructureMountDamageBinding> MountBindings;

    int32 ImpactTargetComponentIndex = INDEX_NONE;
};

struct TA_DAMAGE_API FTAStructureDamageBridgeState
{
    TArray<uint8> FractureEventEmitted;
    TArray<int32> HighestMountDisplacementLevelEmitted;

    uint32 NextSequenceId = 0;

    void ResetKeepCapacity()
    {
        for (uint8& Value : FractureEventEmitted)
        {
            Value = 0;
        }

        for (int32& Value : HighestMountDisplacementLevelEmitted)
        {
            Value = INDEX_NONE;
        }

        NextSequenceId = 0;
    }
};

namespace TAStructureDamageBridge
{
    TA_DAMAGE_API bool ValidateConfig(
        const FTAStructureDamageBridgeConfig& Config,
        int32 StructureNodeCount,
        int32 ConstraintCount);

    TA_DAMAGE_API bool InitializeState(
        const FTAStructureDamageBridgeConfig& Config,
        int32 ConstraintCount,
        FTAStructureDamageBridgeState& OutState);

    TA_DAMAGE_API bool EmitStructureStateSignals(
        const FTAStructureDamageBridgeConfig& Config,
        uint64 SimulationTick,
        uint16 Substep,
        TConstArrayView<FTAStructureNode> Nodes,
        TConstArrayView<FTADistanceConstraint> Constraints,
        FTAStructureDamageBridgeState& InOutState,
        FTADamageEventQueue& InOutQueue);

    TA_DAMAGE_API bool EmitImpactEnergySignal(
        const FTAStructureDamageBridgeConfig& Config,
        uint64 SimulationTick,
        uint16 Substep,
        const FTAStructureImpactOutput& Impact,
        FTAStructureDamageBridgeState& InOutState,
        FTADamageEventQueue& InOutQueue);
}
