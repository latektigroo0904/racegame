#pragma once

#include "CoreMinimal.h"
#include "TACollisionStructureCoupling.h"
#include "TAStructureDamageBridge.h"
#include "TAVehicleDamageRouter.h"

struct TA_VEHICLE_API FTACrashDamagePipelineConfig
{
    FTACollisionStructureCouplingConfig CollisionCoupling;
    FTAStructureSolverConfig StructureSolver;
    FTAStructureDamageBridgeConfig DamageBridge;
    FTAVehicleDamageRoutingConfig DamageRouting;
};

struct TA_VEHICLE_API FTACrashDamagePipelineInput
{
    uint64 SimulationTick = 0;
    uint16 Substep = 0;

    double StructureDeltaTimeSeconds = 1.0 / 240.0;

    FTACollisionStructureInput Collision;
};

struct TA_VEHICLE_API FTACrashDamagePipelineOutput
{
    bool bCollisionProcessed = false;
    bool bImpactSignalEmitted = false;
    bool bStructureSignalsEmitted = false;
    bool bDamageRouted = false;

    int32 EmittedSignalCount = 0;

    FTACollisionStructureOutput Collision;
    FTAVehicleDamageRoutingOutput Routing;
};

namespace TACrashDamagePipeline
{
    TA_VEHICLE_API bool ProcessImpact(
        const FTACrashDamagePipelineConfig& PipelineConfig,
        const FTAVehicleRuntimeConfig& VehicleConfig,
        const FTACrashDamagePipelineInput& Input,
        FTAVehicleRuntimeState& InOutVehicleState,
        TArray<FTAStructureNode>& InOutStructureNodes,
        TArray<FTADistanceConstraint>& InOutConstraints,
        FTAStructureImpactScratch& InOutImpactScratch,
        FTAStructureDamageBridgeState& InOutDamageBridgeState,
        FTADamageEventQueue& InOutDamageQueue,
        FTACrashDamagePipelineOutput& OutOutput);
}
