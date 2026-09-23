#pragma once

#include "CoreMinimal.h"
#include "TAStructureSolver.h"
#include "TAStructureDamageBridge.h"
#include "TASuspensionDamageBinding.h"
#include "TAVehicleDamageRouter.h"

struct TA_VEHICLE_API FTAVehicleStructureCompiledConfig
{
    FTAStructureSolverConfig Solver;

    TArray<FTAStructureNode> InitialNodes;
    TArray<FTADistanceConstraint> Constraints;

    FTAStructureDamageBridgeConfig DamageBridge;
    FTAVehicleDamageRoutingConfig DamageRouting;

    FTADoubleWishboneStructuralBindings FrontLeftSuspensionBindings;
    FTADoubleWishboneStructuralBindings FrontRightSuspensionBindings;

    FTAMultiLinkStructuralBindings RearLeftSuspensionBindings;
    FTAMultiLinkStructuralBindings RearRightSuspensionBindings;

    bool HasStructure() const
    {
        return InitialNodes.Num() > 0;
    }
};
