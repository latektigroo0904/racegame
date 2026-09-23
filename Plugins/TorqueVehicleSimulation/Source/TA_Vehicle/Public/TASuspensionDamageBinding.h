#pragma once

#include "CoreMinimal.h"
#include "TADoubleWishboneSolver.h"
#include "TAMultiLinkSolver.h"
#include "TAStructureSolver.h"

struct TA_VEHICLE_API FTAStructureDisplacementBinding
{
    TArray<int32> NodeIndices;
    TArray<double> Weights;
};

struct TA_VEHICLE_API FTADoubleWishboneStructuralBindings
{
    FTAStructureDisplacementBinding UpperInnerA;
    FTAStructureDisplacementBinding UpperInnerB;

    FTAStructureDisplacementBinding LowerInnerA;
    FTAStructureDisplacementBinding LowerInnerB;

    FTAStructureDisplacementBinding TieRodInner;
    FTAStructureDisplacementBinding DamperChassis;
};

struct TA_VEHICLE_API FTAMultiLinkStructuralBindings
{
    FTAStructureDisplacementBinding ChassisPickups[TARearMultiLinkCount];
    FTAStructureDisplacementBinding DamperChassis;
};

namespace TASuspensionDamageBinding
{
    TA_VEHICLE_API bool ValidateBinding(
        const FTAStructureDisplacementBinding& Binding,
        int32 StructureNodeCount);

    TA_VEHICLE_API bool ResolveBindingDisplacement(
        TConstArrayView<FTAStructureNode> StructureNodes,
        const FTAStructureDisplacementBinding& Binding,
        FVector3d& OutDisplacementM);

    TA_VEHICLE_API bool ResolveDoubleWishboneDamageOffsets(
        TConstArrayView<FTAStructureNode> StructureNodes,
        const FTADoubleWishboneStructuralBindings& Bindings,
        FTADoubleWishboneDamageOffsets& OutOffsets);

    TA_VEHICLE_API bool ResolveMultiLinkDamageOffsets(
        TConstArrayView<FTAStructureNode> StructureNodes,
        const FTAMultiLinkStructuralBindings& Bindings,
        FTAMultiLinkDamageOffsets& OutOffsets);
}
