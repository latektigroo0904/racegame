#include "TAStructureDamageBridge.h"

namespace
{
    bool ValidateMountBinding(
        const FTAStructureMountDamageBinding& Binding,
        const int32 StructureNodeCount)
    {
        if (Binding.NodeIndices.Num() <= 0 ||
            Binding.NodeIndices.Num() != Binding.Weights.Num() ||
            Binding.DisplacementThresholdsM.Num() <= 0)
        {
            return false;
        }

        double WeightSum = 0.0;

        for (int32 Index = 0;
             Index < Binding.NodeIndices.Num();
             ++Index)
        {
            if (Binding.NodeIndices[Index] < 0 ||
                Binding.NodeIndices[Index] >= StructureNodeCount ||
                !FMath::IsFinite(Binding.Weights[Index]) ||
                Binding.Weights[Index] < 0.0)
            {
                return false;
            }

            WeightSum +=
                Binding.Weights[Index];
        }

        if (WeightSum <= UE_DOUBLE_SMALL_NUMBER)
        {
            return false;
        }

        double PreviousThresholdM = 0.0;

        for (const double ThresholdM :
             Binding.DisplacementThresholdsM)
        {
            if (!FMath::IsFinite(ThresholdM) ||
                ThresholdM <= PreviousThresholdM)
            {
                return false;
            }

            PreviousThresholdM =
                ThresholdM;
        }

        return true;
    }

    bool CalculateWeightedMountDisplacement(
        const FTAStructureMountDamageBinding& Binding,
        const TConstArrayView<FTAStructureNode> Nodes,
        FVector3d& OutDisplacementM)
    {
        OutDisplacementM =
            FVector3d::ZeroVector;

        double WeightSum = 0.0;

        for (int32 Index = 0;
             Index < Binding.NodeIndices.Num();
             ++Index)
        {
            const int32 NodeIndex =
                Binding.NodeIndices[Index];

            if (!Nodes.IsValidIndex(NodeIndex))
            {
                return false;
            }

            const double Weight =
                Binding.Weights[Index];

            OutDisplacementM +=
                (Nodes[NodeIndex].PositionM
                    - Nodes[NodeIndex].ReferencePositionM)
                * Weight;

            WeightSum +=
                Weight;
        }

        if (WeightSum <= UE_DOUBLE_SMALL_NUMBER)
        {
            return false;
        }

        OutDisplacementM /=
            WeightSum;

        return true;
    }

    int32 FindHighestCrossedLevel(
        const TArray<double>& ThresholdsM,
        const double MagnitudeM)
    {
        int32 HighestLevel =
            INDEX_NONE;

        for (int32 Index = 0;
             Index < ThresholdsM.Num();
             ++Index)
        {
            if (MagnitudeM >= ThresholdsM[Index])
            {
                HighestLevel =
                    Index;
            }
            else
            {
                break;
            }
        }

        return HighestLevel;
    }

    bool PushSignal(
        FTADamageSignal& Signal,
        FTAStructureDamageBridgeState& InOutState,
        FTADamageEventQueue& InOutQueue)
    {
        Signal.SequenceId =
            InOutState.NextSequenceId;

        if (!InOutQueue.Push(Signal))
        {
            return false;
        }

        ++InOutState.NextSequenceId;
        return true;
    }
}

bool TAStructureDamageBridge::ValidateConfig(
    const FTAStructureDamageBridgeConfig& Config,
    const int32 StructureNodeCount,
    const int32 ConstraintCount)
{
    if (StructureNodeCount < 0 ||
        ConstraintCount < 0)
    {
        return false;
    }

    if (Config.ConstraintTargetComponentIndices.Num() != 0 &&
        Config.ConstraintTargetComponentIndices.Num() != ConstraintCount)
    {
        return false;
    }

    for (const FTAStructureMountDamageBinding& Binding :
         Config.MountBindings)
    {
        if (!ValidateMountBinding(
                Binding,
                StructureNodeCount))
        {
            return false;
        }
    }

    return true;
}

bool TAStructureDamageBridge::InitializeState(
    const FTAStructureDamageBridgeConfig& Config,
    const int32 ConstraintCount,
    FTAStructureDamageBridgeState& OutState)
{
    if (ConstraintCount < 0)
    {
        return false;
    }

    OutState.FractureEventEmitted.SetNumZeroed(
        ConstraintCount);

    OutState.HighestMountDisplacementLevelEmitted.SetNum(
        Config.MountBindings.Num());

    for (int32& Level :
         OutState.HighestMountDisplacementLevelEmitted)
    {
        Level =
            INDEX_NONE;
    }

    OutState.NextSequenceId =
        0;

    return true;
}

bool TAStructureDamageBridge::EmitStructureStateSignals(
    const FTAStructureDamageBridgeConfig& Config,
    const uint64 SimulationTick,
    const uint16 Substep,
    const TConstArrayView<FTAStructureNode> Nodes,
    const TConstArrayView<FTADistanceConstraint> Constraints,
    FTAStructureDamageBridgeState& InOutState,
    FTADamageEventQueue& InOutQueue)
{
    if (!ValidateConfig(
            Config,
            Nodes.Num(),
            Constraints.Num()) ||
        InOutState.FractureEventEmitted.Num()
            != Constraints.Num() ||
        InOutState.HighestMountDisplacementLevelEmitted.Num()
            != Config.MountBindings.Num())
    {
        return false;
    }

    bool bAllPushesSucceeded =
        true;

    for (int32 ConstraintIndex = 0;
         ConstraintIndex < Constraints.Num();
         ++ConstraintIndex)
    {
        const FTADistanceConstraint& Constraint =
            Constraints[ConstraintIndex];

        if (!Constraint.bBroken ||
            InOutState.FractureEventEmitted[ConstraintIndex] != 0)
        {
            continue;
        }

        FTADamageSignal Signal;
        Signal.SimulationTick =
            SimulationTick;
        Signal.Substep =
            Substep;

        Signal.TargetComponentIndex =
            Config.ConstraintTargetComponentIndices.Num() > 0
            ? Config.ConstraintTargetComponentIndices[ConstraintIndex]
            : INDEX_NONE;

        Signal.SourceElementIndex =
            ConstraintIndex;

        Signal.Type =
            ETADamageSignalType::StructuralFracture;

        if (Nodes.IsValidIndex(Constraint.NodeA) &&
            Nodes.IsValidIndex(Constraint.NodeB))
        {
            const FVector3d Delta =
                Nodes[Constraint.NodeB].PositionM
                - Nodes[Constraint.NodeA].PositionM;

            const double CurrentLengthM =
                Delta.Length();

            const double RestLengthM =
                FMath::Max(
                    UE_DOUBLE_SMALL_NUMBER,
                    Constraint.RestLengthM);

            Signal.ScalarValue =
                FMath::Abs(
                    (CurrentLengthM - RestLengthM)
                    / RestLengthM);

            Signal.VectorValue =
                Delta;
        }

        if (PushSignal(
                Signal,
                InOutState,
                InOutQueue))
        {
            InOutState.FractureEventEmitted[ConstraintIndex] =
                1;
        }
        else
        {
            bAllPushesSucceeded =
                false;
        }
    }

    for (int32 BindingIndex = 0;
         BindingIndex < Config.MountBindings.Num();
         ++BindingIndex)
    {
        const FTAStructureMountDamageBinding& Binding =
            Config.MountBindings[BindingIndex];

        FVector3d DisplacementM;

        if (!CalculateWeightedMountDisplacement(
                Binding,
                Nodes,
                DisplacementM))
        {
            bAllPushesSucceeded =
                false;
            continue;
        }

        const double MagnitudeM =
            DisplacementM.Length();

        const int32 HighestCrossedLevel =
            FindHighestCrossedLevel(
                Binding.DisplacementThresholdsM,
                MagnitudeM);

        if (HighestCrossedLevel == INDEX_NONE ||
            HighestCrossedLevel <=
                InOutState.HighestMountDisplacementLevelEmitted[
                    BindingIndex])
        {
            continue;
        }

        FTADamageSignal Signal;
        Signal.SimulationTick =
            SimulationTick;
        Signal.Substep =
            Substep;
        Signal.TargetComponentIndex =
            Binding.TargetComponentIndex;
        Signal.SourceElementIndex =
            BindingIndex;
        Signal.Type =
            ETADamageSignalType::StructuralDisplacement;
        Signal.ScalarValue =
            MagnitudeM;
        Signal.VectorValue =
            DisplacementM;

        if (PushSignal(
                Signal,
                InOutState,
                InOutQueue))
        {
            InOutState.HighestMountDisplacementLevelEmitted[
                BindingIndex] =
                HighestCrossedLevel;
        }
        else
        {
            bAllPushesSucceeded =
                false;
        }
    }

    return bAllPushesSucceeded;
}

bool TAStructureDamageBridge::EmitImpactEnergySignal(
    const FTAStructureDamageBridgeConfig& Config,
    const uint64 SimulationTick,
    const uint16 Substep,
    const FTAStructureImpactOutput& Impact,
    FTAStructureDamageBridgeState& InOutState,
    FTADamageEventQueue& InOutQueue)
{
    if (!Impact.bApplied ||
        Impact.InjectedDeformationKineticEnergyJ <= 0.0)
    {
        return true;
    }

    FTADamageSignal Signal;
    Signal.SimulationTick =
        SimulationTick;
    Signal.Substep =
        Substep;
    Signal.TargetComponentIndex =
        Config.ImpactTargetComponentIndex;
    Signal.SourceElementIndex =
        INDEX_NONE;
    Signal.Type =
        ETADamageSignalType::ImpactEnergy;
    Signal.ScalarValue =
        Impact.InjectedDeformationKineticEnergyJ;

    return PushSignal(
        Signal,
        InOutState,
        InOutQueue);
}
