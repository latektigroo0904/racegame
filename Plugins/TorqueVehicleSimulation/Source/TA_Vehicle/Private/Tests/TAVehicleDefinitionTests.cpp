#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TAVehicleDefinition.h"
#include "TACrashDamagePipeline.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDefinitionCompileTest,
    "TorqueAtlas.Vehicle.Definition.CompilesDefaultPrototype",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDefinitionCompileTest::RunTest(const FString& Parameters)
{
    UTAVehicleDefinition* Definition = NewObject<UTAVehicleDefinition>();

    FTAVehicleCompiledConfig Config;
    FTAValidationResult Validation;

    const bool bCompiled = Definition->BuildCompiledConfig(Config, Validation);

    TestTrue(TEXT("Default prototype definition compiles"), bCompiled);
    TestFalse(TEXT("Default prototype has no validation errors"), Validation.HasErrors());
    TestEqual(TEXT("Default wheel count"), Config.WheelCount, 4);
    TestTrue(TEXT("Default mass is positive"), Config.MassKg > 0.0);
    TestTrue(TEXT("Physics config hash is populated"), Config.PhysicsConfigHash != 0u);

    TestEqual(
        TEXT("Compiled runtime has four wheel configs"),
        Config.VehicleRuntime.Wheels.Num(),
        4);

    TestEqual(
        TEXT("Compiled runtime has four tire configs"),
        Config.VehicleRuntime.Tires.Num(),
        4);

    TestTrue(
        TEXT("Compiled front geometry validates"),
        TADoubleWishboneSolver::ValidateConfig(
            Config.FourWheelRuntime.FrontAxle.RightGeometry));

    TestTrue(
        TEXT("Compiled rear five-link geometry validates"),
        TAMultiLinkSolver::ValidateConfig(
            Config.FourWheelRuntime.RearAxle.RightGeometry));

    TestTrue(
        TEXT("Default prototype is rear-wheel drive"),
        !Config.VehicleRuntime.Wheels[0].bDriven
        && !Config.VehicleRuntime.Wheels[1].bDriven
        && Config.VehicleRuntime.Wheels[2].bDriven
        && Config.VehicleRuntime.Wheels[3].bDriven);

    const uint32 OriginalHash =
        Config.PhysicsConfigHash;

    Definition->FrontSuspension.SpringRateNPerM +=
        1000.0;

    FTAVehicleCompiledConfig ModifiedConfig;
    FTAValidationResult ModifiedValidation;

    TestTrue(
        TEXT("Modified handling definition still compiles"),
        Definition->BuildCompiledConfig(
            ModifiedConfig,
            ModifiedValidation));

    TestTrue(
        TEXT("Handling-critical spring change alters physics hash"),
        ModifiedConfig.PhysicsConfigHash != OriginalHash);

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDefinitionHashSensitivityTest,
    "TorqueAtlas.Vehicle.Definition.HashChangesWithPhysics",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDefinitionHashSensitivityTest::RunTest(const FString& Parameters)
{
    UTAVehicleDefinition* Definition =
        NewObject<UTAVehicleDefinition>();

    FTAVehicleCompiledConfig Baseline;
    FTAValidationResult BaselineValidation;

    TestTrue(
        TEXT("Baseline definition compiles"),
        Definition->BuildCompiledConfig(
            Baseline,
            BaselineValidation));

    Definition->Tire.RadialStiffnessNPerM +=
        1000.0;

    FTAVehicleCompiledConfig Modified;
    FTAValidationResult ModifiedValidation;

    TestTrue(
        TEXT("Modified definition compiles"),
        Definition->BuildCompiledConfig(
            Modified,
            ModifiedValidation));

    TestTrue(
        TEXT("Handling-critical tire change changes physics config hash"),
        Modified.PhysicsConfigHash
            != Baseline.PhysicsConfigHash);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDefinitionGeometryValidationTest,
    "TorqueAtlas.Vehicle.Definition.RejectsDegenerateSuspension",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDefinitionGeometryValidationTest::RunTest(const FString& Parameters)
{
    UTAVehicleDefinition* Definition =
        NewObject<UTAVehicleDefinition>();

    Definition->FrontSuspension.UpperInnerB =
        Definition->FrontSuspension.UpperInnerA;

    FTAVehicleCompiledConfig Config;
    FTAValidationResult Validation;

    TestFalse(
        TEXT("Degenerate front suspension fails asset compilation"),
        Definition->BuildCompiledConfig(
            Config,
            Validation));

    TestTrue(
        TEXT("Degenerate front suspension emits validation errors"),
        Validation.HasErrors());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDefinitionComTransformTest,
    "TorqueAtlas.Vehicle.Definition.CompilesHardpointsToComLocal",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDefinitionComTransformTest::RunTest(const FString& Parameters)
{
    UTAVehicleDefinition* Definition =
        NewObject<UTAVehicleDefinition>();

    const FVector AuthoredWheelCenter =
        Definition->FrontSuspension.WheelCenter;

    Definition->Mass.CenterOfMassMeters =
        FVector(0.10, 0.0, 0.05);

    FTAVehicleCompiledConfig Config;
    FTAValidationResult Validation;

    TestTrue(
        TEXT("Definition with non-zero COM compiles"),
        Definition->BuildCompiledConfig(
            Config,
            Validation));

    const FVector3d CompiledWheelCenter =
        Config.FourWheelRuntime
            .FrontAxle.RightGeometry
            .Hardpoints.WheelCenterReference;

    TestTrue(
        TEXT("Compiled hardpoint is translated into COM-local frame"),
        CompiledWheelCenter.Equals(
            FVector3d(AuthoredWheelCenter)
                - FVector3d(Definition->Mass.CenterOfMassMeters),
            1.0e-9));

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDefinitionRuntimeIntegrationTest,
    "TorqueAtlas.Vehicle.Definition.CompiledConfigRunsFourWheelStep",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDefinitionRuntimeIntegrationTest::RunTest(const FString& Parameters)
{
    UTAVehicleDefinition* Definition =
        NewObject<UTAVehicleDefinition>();

    FTAVehicleCompiledConfig Config;
    FTAValidationResult Validation;

    TestTrue(
        TEXT("Vehicle asset compiles into runtime config"),
        Definition->BuildCompiledConfig(
            Config,
            Validation));

    FTAFourWheelRuntimeState State;

    TestTrue(
        TEXT("Compiled vehicle runtime initializes"),
        TAFourWheelVehicleRuntime::Initialize(
            Config.VehicleRuntime,
            State));

    State.Vehicle.Chassis.PositionWorldM =
        FVector3d(0.0, 0.0, 0.777);

    FTAFourWheelStepInput Input;

    Input.FrontLeftRoad.NormalWorld =
        FVector3d(0.0, 0.0, 1.0);

    Input.FrontRightRoad.NormalWorld =
        FVector3d(0.0, 0.0, 1.0);

    Input.RearLeftRoad.NormalWorld =
        FVector3d(0.0, 0.0, 1.0);

    Input.RearRightRoad.NormalWorld =
        FVector3d(0.0, 0.0, 1.0);

    Input.FrontLeftRoad.Surface.Material =
        ETASurfaceMaterial::FreshAsphalt;

    Input.FrontRightRoad.Surface.Material =
        ETASurfaceMaterial::FreshAsphalt;

    Input.RearLeftRoad.Surface.Material =
        ETASurfaceMaterial::FreshAsphalt;

    Input.RearRightRoad.Surface.Material =
        ETASurfaceMaterial::FreshAsphalt;

    FTAFourWheelStepOutput Output;

    TestTrue(
        TEXT("Compiled asset executes canonical four-wheel physics step"),
        TAFourWheelVehicleRuntime::Step(
            Config.VehicleRuntime,
            Config.FourWheelRuntime,
            Input,
            1.0 / 240.0,
            State,
            Output));

    TestTrue(
        TEXT("All four compiled-asset contacts are solved"),
        Output.FrontAxle.LeftContact.bInContact
        && Output.FrontAxle.RightContact.bInContact
        && Output.RearAxle.LeftContact.bInContact
        && Output.RearAxle.RightContact.bInContact);

    TestTrue(
        TEXT("Compiled asset uses finite radial tire compliance"),
        Output.FrontAxle.LeftContact.TireRadialDeflectionM > 0.0
        && Output.RearAxle.LeftContact.TireRadialDeflectionM > 0.0);

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDefinitionStructureRuntimeTest,
    "TorqueAtlas.Vehicle.Definition.CompiledStructureRunsCrashPipeline",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDefinitionStructureRuntimeTest::RunTest(
    const FString& Parameters)
{
    UTAVehicleDefinition* Definition =
        NewObject<UTAVehicleDefinition>();

    const FVector NodePositions[] =
    {
        FVector(1.50, 0.35, -0.48),
        FVector(1.12, 0.35, -0.48),
        FVector(1.55, 0.55, -0.25),
        FVector(1.15, 0.55, -0.25),
        FVector(1.55, 0.20, -0.05),
        FVector(1.15, 0.20, -0.05),
        FVector(1.35, 0.60,  0.10),
        FVector(1.35, 0.10,  0.10)
    };

    for (const FVector& Position : NodePositions)
    {
        FTAStructureNodeAuthoringDefinition Node;
        Node.PositionVehicleLocalM = Position;
        Node.MassKg = 18.0;
        Definition->Structure.Nodes.Add(Node);
    }

    FTAStructureConstraintAuthoringDefinition Constraint;
    Constraint.NodeA = 0;
    Constraint.NodeB = 1;
    Constraint.Compliance = 1.0e-7;
    Constraint.YieldStrain = 0.04;
    Constraint.FractureStrain = 0.20;
    Definition->Structure.Constraints.Add(Constraint);

    Definition->Structure.ImpactTargetComponentIndex = 7;
    Definition->Structure.ImpactDistributionRadiusM = 0.90;
    Definition->Structure.DeformationImpulseFraction01 = 0.45;
    Definition->Structure.MaxNodeDeltaVelocityMps = 8.0;

    FTAStructureMountDamageAuthoringDefinition Mount;
    Mount.TargetComponentIndex = 42;
    Mount.NodeIndices = { 0 };
    Mount.Weights = { 1.0 };
    Mount.DisplacementThresholdsM = { 0.0005, 0.005, 0.015 };
    Definition->Structure.MountDamageBindings.Add(Mount);

    FTAVehicleDamageRouteAuthoringDefinition Route;
    Route.TargetComponentIndex = 7;
    Route.Consumer = ETAVehicleDamageConsumerAuthoringType::Radiator;
    Route.ImpactEnergyScale = 1.0;
    Route.FullCrushDisplacementM = 0.10;
    Definition->Structure.DamageRoutes.Add(Route);

    Definition->Structure
        .FrontRightSuspensionBindings
        .LowerInnerA.NodeIndices = { 0 };

    Definition->Structure
        .FrontRightSuspensionBindings
        .LowerInnerA.Weights = { 1.0 };

    Definition->Structure
        .FrontRightSuspensionBindings
        .LowerInnerB.NodeIndices = { 1 };

    Definition->Structure
        .FrontRightSuspensionBindings
        .LowerInnerB.Weights = { 1.0 };

    FTAVehicleCompiledConfig Config;
    FTAValidationResult Validation;

    TestTrue(
        TEXT("Vehicle asset with structural data compiles"),
        Definition->BuildCompiledConfig(
            Config,
            Validation));

    TestTrue(
        TEXT("Compiled vehicle reports structural runtime"),
        Config.StructureRuntime.HasStructure());

    TestEqual(
        TEXT("All authored structure nodes compile"),
        Config.StructureRuntime.InitialNodes.Num(),
        8);

    TestEqual(
        TEXT("Authored constraint compiles"),
        Config.StructureRuntime.Constraints.Num(),
        1);

    const FVector3d ExpectedNode0 =
        FVector3d(NodePositions[0])
        - FVector3d(Definition->Mass.CenterOfMassMeters);

    TestTrue(
        TEXT("Structure node positions compile into COM-local coordinates"),
        Config.StructureRuntime.InitialNodes[0]
            .ReferencePositionM.Equals(
                ExpectedNode0,
                1.0e-9));

    TestTrue(
        TEXT("Constraint rest length derives from undamaged authored geometry"),
        FMath::IsNearlyEqual(
            Config.StructureRuntime.Constraints[0].RestLengthM,
            (FVector3d(NodePositions[1])
                - FVector3d(NodePositions[0])).Length(),
            1.0e-9));

    FTAVehicleRuntimeState VehicleState;

    TestTrue(
        TEXT("Compiled vehicle physics initializes"),
        TAVehicleSimulation::Initialize(
            Config.VehicleRuntime,
            VehicleState));

    // Compact fixture-specific radiator threshold.
    FTAVehicleRuntimeConfig CrashVehicleConfig =
        Config.VehicleRuntime;

    CrashVehicleConfig.Radiator.PunctureThresholdEnergyJ = 50.0;
    CrashVehicleConfig.Radiator.FullLeakEnergyJ = 500.0;

    TADamage::InitializeRadiatorState(
        CrashVehicleConfig.Radiator,
        VehicleState.Radiator);

    TArray<FTAStructureNode> Nodes =
        Config.StructureRuntime.InitialNodes;

    TArray<FTADistanceConstraint> Constraints =
        Config.StructureRuntime.Constraints;

    FTACrashDamagePipelineConfig PipelineConfig;
    PipelineConfig.CollisionCoupling.StructureImpact =
        Config.StructureRuntime.ImpactDistribution;

    PipelineConfig.StructureSolver =
        Config.StructureRuntime.Solver;

    PipelineConfig.StructureSolver.GravityMps2 =
        FVector3d::ZeroVector;

    PipelineConfig.DamageBridge =
        Config.StructureRuntime.DamageBridge;

    PipelineConfig.DamageRouting =
        Config.StructureRuntime.DamageRouting;

    FTAStructureDamageBridgeState BridgeState;

    TestTrue(
        TEXT("Compiled damage bridge initializes"),
        TAStructureDamageBridge::InitializeState(
            PipelineConfig.DamageBridge,
            Constraints.Num(),
            BridgeState));

    FTAStructureImpactScratch Scratch;
    Scratch.Initialize(32);

    FTADamageEventQueue Queue;
    Queue.Initialize(32);

    FTACrashDamagePipelineInput Input;
    Input.SimulationTick = 700;
    Input.Substep = 0;
    Input.StructureDeltaTimeSeconds = 1.0 / 120.0;
    Input.Collision.ContactPointWorldM =
        FVector3d(1.62, 0.70, -0.42);

    Input.Collision.CollisionImpulseWorldNs =
        FVector3d(-4200.0, -650.0, 120.0);

    FTACrashDamagePipelineOutput Output;

    TestTrue(
        TEXT("Compiled structural content executes crash pipeline"),
        TACrashDamagePipeline::ProcessImpact(
            PipelineConfig,
            CrashVehicleConfig,
            Input,
            VehicleState,
            Nodes,
            Constraints,
            Scratch,
            BridgeState,
            Queue,
            Output));

    TestTrue(
        TEXT("Compiled radiator route receives crash consequence"),
        VehicleState.Radiator.bPunctured
        || VehicleState.Radiator.AirflowEfficiency01 < 1.0);

    FTADoubleWishboneDamageOffsets DamageOffsets;

    TestTrue(
        TEXT("Compiled front-right structural binding resolves after crash"),
        TASuspensionDamageBinding::ResolveDoubleWishboneDamageOffsets(
            MakeArrayView(Nodes),
            Config.StructureRuntime.FrontRightSuspensionBindings,
            DamageOffsets));

    TestTrue(
        TEXT("Compiled crash content physically moves a bound suspension pickup"),
        DamageOffsets.LowerInnerA.Length() > 0.0005
        || DamageOffsets.LowerInnerB.Length() > 0.0005);

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDefinitionRejectsSpatiallyWrongBindingTest,
    "TorqueAtlas.Vehicle.Definition.RejectsSpatiallyWrongStructureBinding",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDefinitionRejectsSpatiallyWrongBindingTest::RunTest(
    const FString& Parameters)
{
    UTAVehicleDefinition* Definition =
        NewObject<UTAVehicleDefinition>();

    FTAStructureNodeAuthoringDefinition Node;
    Node.PositionVehicleLocalM =
        FVector(-2.0, -2.0, 2.0);
    Node.MassKg = 20.0;

    Definition->Structure.Nodes.Add(Node);

    Definition->Structure
        .FrontRightSuspensionBindings
        .LowerInnerA.NodeIndices = { 0 };

    Definition->Structure
        .FrontRightSuspensionBindings
        .LowerInnerA.Weights = { 1.0 };

    FTAVehicleCompiledConfig Config;
    FTAValidationResult Validation;

    TestFalse(
        TEXT("Far-away structural binding fails vehicle compilation"),
        Definition->BuildCompiledConfig(
            Config,
            Validation));

    TestTrue(
        TEXT("Spatially invalid structural binding emits errors"),
        Validation.HasErrors());

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDefinitionFunctionalDamageRoutesTest,
    "TorqueAtlas.Vehicle.Definition.CompilesFunctionalDamageRoutes",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDefinitionFunctionalDamageRoutesTest::RunTest(
    const FString& Parameters)
{
    UTAVehicleDefinition* Definition =
        NewObject<UTAVehicleDefinition>();

    FTAStructureNodeAuthoringDefinition Node;
    Node.PositionVehicleLocalM =
        FVector(1.20, 0.30, -0.30);
    Node.MassKg =
        15.0;

    Definition->Structure.Nodes.Add(
        Node);

    FTAVehicleDamageRouteAuthoringDefinition SteeringRoute;
    SteeringRoute.TargetComponentIndex =
        8;
    SteeringRoute.Consumer =
        ETAVehicleDamageConsumerAuthoringType::SteeringRack;
    SteeringRoute.FullDamageEnergyJ =
        9000.0;
    SteeringRoute.MinimumSteeringAuthority01 =
        0.30;
    SteeringRoute.MaximumSteeringFreePlayM =
        0.008;

    Definition->Structure.DamageRoutes.Add(
        SteeringRoute);

    FTAVehicleDamageRouteAuthoringDefinition HubRoute;
    HubRoute.TargetComponentIndex =
        9;
    HubRoute.Consumer =
        ETAVehicleDamageConsumerAuthoringType::WheelHub;
    HubRoute.WheelIndex =
        1;
    HubRoute.FullDamageEnergyJ =
        11000.0;
    HubRoute.MinimumBrakeEfficiency01 =
        0.25;
    HubRoute.MinimumDriveEfficiency01 =
        0.10;
    HubRoute.MaximumBearingDragTorqueNm =
        65.0;

    Definition->Structure.DamageRoutes.Add(
        HubRoute);

    FTAVehicleCompiledConfig Config;
    FTAValidationResult Validation;

    TestTrue(
        TEXT("Functional damage routes compile"),
        Definition->BuildCompiledConfig(
            Config,
            Validation));

    TestFalse(
        TEXT("Functional damage route asset has no validation errors"),
        Validation.HasErrors());

    TestEqual(
        TEXT("Two damage routes compile"),
        Config.StructureRuntime.DamageRouting.Routes.Num(),
        2);

    const FTAVehicleDamageRoute& CompiledSteering =
        Config.StructureRuntime.DamageRouting.Routes[0];

    TestTrue(
        TEXT("Steering consumer type is preserved"),
        CompiledSteering.Consumer
            == ETAVehicleDamageConsumerType::SteeringRack);

    TestTrue(
        TEXT("Steering authority floor is preserved"),
        FMath::IsNearlyEqual(
            CompiledSteering.MinimumSteeringAuthority01,
            0.30,
            1.0e-9));

    TestTrue(
        TEXT("Steering free-play maximum is preserved"),
        FMath::IsNearlyEqual(
            CompiledSteering.MaximumSteeringFreePlayM,
            0.008,
            1.0e-9));

    const FTAVehicleDamageRoute& CompiledHub =
        Config.StructureRuntime.DamageRouting.Routes[1];

    TestTrue(
        TEXT("Wheel-hub consumer type is preserved"),
        CompiledHub.Consumer
            == ETAVehicleDamageConsumerType::WheelHub);

    TestEqual(
        TEXT("Wheel-hub target wheel is preserved"),
        CompiledHub.WheelIndex,
        1);

    TestTrue(
        TEXT("Wheel-hub bearing drag calibration is preserved"),
        FMath::IsNearlyEqual(
            CompiledHub.MaximumBearingDragTorqueNm,
            65.0,
            1.0e-9));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDefinitionDamageRouteHashSensitivityTest,
    "TorqueAtlas.Vehicle.Definition.DamageRouteChangesPhysicsHash",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDefinitionDamageRouteHashSensitivityTest::RunTest(
    const FString& Parameters)
{
    UTAVehicleDefinition* Definition =
        NewObject<UTAVehicleDefinition>();

    FTAStructureNodeAuthoringDefinition Node;
    Node.PositionVehicleLocalM =
        FVector(1.20, 0.30, -0.30);
    Node.MassKg =
        15.0;

    Definition->Structure.Nodes.Add(
        Node);

    FTAVehicleDamageRouteAuthoringDefinition Route;
    Route.TargetComponentIndex =
        8;
    Route.Consumer =
        ETAVehicleDamageConsumerAuthoringType::SteeringRack;
    Route.bAcceptStructuralFracture =
        false;
    Route.MaximumSteeringFreePlayM =
        0.005;

    Definition->Structure.DamageRoutes.Add(
        Route);

    FTAVehicleCompiledConfig Baseline;
    FTAValidationResult BaselineValidation;

    TestTrue(
        TEXT("Baseline damage route compiles"),
        Definition->BuildCompiledConfig(
            Baseline,
            BaselineValidation));

    Definition->Structure.DamageRoutes[0]
        .bAcceptStructuralFracture =
        true;

    FTAVehicleCompiledConfig FlagModified;
    FTAValidationResult FlagValidation;

    TestTrue(
        TEXT("Modified signal acceptance route compiles"),
        Definition->BuildCompiledConfig(
            FlagModified,
            FlagValidation));

    TestTrue(
        TEXT("Changing accepted signal types changes physics hash"),
        FlagModified.PhysicsConfigHash
            != Baseline.PhysicsConfigHash);

    Definition->Structure.DamageRoutes[0]
        .MaximumSteeringFreePlayM =
        0.012;

    FTAVehicleCompiledConfig ParameterModified;
    FTAValidationResult ParameterValidation;

    TestTrue(
        TEXT("Modified steering damage calibration compiles"),
        Definition->BuildCompiledConfig(
            ParameterModified,
            ParameterValidation));

    TestTrue(
        TEXT("Changing functional damage calibration changes physics hash"),
        ParameterModified.PhysicsConfigHash
            != FlagModified.PhysicsConfigHash);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDefinitionRejectsInvalidHubRouteWheelTest,
    "TorqueAtlas.Vehicle.Definition.RejectsInvalidHubRouteWheel",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDefinitionRejectsInvalidHubRouteWheelTest::RunTest(
    const FString& Parameters)
{
    UTAVehicleDefinition* Definition =
        NewObject<UTAVehicleDefinition>();

    FTAStructureNodeAuthoringDefinition Node;
    Node.PositionVehicleLocalM =
        FVector(1.20, 0.30, -0.30);
    Node.MassKg =
        15.0;

    Definition->Structure.Nodes.Add(
        Node);

    FTAVehicleDamageRouteAuthoringDefinition Route;
    Route.TargetComponentIndex =
        9;
    Route.Consumer =
        ETAVehicleDamageConsumerAuthoringType::WheelHub;
    Route.WheelIndex =
        4;

    Definition->Structure.DamageRoutes.Add(
        Route);

    FTAVehicleCompiledConfig Config;
    FTAValidationResult Validation;

    TestFalse(
        TEXT("Out-of-range wheel-hub route fails asset compilation"),
        Definition->BuildCompiledConfig(
            Config,
            Validation));

    TestTrue(
        TEXT("Invalid wheel-hub route emits validation errors"),
        Validation.HasErrors());

    return true;
}

#endif
