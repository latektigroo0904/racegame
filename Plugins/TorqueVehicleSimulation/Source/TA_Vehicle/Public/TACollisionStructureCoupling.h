#pragma once

#include "CoreMinimal.h"
#include "TAChassisDynamics.h"
#include "TAStructureImpactDistributor.h"

struct TA_VEHICLE_API FTACollisionStructureCouplingConfig
{
    FTAStructureImpactConfig StructureImpact;
};

struct TA_VEHICLE_API FTACollisionStructureInput
{
    FVector3d ContactPointWorldM = FVector3d::ZeroVector;
    FVector3d CollisionImpulseWorldNs = FVector3d::ZeroVector;
};

struct TA_VEHICLE_API FTACollisionStructureOutput
{
    bool bRigidImpulseApplied = false;
    bool bInternalDeformationApplied = false;

    FVector3d ChassisDeltaLinearVelocityMps = FVector3d::ZeroVector;
    FVector3d ChassisDeltaAngularVelocityRadPerSec = FVector3d::ZeroVector;

    FTAStructureImpactOutput StructureImpact;
};

namespace TACollisionStructureCoupling
{
    TA_VEHICLE_API bool ApplyCollisionImpact(
        const FTACollisionStructureCouplingConfig& Config,
        const FTAChassisConfig& ChassisConfig,
        const FTACollisionStructureInput& Input,
        FTAChassisState& InOutChassisState,
        TArray<FTAStructureNode>& InOutStructureNodes,
        FTAStructureImpactScratch& InOutStructureScratch,
        FTACollisionStructureOutput& OutOutput);
}
