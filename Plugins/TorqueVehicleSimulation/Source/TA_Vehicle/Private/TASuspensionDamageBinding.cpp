#include "TASuspensionDamageBinding.h"

bool TASuspensionDamageBinding::ValidateBinding(
    const FTAStructureDisplacementBinding& Binding,
    const int32 StructureNodeCount)
{
    if (Binding.NodeIndices.Num() != Binding.Weights.Num())
    {
        return false;
    }

    if (Binding.NodeIndices.Num() == 0)
    {
        return true;
    }

    double WeightSum = 0.0;

    for (int32 Index = 0; Index < Binding.NodeIndices.Num(); ++Index)
    {
        const int32 NodeIndex =
            Binding.NodeIndices[Index];

        const double Weight =
            Binding.Weights[Index];

        if (NodeIndex < 0 ||
            NodeIndex >= StructureNodeCount ||
            !FMath::IsFinite(Weight) ||
            Weight < 0.0)
        {
            return false;
        }

        WeightSum += Weight;
    }

    return WeightSum > UE_DOUBLE_SMALL_NUMBER;
}

bool TASuspensionDamageBinding::ResolveBindingDisplacement(
    const TConstArrayView<FTAStructureNode> StructureNodes,
    const FTAStructureDisplacementBinding& Binding,
    FVector3d& OutDisplacementM)
{
    OutDisplacementM =
        FVector3d::ZeroVector;

    if (!ValidateBinding(
            Binding,
            StructureNodes.Num()))
    {
        return false;
    }

    if (Binding.NodeIndices.Num() == 0)
    {
        return true;
    }

    double WeightSum = 0.0;

    for (int32 Index = 0; Index < Binding.NodeIndices.Num(); ++Index)
    {
        const double Weight =
            Binding.Weights[Index];

        const FTAStructureNode& Node =
            StructureNodes[Binding.NodeIndices[Index]];

        OutDisplacementM +=
            TAStructureSolver::CalculateNodeDisplacement(Node)
            * Weight;

        WeightSum += Weight;
    }

    if (WeightSum <= UE_DOUBLE_SMALL_NUMBER)
    {
        return false;
    }

    OutDisplacementM /= WeightSum;
    return true;
}

bool TASuspensionDamageBinding::ResolveDoubleWishboneDamageOffsets(
    const TConstArrayView<FTAStructureNode> StructureNodes,
    const FTADoubleWishboneStructuralBindings& Bindings,
    FTADoubleWishboneDamageOffsets& OutOffsets)
{
    OutOffsets =
        FTADoubleWishboneDamageOffsets{};

    return
        ResolveBindingDisplacement(
            StructureNodes,
            Bindings.UpperInnerA,
            OutOffsets.UpperInnerA)
        && ResolveBindingDisplacement(
            StructureNodes,
            Bindings.UpperInnerB,
            OutOffsets.UpperInnerB)
        && ResolveBindingDisplacement(
            StructureNodes,
            Bindings.LowerInnerA,
            OutOffsets.LowerInnerA)
        && ResolveBindingDisplacement(
            StructureNodes,
            Bindings.LowerInnerB,
            OutOffsets.LowerInnerB)
        && ResolveBindingDisplacement(
            StructureNodes,
            Bindings.TieRodInner,
            OutOffsets.TieRodInner)
        && ResolveBindingDisplacement(
            StructureNodes,
            Bindings.DamperChassis,
            OutOffsets.DamperChassis);
}
