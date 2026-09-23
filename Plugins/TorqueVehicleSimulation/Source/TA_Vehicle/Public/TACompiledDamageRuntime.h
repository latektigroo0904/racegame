#pragma once

#include "CoreMinimal.h"
#include "TACrashDamagePipeline.h"
#include "TAFourWheelVehicleRuntime.h"
#include "TAVehicleRuntime.h"

struct TA_VEHICLE_API FTACompiledDamageRuntimeState
{
    TArray<FTAStructureNode> StructureNodes;
    TArray<FTADistanceConstraint> StructureConstraints;

    FTAStructureImpactScratch ImpactScratch;
    FTAStructureDamageBridgeState DamageBridgeState;
    FTADamageEventQueue DamageQueue;

    bool bInitialized = false;
};

namespace TACompiledDamageRuntime
{
    TA_VEHICLE_API bool Initialize(
        const FTAVehicleCompiledConfig& CompiledConfig,
        int32 MaxDamageEvents,
        FTACompiledDamageRuntimeState& OutState);

    TA_VEHICLE_API bool ProcessCrash(
        const FTAVehicleCompiledConfig& CompiledConfig,
        const FTACrashDamagePipelineInput& Input,
        FTAFourWheelRuntimeState& InOutVehicleState,
        FTACompiledDamageRuntimeState& InOutDamageState,
        FTACrashDamagePipelineOutput& OutOutput);

    TA_VEHICLE_API bool ApplyCurrentStructureToFourWheelInput(
        const FTAVehicleCompiledConfig& CompiledConfig,
        const FTACompiledDamageRuntimeState& DamageState,
        FTAFourWheelStepInput& InOutStepInput);
}
