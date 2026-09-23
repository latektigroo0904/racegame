#include "TAVehicleDefinition.h"

namespace
{
    void AddValidation(
        FTAValidationResult& Result,
        const ETAValidationSeverity Severity,
        const FName Code,
        const FString& Message)
    {
        FTAValidationMessage Entry;
        Entry.Severity = Severity;
        Entry.Code = Code;
        Entry.Message = Message;
        Result.Messages.Add(MoveTemp(Entry));
    }

    uint32 HashDouble(
        uint32 Seed,
        const double Value)
    {
        return HashCombineFast(
            Seed,
            GetTypeHash(Value));
    }

    uint32 HashBool(
        uint32 Seed,
        const bool Value)
    {
        return HashCombineFast(
            Seed,
            GetTypeHash(Value));
    }

    uint32 HashVector(
        uint32 Seed,
        const FVector3d& Value)
    {
        Seed = HashDouble(Seed, Value.X);
        Seed = HashDouble(Seed, Value.Y);
        Seed = HashDouble(Seed, Value.Z);
        return Seed;
    }

    FVector3d ToComLocal(
        const FVector& VehicleLocalPoint,
        const FVector3d& CenterOfMassVehicleLocalM)
    {
        return
            FVector3d(VehicleLocalPoint)
            - CenterOfMassVehicleLocalM;
    }

    FTASuspensionRuntimeConfig BuildSuspensionConfig(
        const double SpringRateNPerM,
        const double StaticSpringCompressionM,
        const double BumpDampingNsPerM,
        const double ReboundDampingNsPerM,
        const double MinTravelM,
        const double MaxTravelM)
    {
        FTASuspensionRuntimeConfig Config;

        Config.SpringRateNPerM =
            SpringRateNPerM;

        Config.StaticSpringCompressionM =
            StaticSpringCompressionM;

        Config.BumpDampingNsPerM =
            BumpDampingNsPerM;

        Config.ReboundDampingNsPerM =
            ReboundDampingNsPerM;

        Config.DroopStopTravelM =
            MinTravelM;

        Config.BumpStopTravelM =
            MaxTravelM;

        return Config;
    }

    void CompileTire(
        const FTAPrototypeTireDefinition& Authored,
        FTATireRuntimeConfig& Out)
    {
        Out.UnloadedRadiusM =
            Authored.UnloadedRadiusM;

        Out.ReferenceLoadN =
            Authored.ReferenceLoadN;

        Out.ReferencePressureKPa =
            Authored.ReferencePressureKPa;

        Out.DryPeakMu =
            Authored.DryPeakMu;

        Out.NewTreadDepthMm =
            Authored.NewTreadDepthMm;

        Out.RadialStiffnessNPerM =
            Authored.RadialStiffnessNPerM;

        Out.RadialProgressiveStiffnessNPerM2 =
            Authored.RadialProgressiveStiffnessNPerM2;

        Out.RadialDampingNsPerM =
            Authored.RadialDampingNsPerM;

        Out.MaxRadialDeflectionM =
            Authored.MaxRadialDeflectionM;
    }

    uint32 HashTireConfig(
        uint32 Hash,
        const FTATireRuntimeConfig& Tire)
    {
        Hash = HashDouble(Hash, Tire.UnloadedRadiusM);
        Hash = HashDouble(Hash, Tire.ReferenceLoadN);
        Hash = HashDouble(Hash, Tire.DryPeakMu);
        Hash = HashDouble(Hash, Tire.LoadSensitivityExponent);
        Hash = HashDouble(Hash, Tire.LongitudinalStiffnessN);
        Hash = HashDouble(Hash, Tire.CorneringStiffnessNPerRad);
        Hash = HashDouble(Hash, Tire.CamberStiffnessNPerRad);
        Hash = HashDouble(Hash, Tire.SaturationExponent);
        Hash = HashDouble(Hash, Tire.PneumaticTrailM);
        Hash = HashDouble(Hash, Tire.RollingResistanceCoefficient);
        Hash = HashDouble(Hash, Tire.ReferencePressureKPa);
        Hash = HashDouble(Hash, Tire.ReferencePressureTemperatureC);
        Hash = HashDouble(Hash, Tire.RadialStiffnessNPerM);
        Hash = HashDouble(Hash, Tire.RadialProgressiveStiffnessNPerM2);
        Hash = HashDouble(Hash, Tire.RadialDampingNsPerM);
        Hash = HashDouble(Hash, Tire.MaxRadialDeflectionM);
        Hash = HashDouble(Hash, Tire.PressureRadialStiffnessExponent);
        Hash = HashDouble(Hash, Tire.NewTreadDepthMm);
        Hash = HashDouble(Hash, Tire.MinimumTreadDepthMm);
        Hash = HashDouble(Hash, Tire.OptimalSurfaceTemperatureC);
        Hash = HashDouble(Hash, Tire.ColdGripMultiplier);
        Hash = HashDouble(Hash, Tire.HotGripMultiplier);
        Hash = HashDouble(Hash, Tire.HotGripTemperatureC);
        Hash = HashDouble(Hash, Tire.PressureGripSensitivity);
        Hash = HashDouble(Hash, Tire.WearGripLossAtEnd);
        Hash = HashDouble(Hash, Tire.HydroReferenceOnsetSpeedMps);
        Hash = HashDouble(Hash, Tire.HydroReferenceWaterDepthMm);
        return Hash;
    }

    uint32 HashSuspensionConfig(
        uint32 Hash,
        const FTASuspensionRuntimeConfig& Suspension)
    {
        Hash = HashDouble(Hash, Suspension.SpringRateNPerM);
        Hash = HashDouble(Hash, Suspension.StaticSpringCompressionM);
        Hash = HashDouble(Hash, Suspension.BumpDampingNsPerM);
        Hash = HashDouble(Hash, Suspension.ReboundDampingNsPerM);
        Hash = HashDouble(Hash, Suspension.BumpStopTravelM);
        Hash = HashDouble(Hash, Suspension.DroopStopTravelM);
        Hash = HashDouble(Hash, Suspension.BumpStopRateNPerM);
        Hash = HashDouble(Hash, Suspension.DroopStopRateNPerM);
        return Hash;
    }

    uint32 HashFrontGeometry(
        uint32 Hash,
        const FTADoubleWishboneSolverConfig& Config)
    {
        const FTADoubleWishboneHardpoints& H =
            Config.Hardpoints;

        Hash = HashVector(Hash, H.UpperInnerA);
        Hash = HashVector(Hash, H.UpperInnerB);
        Hash = HashVector(Hash, H.LowerInnerA);
        Hash = HashVector(Hash, H.LowerInnerB);
        Hash = HashVector(Hash, H.TieRodInner);
        Hash = HashVector(Hash, H.DamperChassis);
        Hash = HashVector(Hash, H.DamperLowerArmReference);
        Hash = HashVector(Hash, H.UpperBallJointReference);
        Hash = HashVector(Hash, H.LowerBallJointReference);
        Hash = HashVector(Hash, H.TieRodOuterReference);
        Hash = HashVector(Hash, H.WheelCenterReference);
        Hash = HashDouble(Hash, Config.MinTravelM);
        Hash = HashDouble(Hash, Config.MaxTravelM);
        Hash = HashCombineFast(Hash, GetTypeHash(Config.MaxIterations));
        Hash = HashDouble(Hash, Config.PositionToleranceM);
        return Hash;
    }

    uint32 HashRearGeometry(
        uint32 Hash,
        const FTAMultiLinkSolverConfig& Config)
    {
        for (int32 Index = 0;
             Index < TARearMultiLinkCount;
             ++Index)
        {
            Hash = HashVector(
                Hash,
                Config.Links[Index].ChassisPickupReference);

            Hash = HashVector(
                Hash,
                Config.Links[Index].UprightPickupReference);
        }

        Hash = HashVector(
            Hash,
            Config.WheelCenterReference);

        Hash = HashVector(
            Hash,
            Config.DamperChassisReference);

        Hash = HashVector(
            Hash,
            Config.DamperUprightReference);

        Hash = HashDouble(Hash, Config.MinTravelM);
        Hash = HashDouble(Hash, Config.MaxTravelM);
        Hash = HashCombineFast(Hash, GetTypeHash(Config.MaxIterations));
        Hash = HashDouble(Hash, Config.PositionToleranceM);

        return Hash;
    }
    void CompileDisplacementBinding(
        const FTAStructureDisplacementBindingAuthoringDefinition& Authored,
        FTAStructureDisplacementBinding& Out)
    {
        Out.NodeIndices =
            Authored.NodeIndices;

        Out.Weights =
            Authored.Weights;
    }

    bool HasBindingData(
        const FTAStructureDisplacementBindingAuthoringDefinition& Binding)
    {
        return
            Binding.NodeIndices.Num() > 0
            || Binding.Weights.Num() > 0;
    }

    bool HasAnyStructureBindingData(
        const FTAVehicleStructureAuthoringDefinition& Structure)
    {
        const FTADoubleWishboneStructuralBindingsAuthoringDefinition* FrontBindings[2] =
        {
            &Structure.FrontLeftSuspensionBindings,
            &Structure.FrontRightSuspensionBindings
        };

        for (const FTADoubleWishboneStructuralBindingsAuthoringDefinition* Front :
             FrontBindings)
        {
            if (HasBindingData(Front->UpperInnerA) ||
                HasBindingData(Front->UpperInnerB) ||
                HasBindingData(Front->LowerInnerA) ||
                HasBindingData(Front->LowerInnerB) ||
                HasBindingData(Front->TieRodInner) ||
                HasBindingData(Front->DamperChassis))
            {
                return true;
            }
        }

        const FTAMultiLinkStructuralBindingsAuthoringDefinition* RearBindings[2] =
        {
            &Structure.RearLeftSuspensionBindings,
            &Structure.RearRightSuspensionBindings
        };

        for (const FTAMultiLinkStructuralBindingsAuthoringDefinition* Rear :
             RearBindings)
        {
            if (HasBindingData(Rear->Link0) ||
                HasBindingData(Rear->Link1) ||
                HasBindingData(Rear->Link2) ||
                HasBindingData(Rear->Link3) ||
                HasBindingData(Rear->Link4) ||
                HasBindingData(Rear->DamperChassis))
            {
                return true;
            }
        }

        return false;
    }

    void CompileFrontStructuralBindings(
        const FTADoubleWishboneStructuralBindingsAuthoringDefinition& Authored,
        FTADoubleWishboneStructuralBindings& Out)
    {
        CompileDisplacementBinding(
            Authored.UpperInnerA,
            Out.UpperInnerA);

        CompileDisplacementBinding(
            Authored.UpperInnerB,
            Out.UpperInnerB);

        CompileDisplacementBinding(
            Authored.LowerInnerA,
            Out.LowerInnerA);

        CompileDisplacementBinding(
            Authored.LowerInnerB,
            Out.LowerInnerB);

        CompileDisplacementBinding(
            Authored.TieRodInner,
            Out.TieRodInner);

        CompileDisplacementBinding(
            Authored.DamperChassis,
            Out.DamperChassis);
    }

    void CompileRearStructuralBindings(
        const FTAMultiLinkStructuralBindingsAuthoringDefinition& Authored,
        FTAMultiLinkStructuralBindings& Out)
    {
        const FTAStructureDisplacementBindingAuthoringDefinition* AuthoredLinks[TARearMultiLinkCount] =
        {
            &Authored.Link0,
            &Authored.Link1,
            &Authored.Link2,
            &Authored.Link3,
            &Authored.Link4
        };

        for (int32 Index = 0;
             Index < TARearMultiLinkCount;
             ++Index)
        {
            CompileDisplacementBinding(
                *AuthoredLinks[Index],
                Out.ChassisPickups[Index]);
        }

        CompileDisplacementBinding(
            Authored.DamperChassis,
            Out.DamperChassis);
    }

    bool ValidateFrontStructuralBindings(
        const FTADoubleWishboneStructuralBindings& Bindings,
        const int32 NodeCount)
    {
        return
            TASuspensionDamageBinding::ValidateBinding(
                Bindings.UpperInnerA,
                NodeCount)
            && TASuspensionDamageBinding::ValidateBinding(
                Bindings.UpperInnerB,
                NodeCount)
            && TASuspensionDamageBinding::ValidateBinding(
                Bindings.LowerInnerA,
                NodeCount)
            && TASuspensionDamageBinding::ValidateBinding(
                Bindings.LowerInnerB,
                NodeCount)
            && TASuspensionDamageBinding::ValidateBinding(
                Bindings.TieRodInner,
                NodeCount)
            && TASuspensionDamageBinding::ValidateBinding(
                Bindings.DamperChassis,
                NodeCount);
    }

    bool ValidateRearStructuralBindings(
        const FTAMultiLinkStructuralBindings& Bindings,
        const int32 NodeCount)
    {
        for (int32 Index = 0;
             Index < TARearMultiLinkCount;
             ++Index)
        {
            if (!TASuspensionDamageBinding::ValidateBinding(
                    Bindings.ChassisPickups[Index],
                    NodeCount))
            {
                return false;
            }
        }

        return TASuspensionDamageBinding::ValidateBinding(
            Bindings.DamperChassis,
            NodeCount);
    }

    ETAVehicleDamageConsumerType CompileDamageConsumer(
        const ETAVehicleDamageConsumerAuthoringType Consumer)
    {
        switch (Consumer)
        {
        case ETAVehicleDamageConsumerAuthoringType::Radiator:
        default:
            return ETAVehicleDamageConsumerType::Radiator;
        }
    }

    bool CompileStructureRuntime(
        const FTAVehicleStructureAuthoringDefinition& Authored,
        const FVector3d& CenterOfMassVehicleLocalM,
        FTAVehicleStructureCompiledConfig& Out,
        FTAValidationResult& OutValidation)
    {
        Out =
            FTAVehicleStructureCompiledConfig{};

        const bool bHasDependentData =
            Authored.Constraints.Num() > 0
            || Authored.MountDamageBindings.Num() > 0
            || Authored.DamageRoutes.Num() > 0
            || HasAnyStructureBindingData(Authored);

        if (Authored.Nodes.Num() == 0)
        {
            if (bHasDependentData)
            {
                AddValidation(
                    OutValidation,
                    ETAValidationSeverity::Error,
                    TEXT("Vehicle.StructureMissingNodes"),
                    TEXT("Structural constraints, routes or bindings require authored structural nodes."));

                return false;
            }

            return true;
        }

        Out.Solver.ConstraintIterations =
            Authored.ConstraintIterations;

        Out.Solver.MaxPositionCorrectionM =
            Authored.MaxPositionCorrectionM;

        Out.Solver.MaxPlasticRestChangeFractionPerStep =
            Authored.MaxPlasticRestChangeFractionPerStep;

        Out.ImpactDistribution.DistributionRadiusM =
            Authored.ImpactDistributionRadiusM;

        Out.ImpactDistribution.DistanceFalloffExponent =
            Authored.ImpactDistanceFalloffExponent;

        Out.ImpactDistribution.DeformationImpulseFraction01 =
            Authored.DeformationImpulseFraction01;

        Out.ImpactDistribution.MaxNodeDeltaVelocityMps =
            Authored.MaxNodeDeltaVelocityMps;

        Out.InitialNodes.SetNum(
            Authored.Nodes.Num());

        for (int32 NodeIndex = 0;
             NodeIndex < Authored.Nodes.Num();
             ++NodeIndex)
        {
            const FTAStructureNodeAuthoringDefinition& Source =
                Authored.Nodes[NodeIndex];

            if (Source.MassKg <= 0.0)
            {
                AddValidation(
                    OutValidation,
                    ETAValidationSeverity::Error,
                    TEXT("Vehicle.StructureInvalidNodeMass"),
                    FString::Printf(
                        TEXT("Structural node %d has non-positive mass."),
                        NodeIndex));

                continue;
            }

            FTAStructureNode& Node =
                Out.InitialNodes[NodeIndex];

            Node.PositionM =
                ToComLocal(
                    Source.PositionVehicleLocalM,
                    CenterOfMassVehicleLocalM);

            Node.ReferencePositionM =
                Node.PositionM;

            Node.PreviousPositionM =
                Node.PositionM;

            Node.VelocityMps =
                FVector3d::ZeroVector;

            Node.bPinned =
                Source.bPinned;

            Node.InverseMassPerKg =
                Source.bPinned
                ? 0.0
                : 1.0 / Source.MassKg;
        }

        Out.Constraints.SetNum(
            Authored.Constraints.Num());

        Out.DamageBridge.ConstraintTargetComponentIndices.SetNum(
            Authored.Constraints.Num());

        for (int32 ConstraintIndex = 0;
             ConstraintIndex < Authored.Constraints.Num();
             ++ConstraintIndex)
        {
            const FTAStructureConstraintAuthoringDefinition& Source =
                Authored.Constraints[ConstraintIndex];

            if (!Out.InitialNodes.IsValidIndex(Source.NodeA) ||
                !Out.InitialNodes.IsValidIndex(Source.NodeB) ||
                Source.NodeA == Source.NodeB)
            {
                AddValidation(
                    OutValidation,
                    ETAValidationSeverity::Error,
                    TEXT("Vehicle.StructureInvalidConstraintNodes"),
                    FString::Printf(
                        TEXT("Structural constraint %d references invalid or identical nodes."),
                        ConstraintIndex));

                continue;
            }

            FTADistanceConstraint& Constraint =
                Out.Constraints[ConstraintIndex];

            Constraint.NodeA =
                Source.NodeA;

            Constraint.NodeB =
                Source.NodeB;

            Constraint.RestLengthM =
                (Out.InitialNodes[Source.NodeB].PositionM
                    - Out.InitialNodes[Source.NodeA].PositionM).Length();

            if (Constraint.RestLengthM <= UE_DOUBLE_SMALL_NUMBER)
            {
                AddValidation(
                    OutValidation,
                    ETAValidationSeverity::Error,
                    TEXT("Vehicle.StructureZeroLengthConstraint"),
                    FString::Printf(
                        TEXT("Structural constraint %d has zero rest length."),
                        ConstraintIndex));
            }

            Constraint.Compliance =
                FMath::Max(
                    0.0,
                    Source.Compliance);

            Constraint.YieldStrain =
                FMath::Max(
                    0.0,
                    Source.YieldStrain);

            Constraint.FractureStrain =
                FMath::Max(
                    Constraint.YieldStrain,
                    Source.FractureStrain);

            Constraint.PlasticFlowRate01 =
                FMath::Clamp(
                    Source.PlasticFlowRate01,
                    0.0,
                    1.0);

            Out.DamageBridge.ConstraintTargetComponentIndices[ConstraintIndex] =
                Source.TargetComponentIndex;
        }

        Out.DamageBridge.ImpactTargetComponentIndex =
            Authored.ImpactTargetComponentIndex;

        for (const FTAStructureMountDamageAuthoringDefinition& Source :
             Authored.MountDamageBindings)
        {
            FTAStructureMountDamageBinding Binding;

            Binding.TargetComponentIndex =
                Source.TargetComponentIndex;

            Binding.NodeIndices =
                Source.NodeIndices;

            Binding.Weights =
                Source.Weights;

            Binding.DisplacementThresholdsM =
                Source.DisplacementThresholdsM;

            Out.DamageBridge.MountBindings.Add(
                MoveTemp(Binding));
        }

        for (const FTAVehicleDamageRouteAuthoringDefinition& Source :
             Authored.DamageRoutes)
        {
            FTAVehicleDamageRoute Route;

            Route.TargetComponentIndex =
                Source.TargetComponentIndex;

            Route.Consumer =
                CompileDamageConsumer(
                    Source.Consumer);

            Route.bAcceptImpactEnergy =
                Source.bAcceptImpactEnergy;

            Route.bAcceptStructuralDisplacement =
                Source.bAcceptStructuralDisplacement;

            Route.bAcceptStructuralFracture =
                Source.bAcceptStructuralFracture;

            Route.ImpactEnergyScale =
                Source.ImpactEnergyScale;

            Route.FullCrushDisplacementM =
                Source.FullCrushDisplacementM;

            Out.DamageRouting.Routes.Add(
                Route);
        }

        CompileFrontStructuralBindings(
            Authored.FrontLeftSuspensionBindings,
            Out.FrontLeftSuspensionBindings);

        CompileFrontStructuralBindings(
            Authored.FrontRightSuspensionBindings,
            Out.FrontRightSuspensionBindings);

        CompileRearStructuralBindings(
            Authored.RearLeftSuspensionBindings,
            Out.RearLeftSuspensionBindings);

        CompileRearStructuralBindings(
            Authored.RearRightSuspensionBindings,
            Out.RearRightSuspensionBindings);

        const int32 NodeCount =
            Out.InitialNodes.Num();

        const bool bBindingsValid =
            ValidateFrontStructuralBindings(
                Out.FrontLeftSuspensionBindings,
                NodeCount)
            && ValidateFrontStructuralBindings(
                Out.FrontRightSuspensionBindings,
                NodeCount)
            && ValidateRearStructuralBindings(
                Out.RearLeftSuspensionBindings,
                NodeCount)
            && ValidateRearStructuralBindings(
                Out.RearRightSuspensionBindings,
                NodeCount);

        if (!bBindingsValid)
        {
            AddValidation(
                OutValidation,
                ETAValidationSeverity::Error,
                TEXT("Vehicle.StructureInvalidSuspensionBinding"),
                TEXT("One or more structural suspension bindings are invalid."));
        }

        if (!TAStructureDamageBridge::ValidateConfig(
                Out.DamageBridge,
                Out.InitialNodes.Num(),
                Out.Constraints.Num()))
        {
            AddValidation(
                OutValidation,
                ETAValidationSeverity::Error,
                TEXT("Vehicle.StructureInvalidDamageBridge"),
                TEXT("Structural damage-event bindings are invalid."));
        }

        if (!TAVehicleDamageRouter::ValidateConfig(
                Out.DamageRouting))
        {
            AddValidation(
                OutValidation,
                ETAValidationSeverity::Error,
                TEXT("Vehicle.StructureInvalidDamageRouting"),
                TEXT("Vehicle damage routing contains invalid or duplicate component targets."));
        }

        return !OutValidation.HasErrors();
    }

    uint32 HashStructureRuntime(
        uint32 Hash,
        const FTAVehicleStructureCompiledConfig& Structure)
    {
        Hash = HashCombineFast(
            Hash,
            GetTypeHash(Structure.InitialNodes.Num()));

        Hash = HashCombineFast(
            Hash,
            GetTypeHash(Structure.Constraints.Num()));

        Hash = HashDouble(
            Hash,
            Structure.ImpactDistribution.DistributionRadiusM);

        Hash = HashDouble(
            Hash,
            Structure.ImpactDistribution.DistanceFalloffExponent);

        Hash = HashDouble(
            Hash,
            Structure.ImpactDistribution.DeformationImpulseFraction01);

        Hash = HashDouble(
            Hash,
            Structure.ImpactDistribution.MaxNodeDeltaVelocityMps);

        for (const FTAStructureNode& Node :
             Structure.InitialNodes)
        {
            Hash = HashVector(
                Hash,
                Node.ReferencePositionM);

            Hash = HashDouble(
                Hash,
                Node.InverseMassPerKg);

            Hash = HashBool(
                Hash,
                Node.bPinned);
        }

        for (const FTADistanceConstraint& Constraint :
             Structure.Constraints)
        {
            Hash = HashCombineFast(
                Hash,
                GetTypeHash(Constraint.NodeA));

            Hash = HashCombineFast(
                Hash,
                GetTypeHash(Constraint.NodeB));

            Hash = HashDouble(
                Hash,
                Constraint.RestLengthM);

            Hash = HashDouble(
                Hash,
                Constraint.Compliance);

            Hash = HashDouble(
                Hash,
                Constraint.YieldStrain);

            Hash = HashDouble(
                Hash,
                Constraint.FractureStrain);

            Hash = HashDouble(
                Hash,
                Constraint.PlasticFlowRate01);
        }

        for (const FTAVehicleDamageRoute& Route :
             Structure.DamageRouting.Routes)
        {
            Hash = HashCombineFast(
                Hash,
                GetTypeHash(Route.TargetComponentIndex));

            Hash = HashCombineFast(
                Hash,
                GetTypeHash(
                    static_cast<uint8>(Route.Consumer)));

            Hash = HashDouble(
                Hash,
                Route.ImpactEnergyScale);

            Hash = HashDouble(
                Hash,
                Route.FullCrushDisplacementM);
        }

        return Hash;
    }

}

FTARearSuspensionDefinition::FTARearSuspensionDefinition()
{
    Link0.ChassisPickup =
        FVector(-1.15, 0.35, -0.18);
    Link0.UprightPickup =
        FVector(-1.20, 0.68, -0.24);

    Link1.ChassisPickup =
        FVector(-1.50, 0.36, -0.20);
    Link1.UprightPickup =
        FVector(-1.42, 0.69, -0.25);

    Link2.ChassisPickup =
        FVector(-1.10, 0.34, -0.46);
    Link2.UprightPickup =
        FVector(-1.18, 0.72, -0.49);

    Link3.ChassisPickup =
        FVector(-1.53, 0.33, -0.47);
    Link3.UprightPickup =
        FVector(-1.45, 0.73, -0.50);

    Link4.ChassisPickup =
        FVector(-1.50, 0.38, -0.36);
    Link4.UprightPickup =
        FVector(-1.48, 0.71, -0.38);
}

bool UTAVehicleDefinition::BuildCompiledConfig(
    FTAVehicleCompiledConfig& OutConfig,
    FTAValidationResult& OutValidation) const
{
    OutConfig =
        FTAVehicleCompiledConfig{};

    OutValidation.Messages.Reset();

    if (DefinitionId.IsNone())
    {
        AddValidation(
            OutValidation,
            ETAValidationSeverity::Error,
            TEXT("Vehicle.MissingDefinitionId"),
            TEXT("Vehicle DefinitionId must not be None."));
    }

    if (Mass.ReferenceMassKg <= 0.0)
    {
        AddValidation(
            OutValidation,
            ETAValidationSeverity::Error,
            TEXT("Vehicle.InvalidMass"),
            TEXT("Reference mass must be greater than zero."));
    }

    if (Mass.PrincipalInertiaKgm2.X <= 0.0 ||
        Mass.PrincipalInertiaKgm2.Y <= 0.0 ||
        Mass.PrincipalInertiaKgm2.Z <= 0.0)
    {
        AddValidation(
            OutValidation,
            ETAValidationSeverity::Error,
            TEXT("Vehicle.InvalidInertia"),
            TEXT("All principal inertia values must be greater than zero."));
    }

    if (Dimensions.WheelbaseMeters <= 0.0 ||
        Dimensions.TrackFrontMeters <= 0.0 ||
        Dimensions.TrackRearMeters <= 0.0)
    {
        AddValidation(
            OutValidation,
            ETAValidationSeverity::Error,
            TEXT("Vehicle.InvalidGeometry"),
            TEXT("Wheelbase and track widths must be greater than zero."));
    }

    if (WheelCount != 4)
    {
        AddValidation(
            OutValidation,
            ETAValidationSeverity::Error,
            TEXT("Vehicle.PrototypeRequiresFourWheels"),
            TEXT("The current high-fidelity compiled runtime requires exactly four wheels."));
    }

    if (Tire.UnloadedRadiusM <= 0.0 ||
        Tire.ReferenceLoadN <= 0.0 ||
        Tire.RadialStiffnessNPerM <= 0.0 ||
        Tire.MaxRadialDeflectionM <= 0.0)
    {
        AddValidation(
            OutValidation,
            ETAValidationSeverity::Error,
            TEXT("Vehicle.InvalidTire"),
            TEXT("Tire radius, reference load, radial stiffness and max radial deflection must be positive."));
    }

    if (FrontSuspension.MaxTravelM <=
            FrontSuspension.MinTravelM ||
        RearSuspension.MaxTravelM <=
            RearSuspension.MinTravelM)
    {
        AddValidation(
            OutValidation,
            ETAValidationSeverity::Error,
            TEXT("Vehicle.InvalidSuspensionTravel"),
            TEXT("Suspension maximum travel must be greater than minimum travel."));
    }

    if (Drivetrain.ForwardGearRatios.Num() <= 0 ||
        Drivetrain.FinalDriveRatio <= 0.0 ||
        Drivetrain.MechanicalEfficiency <= 0.0)
    {
        AddValidation(
            OutValidation,
            ETAValidationSeverity::Error,
            TEXT("Vehicle.InvalidDrivetrain"),
            TEXT("Drivetrain requires forward gears, a positive final drive and positive efficiency."));
    }

    for (const double Ratio :
         Drivetrain.ForwardGearRatios)
    {
        if (Ratio <= 0.0)
        {
            AddValidation(
                OutValidation,
                ETAValidationSeverity::Error,
                TEXT("Vehicle.InvalidForwardGear"),
                TEXT("All forward gear ratios must be positive."));
            break;
        }
    }

    if (OutValidation.HasErrors())
    {
        return false;
    }

    OutConfig.Version =
        Version;

    OutConfig.DefinitionId =
        DefinitionId;

    OutConfig.MassKg =
        Mass.ReferenceMassKg;

    OutConfig.CenterOfMassMeters =
        FVector3d(Mass.CenterOfMassMeters);

    OutConfig.PrincipalInertiaKgm2 =
        FVector3d(Mass.PrincipalInertiaKgm2);

    OutConfig.LengthMeters =
        Dimensions.LengthMeters;

    OutConfig.WidthMeters =
        Dimensions.WidthMeters;

    OutConfig.HeightMeters =
        Dimensions.HeightMeters;

    OutConfig.WheelbaseMeters =
        Dimensions.WheelbaseMeters;

    OutConfig.TrackFrontMeters =
        Dimensions.TrackFrontMeters;

    OutConfig.TrackRearMeters =
        Dimensions.TrackRearMeters;

    OutConfig.WheelCount =
        WheelCount;

    FTAVehicleRuntimeConfig& VehicleRuntime =
        OutConfig.VehicleRuntime;

    VehicleRuntime.ReferenceMassKg =
        Mass.ReferenceMassKg;

    VehicleRuntime.Chassis.MassKg =
        Mass.ReferenceMassKg;

    VehicleRuntime.Chassis.PrincipalInertiaBodyKgm2 =
        FVector3d(Mass.PrincipalInertiaKgm2);

    VehicleRuntime.Wheels.SetNum(4);
    VehicleRuntime.Tires.SetNum(4);

    FTATireRuntimeConfig TireRuntime;
    CompileTire(
        Tire,
        TireRuntime);

    for (int32 Index = 0;
         Index < 4;
         ++Index)
    {
        VehicleRuntime.Tires[Index] =
            TireRuntime;

        VehicleRuntime.Wheels[Index].RadiusM =
            TireRuntime.UnloadedRadiusM;

        VehicleRuntime.Wheels[Index].InertiaKgm2 =
            Wheel.InertiaKgm2;

        VehicleRuntime.Wheels[Index].MaxBrakeTorqueNm =
            Wheel.MaxBrakeTorqueNm;
    }

    const bool bRearDriven =
        Drivetrain.bRearWheelDrive;

    VehicleRuntime.Wheels[
        static_cast<int32>(
            ETAPrototypeWheelIndex::FrontLeft)].bDriven =
        !bRearDriven;

    VehicleRuntime.Wheels[
        static_cast<int32>(
            ETAPrototypeWheelIndex::FrontRight)].bDriven =
        !bRearDriven;

    VehicleRuntime.Wheels[
        static_cast<int32>(
            ETAPrototypeWheelIndex::RearLeft)].bDriven =
        bRearDriven;

    VehicleRuntime.Wheels[
        static_cast<int32>(
            ETAPrototypeWheelIndex::RearRight)].bDriven =
        bRearDriven;

    VehicleRuntime.Engine.IdleRPM =
        Drivetrain.IdleRPM;

    VehicleRuntime.Engine.RedlineRPM =
        Drivetrain.RedlineRPM;

    VehicleRuntime.Engine.LimiterRPM =
        Drivetrain.LimiterRPM;

    VehicleRuntime.Engine.CrankInertiaKgm2 =
        Drivetrain.CrankInertiaKgm2;

    VehicleRuntime.Clutch.MaxTorqueCapacityNm =
        Drivetrain.ClutchMaxTorqueNm;

    VehicleRuntime.Gearbox.ForwardGearRatios =
        Drivetrain.ForwardGearRatios;

    VehicleRuntime.Gearbox.ReverseGearRatio =
        Drivetrain.ReverseGearRatio;

    VehicleRuntime.Gearbox.FinalDriveRatio =
        Drivetrain.FinalDriveRatio;

    VehicleRuntime.Gearbox.MechanicalEfficiency =
        Drivetrain.MechanicalEfficiency;

    const FVector3d CenterOfMassVehicleLocalM =
        OutConfig.CenterOfMassMeters;

    FTAFourWheelRuntimeConfig& FourWheel =
        OutConfig.FourWheelRuntime;

    FTADoubleWishboneSolverConfig& FrontGeometry =
        FourWheel.FrontAxle.RightGeometry;

    FTADoubleWishboneHardpoints& FrontHardpoints =
        FrontGeometry.Hardpoints;

    FrontHardpoints.UpperInnerA =
        ToComLocal(
            FrontSuspension.UpperInnerA,
            CenterOfMassVehicleLocalM);

    FrontHardpoints.UpperInnerB =
        ToComLocal(
            FrontSuspension.UpperInnerB,
            CenterOfMassVehicleLocalM);

    FrontHardpoints.LowerInnerA =
        ToComLocal(
            FrontSuspension.LowerInnerA,
            CenterOfMassVehicleLocalM);

    FrontHardpoints.LowerInnerB =
        ToComLocal(
            FrontSuspension.LowerInnerB,
            CenterOfMassVehicleLocalM);

    FrontHardpoints.TieRodInner =
        ToComLocal(
            FrontSuspension.TieRodInner,
            CenterOfMassVehicleLocalM);

    FrontHardpoints.DamperChassis =
        ToComLocal(
            FrontSuspension.DamperChassis,
            CenterOfMassVehicleLocalM);

    FrontHardpoints.DamperLowerArmReference =
        ToComLocal(
            FrontSuspension.DamperLowerArm,
            CenterOfMassVehicleLocalM);

    FrontHardpoints.UpperBallJointReference =
        ToComLocal(
            FrontSuspension.UpperBallJoint,
            CenterOfMassVehicleLocalM);

    FrontHardpoints.LowerBallJointReference =
        ToComLocal(
            FrontSuspension.LowerBallJoint,
            CenterOfMassVehicleLocalM);

    FrontHardpoints.TieRodOuterReference =
        ToComLocal(
            FrontSuspension.TieRodOuter,
            CenterOfMassVehicleLocalM);

    FrontHardpoints.WheelCenterReference =
        ToComLocal(
            FrontSuspension.WheelCenter,
            CenterOfMassVehicleLocalM);

    FrontHardpoints.WheelForwardReference =
        FVector3d(1.0, 0.0, 0.0);

    FrontHardpoints.WheelUpReference =
        FVector3d(0.0, 0.0, 1.0);

    FrontHardpoints.SteeringRackAxisLocal =
        FVector3d(0.0, 1.0, 0.0);

    FrontHardpoints.SideSign = 1.0;

    FrontGeometry.MinTravelM =
        FrontSuspension.MinTravelM;

    FrontGeometry.MaxTravelM =
        FrontSuspension.MaxTravelM;

    FrontGeometry.MaxIterations =
        FrontSuspension.MaxSolverIterations;

    FrontGeometry.PositionToleranceM =
        FrontSuspension.PositionToleranceM;

    FourWheel.FrontAxle.LeftSuspension =
        BuildSuspensionConfig(
            FrontSuspension.SpringRateNPerM,
            FrontSuspension.StaticSpringCompressionM,
            FrontSuspension.BumpDampingNsPerM,
            FrontSuspension.ReboundDampingNsPerM,
            FrontSuspension.MinTravelM,
            FrontSuspension.MaxTravelM);

    FourWheel.FrontAxle.RightSuspension =
        FourWheel.FrontAxle.LeftSuspension;

    FourWheel.FrontAxle.AntiRollBar.CouplingRateNPerM =
        FrontSuspension.AntiRollCouplingRateNPerM;

    FourWheel.FrontAxle.AntiRollBar.MaxTransferForceN =
        FrontSuspension.AntiRollMaxTransferForceN;

    FourWheel.FrontAxle.SteeringRack.MaxRackDisplacementM =
        FrontSuspension.MaxRackDisplacementM;

    FourWheel.FrontAxle.SteeringRack.InputExponent =
        FrontSuspension.SteeringInputExponent;

    FourWheel.FrontAxle.SteeringRack.SteeringSign =
        FrontSuspension.SteeringSign;

    FTAMultiLinkSolverConfig& RearGeometry =
        FourWheel.RearAxle.RightGeometry;

    const FTARearLinkDefinition* RearLinks[TARearMultiLinkCount] =
    {
        &RearSuspension.Link0,
        &RearSuspension.Link1,
        &RearSuspension.Link2,
        &RearSuspension.Link3,
        &RearSuspension.Link4
    };

    for (int32 Index = 0;
         Index < TARearMultiLinkCount;
         ++Index)
    {
        RearGeometry.Links[Index].ChassisPickupReference =
            ToComLocal(
                RearLinks[Index]->ChassisPickup,
                CenterOfMassVehicleLocalM);

        RearGeometry.Links[Index].UprightPickupReference =
            ToComLocal(
                RearLinks[Index]->UprightPickup,
                CenterOfMassVehicleLocalM);
    }

    RearGeometry.WheelCenterReference =
        ToComLocal(
            RearSuspension.WheelCenter,
            CenterOfMassVehicleLocalM);

    RearGeometry.DamperChassisReference =
        ToComLocal(
            RearSuspension.DamperChassis,
            CenterOfMassVehicleLocalM);

    RearGeometry.DamperUprightReference =
        ToComLocal(
            RearSuspension.DamperUpright,
            CenterOfMassVehicleLocalM);

    RearGeometry.WheelForwardReference =
        FVector3d(1.0, 0.0, 0.0);

    RearGeometry.WheelUpReference =
        FVector3d(0.0, 0.0, 1.0);

    RearGeometry.SideSign = 1.0;

    RearGeometry.MinTravelM =
        RearSuspension.MinTravelM;

    RearGeometry.MaxTravelM =
        RearSuspension.MaxTravelM;

    RearGeometry.MaxIterations =
        RearSuspension.MaxSolverIterations;

    RearGeometry.PositionToleranceM =
        RearSuspension.PositionToleranceM;

    FourWheel.RearAxle.LeftSuspension =
        BuildSuspensionConfig(
            RearSuspension.SpringRateNPerM,
            RearSuspension.StaticSpringCompressionM,
            RearSuspension.BumpDampingNsPerM,
            RearSuspension.ReboundDampingNsPerM,
            RearSuspension.MinTravelM,
            RearSuspension.MaxTravelM);

    FourWheel.RearAxle.RightSuspension =
        FourWheel.RearAxle.LeftSuspension;

    FourWheel.RearAxle.AntiRollBar.CouplingRateNPerM =
        RearSuspension.AntiRollCouplingRateNPerM;

    FourWheel.RearAxle.AntiRollBar.MaxTransferForceN =
        RearSuspension.AntiRollMaxTransferForceN;

    CompileStructureRuntime(
        Structure,
        CenterOfMassVehicleLocalM,
        OutConfig.StructureRuntime,
        OutValidation);

    if (!TADoubleWishboneSolver::ValidateConfig(
            FourWheel.FrontAxle.RightGeometry))
    {
        AddValidation(
            OutValidation,
            ETAValidationSeverity::Error,
            TEXT("Vehicle.InvalidFrontSuspension"),
            TEXT("Compiled front double-wishbone geometry is invalid or degenerate."));
    }

    if (!TAMultiLinkSolver::ValidateConfig(
            FourWheel.RearAxle.RightGeometry))
    {
        AddValidation(
            OutValidation,
            ETAValidationSeverity::Error,
            TEXT("Vehicle.InvalidRearSuspension"),
            TEXT("Compiled rear multi-link geometry is invalid or degenerate."));
    }

    const double CompiledWheelbaseM =
        FrontGeometry.Hardpoints.WheelCenterReference.X
        - RearGeometry.WheelCenterReference.X;

    if (FMath::Abs(
            CompiledWheelbaseM
            - Dimensions.WheelbaseMeters) > 0.02)
    {
        AddValidation(
            OutValidation,
            ETAValidationSeverity::Warning,
            TEXT("Vehicle.WheelbaseHardpointMismatch"),
            FString::Printf(
                TEXT("Authored wheelbase %.3f m differs from suspension wheel-center geometry %.3f m."),
                Dimensions.WheelbaseMeters,
                CompiledWheelbaseM));
    }

    const double CompiledFrontTrackM =
        2.0
        * FMath::Abs(
            FrontGeometry.Hardpoints.WheelCenterReference.Y);

    const double CompiledRearTrackM =
        2.0
        * FMath::Abs(
            RearGeometry.WheelCenterReference.Y);

    if (FMath::Abs(
            CompiledFrontTrackM
            - Dimensions.TrackFrontMeters) > 0.02)
    {
        AddValidation(
            OutValidation,
            ETAValidationSeverity::Warning,
            TEXT("Vehicle.FrontTrackHardpointMismatch"),
            TEXT("Front track metadata differs from suspension wheel-center geometry."));
    }

    if (FMath::Abs(
            CompiledRearTrackM
            - Dimensions.TrackRearMeters) > 0.02)
    {
        AddValidation(
            OutValidation,
            ETAValidationSeverity::Warning,
            TEXT("Vehicle.RearTrackHardpointMismatch"),
            TEXT("Rear track metadata differs from suspension wheel-center geometry."));
    }

    if (OutValidation.HasErrors())
    {
        return false;
    }

    uint32 Hash =
        GetTypeHash(DefinitionId);

    Hash = HashCombineFast(
        Hash,
        GetTypeHash(Version.SchemaVersion));

    Hash = HashCombineFast(
        Hash,
        GetTypeHash(Version.PhysicsVersion));

    Hash = HashCombineFast(
        Hash,
        GetTypeHash(Version.DamageModelVersion));

    Hash = HashDouble(
        Hash,
        OutConfig.MassKg);

    Hash = HashVector(
        Hash,
        OutConfig.CenterOfMassMeters);

    Hash = HashVector(
        Hash,
        OutConfig.PrincipalInertiaKgm2);

    Hash = HashDouble(
        Hash,
        OutConfig.WheelbaseMeters);

    Hash = HashDouble(
        Hash,
        OutConfig.TrackFrontMeters);

    Hash = HashDouble(
        Hash,
        OutConfig.TrackRearMeters);

    Hash = HashCombineFast(
        Hash,
        GetTypeHash(OutConfig.WheelCount));

    Hash = HashTireConfig(
        Hash,
        VehicleRuntime.Tires[0]);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Wheels[0].InertiaKgm2);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Wheels[0].MaxBrakeTorqueNm);

    for (const FTAWheelRuntimeConfig& WheelConfig :
         VehicleRuntime.Wheels)
    {
        Hash = HashBool(
            Hash,
            WheelConfig.bDriven);
    }

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Engine.IdleRPM);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Engine.RedlineRPM);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Engine.LimiterRPM);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Engine.CrankInertiaKgm2);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Clutch.MaxTorqueCapacityNm);

    for (const double Ratio :
         VehicleRuntime.Gearbox.ForwardGearRatios)
    {
        Hash = HashDouble(Hash, Ratio);
    }

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Gearbox.ReverseGearRatio);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Gearbox.FinalDriveRatio);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Gearbox.MechanicalEfficiency);

    Hash = HashFrontGeometry(
        Hash,
        FrontGeometry);

    Hash = HashSuspensionConfig(
        Hash,
        FourWheel.FrontAxle.LeftSuspension);

    Hash = HashDouble(
        Hash,
        FourWheel.FrontAxle.AntiRollBar.CouplingRateNPerM);

    Hash = HashDouble(
        Hash,
        FourWheel.FrontAxle.AntiRollBar.MaxTransferForceN);

    Hash = HashDouble(
        Hash,
        FourWheel.FrontAxle.SteeringRack.MaxRackDisplacementM);

    Hash = HashDouble(
        Hash,
        FourWheel.FrontAxle.SteeringRack.InputExponent);

    Hash = HashDouble(
        Hash,
        FourWheel.FrontAxle.SteeringRack.SteeringSign);

    Hash = HashRearGeometry(
        Hash,
        RearGeometry);

    Hash = HashSuspensionConfig(
        Hash,
        FourWheel.RearAxle.LeftSuspension);

    Hash = HashDouble(
        Hash,
        FourWheel.RearAxle.AntiRollBar.CouplingRateNPerM);

    Hash = HashDouble(
        Hash,
        FourWheel.RearAxle.AntiRollBar.MaxTransferForceN);

    Hash = HashStructureRuntime(
        Hash,
        OutConfig.StructureRuntime);

    OutConfig.PhysicsConfigHash =
        Hash;

    return true;
}
