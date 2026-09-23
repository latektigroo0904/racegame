#include "TACollisionStructureCoupling.h"

bool TACollisionStructureCoupling::ApplyCollisionImpact(
    const FTACollisionStructureCouplingConfig& Config,
    const FTAChassisConfig& ChassisConfig,
    const FTACollisionStructureInput& Input,
    FTAChassisState& InOutChassisState,
    TArray<FTAStructureNode>& InOutStructureNodes,
    FTAStructureImpactScratch& InOutStructureScratch,
    FTACollisionStructureOutput& OutOutput)
{
    OutOutput =
        FTACollisionStructureOutput{};

    const FVector3d BeforeLinearVelocity =
        InOutChassisState.LinearVelocityWorldMps;

    const FVector3d BeforeAngularVelocity =
        InOutChassisState.AngularVelocityWorldRadPerSec;

    if (!TAChassisDynamics::ApplyImpulseAtWorldPoint(
            ChassisConfig,
            Input.CollisionImpulseWorldNs,
            Input.ContactPointWorldM,
            InOutChassisState))
    {
        return false;
    }

    OutOutput.bRigidImpulseApplied =
        true;

    OutOutput.ChassisDeltaLinearVelocityMps =
        InOutChassisState.LinearVelocityWorldMps
        - BeforeLinearVelocity;

    OutOutput.ChassisDeltaAngularVelocityRadPerSec =
        InOutChassisState.AngularVelocityWorldRadPerSec
        - BeforeAngularVelocity;

    // Structure nodes live in chassis-local coordinates. The rigid external
    // collision impulse already belongs to chassis motion, so only a
    // momentum-neutral internal deformation mode is injected into the structure.
    const FVector3d ContactPointLocalM =
        InOutChassisState.OrientationWorld.UnrotateVector(
            Input.ContactPointWorldM
            - InOutChassisState.PositionWorldM);

    const FVector3d CollisionImpulseLocalNs =
        InOutChassisState.OrientationWorld.UnrotateVector(
            Input.CollisionImpulseWorldNs);

    FTAStructureImpactInput StructureInput;
    StructureInput.ContactPointLocalM =
        ContactPointLocalM;

    StructureInput.CollisionImpulseLocalNs =
        CollisionImpulseLocalNs;

    if (Input.AvailableImpactEnergyJ > 0.0)
    {
        StructureInput.MaxDeformationEnergyJ =
            Input.AvailableImpactEnergyJ
            * FMath::Clamp(
                Config.StructureImpact.MaxDeformationEnergyFraction01,
                0.0,
                1.0);
    }

    OutOutput.bInternalDeformationApplied =
        TAStructureImpactDistributor::DistributeImpactAsInternalDeformation(
            Config.StructureImpact,
            StructureInput,
            InOutStructureNodes,
            InOutStructureScratch,
            OutOutput.StructureImpact);

    return true;
}
