#include "TACrashDamagePipeline.h"

bool TACrashDamagePipeline::ProcessImpact(
    const FTACrashDamagePipelineConfig& PipelineConfig,
    const FTAVehicleRuntimeConfig& VehicleConfig,
    const FTACrashDamagePipelineInput& Input,
    FTAChassisState& InOutChassisState,
    FTAVehicleRuntimeState& InOutVehicleState,
    TArray<FTAStructureNode>& InOutStructureNodes,
    TArray<FTADistanceConstraint>& InOutConstraints,
    FTAStructureImpactScratch& InOutImpactScratch,
    FTAStructureDamageBridgeState& InOutDamageBridgeState,
    FTADamageEventQueue& InOutDamageQueue,
    FTACrashDamagePipelineOutput& OutOutput)
{
    OutOutput =
        FTACrashDamagePipelineOutput{};

    if (Input.StructureDeltaTimeSeconds <= 0.0 ||
        !TAChassisDynamics::IsConfigValid(
            VehicleConfig.Chassis) ||
        !TAStructureDamageBridge::ValidateConfig(
            PipelineConfig.DamageBridge,
            InOutStructureNodes.Num(),
            InOutConstraints.Num()) ||
        !TAVehicleDamageRouter::ValidateConfig(
            PipelineConfig.DamageRouting))
    {
        return false;
    }

    InOutDamageQueue.ResetKeepCapacity();

    if (!TACollisionStructureCoupling::ApplyCollisionImpact(
            PipelineConfig.CollisionCoupling,
            VehicleConfig.Chassis,
            Input.Collision,
            InOutChassisState,
            InOutStructureNodes,
            InOutImpactScratch,
            OutOutput.Collision))
    {
        return false;
    }

    OutOutput.bCollisionProcessed =
        true;

    OutOutput.bImpactSignalEmitted =
        TAStructureDamageBridge::EmitImpactEnergySignal(
            PipelineConfig.DamageBridge,
            Input.SimulationTick,
            Input.Substep,
            OutOutput.Collision.StructureImpact,
            InOutDamageBridgeState,
            InOutDamageQueue);

    TAStructureSolver::Step(
        PipelineConfig.StructureSolver,
        Input.StructureDeltaTimeSeconds,
        InOutStructureNodes,
        InOutConstraints);

    OutOutput.bStructureSignalsEmitted =
        TAStructureDamageBridge::EmitStructureStateSignals(
            PipelineConfig.DamageBridge,
            Input.SimulationTick,
            Input.Substep,
            MakeArrayView(InOutStructureNodes),
            MakeArrayView(InOutConstraints),
            InOutDamageBridgeState,
            InOutDamageQueue);

    InOutDamageQueue.SortDeterministic();

    OutOutput.EmittedSignalCount =
        InOutDamageQueue.Num();

    OutOutput.bDamageRouted =
        TAVehicleDamageRouter::RouteSignals(
            PipelineConfig.DamageRouting,
            VehicleConfig,
            InOutDamageQueue.GetEvents(),
            InOutVehicleState,
            OutOutput.Routing);

    return
        OutOutput.bImpactSignalEmitted
        && OutOutput.bStructureSignalsEmitted
        && OutOutput.bDamageRouted;
}
