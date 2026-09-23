#include "TACompiledDamageRuntime.h"

bool TACompiledDamageRuntime::Initialize(
    const FTAVehicleCompiledConfig& CompiledConfig,
    const int32 MaxDamageEvents,
    FTACompiledDamageRuntimeState& OutState)
{
    OutState =
        FTACompiledDamageRuntimeState{};

    if (!CompiledConfig.StructureRuntime.HasStructure() ||
        MaxDamageEvents <= 0)
    {
        return false;
    }

    OutState.StructureNodes =
        CompiledConfig.StructureRuntime.InitialNodes;

    OutState.StructureConstraints =
        CompiledConfig.StructureRuntime.Constraints;

    OutState.ImpactScratch.Initialize(
        OutState.StructureNodes.Num());

    if (!TAStructureDamageBridge::InitializeState(
            CompiledConfig.StructureRuntime.DamageBridge,
            OutState.StructureConstraints.Num(),
            OutState.DamageBridgeState))
    {
        return false;
    }

    OutState.DamageQueue.Initialize(
        MaxDamageEvents);

    OutState.bInitialized =
        true;

    return true;
}

bool TACompiledDamageRuntime::ProcessCrash(
    const FTAVehicleCompiledConfig& CompiledConfig,
    const FTACrashDamagePipelineInput& Input,
    FTAFourWheelRuntimeState& InOutVehicleState,
    FTACompiledDamageRuntimeState& InOutDamageState,
    FTACrashDamagePipelineOutput& OutOutput)
{
    if (!InOutDamageState.bInitialized ||
        !CompiledConfig.StructureRuntime.HasStructure())
    {
        return false;
    }

    FTACrashDamagePipelineConfig PipelineConfig;

    PipelineConfig.CollisionCoupling.StructureImpact =
        CompiledConfig.StructureRuntime.ImpactDistribution;

    PipelineConfig.StructureSolver =
        CompiledConfig.StructureRuntime.Solver;

    PipelineConfig.DamageBridge =
        CompiledConfig.StructureRuntime.DamageBridge;

    PipelineConfig.DamageRouting =
        CompiledConfig.StructureRuntime.DamageRouting;

    return TACrashDamagePipeline::ProcessImpact(
        PipelineConfig,
        CompiledConfig.VehicleRuntime,
        Input,
        InOutVehicleState.Vehicle,
        InOutDamageState.StructureNodes,
        InOutDamageState.StructureConstraints,
        InOutDamageState.ImpactScratch,
        InOutDamageState.DamageBridgeState,
        InOutDamageState.DamageQueue,
        OutOutput);
}

bool TACompiledDamageRuntime::ApplyCurrentStructureToFourWheelInput(
    const FTAVehicleCompiledConfig& CompiledConfig,
    const FTACompiledDamageRuntimeState& DamageState,
    FTAFourWheelStepInput& InOutStepInput)
{
    if (!DamageState.bInitialized ||
        DamageState.StructureNodes.Num()
            != CompiledConfig.StructureRuntime.InitialNodes.Num())
    {
        return false;
    }

    const TConstArrayView<FTAStructureNode> Nodes =
        MakeArrayView(
            DamageState.StructureNodes);

    if (!TASuspensionDamageBinding::ResolveDoubleWishboneDamageOffsets(
            Nodes,
            CompiledConfig.StructureRuntime.FrontLeftSuspensionBindings,
            InOutStepInput.FrontLeftDamage) ||
        !TASuspensionDamageBinding::ResolveDoubleWishboneDamageOffsets(
            Nodes,
            CompiledConfig.StructureRuntime.FrontRightSuspensionBindings,
            InOutStepInput.FrontRightDamage) ||
        !TASuspensionDamageBinding::ResolveMultiLinkDamageOffsets(
            Nodes,
            CompiledConfig.StructureRuntime.RearLeftSuspensionBindings,
            InOutStepInput.RearLeftDamage) ||
        !TASuspensionDamageBinding::ResolveMultiLinkDamageOffsets(
            Nodes,
            CompiledConfig.StructureRuntime.RearRightSuspensionBindings,
            InOutStepInput.RearRightDamage))
    {
        return false;
    }

    return true;
}
