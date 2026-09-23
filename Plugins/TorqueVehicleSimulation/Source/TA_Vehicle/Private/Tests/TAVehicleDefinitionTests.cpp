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


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDefinitionCompleteTireAuthoringTest,
    "TorqueAtlas.Vehicle.Definition.CompilesCompleteTireCalibration",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDefinitionCompleteTireAuthoringTest::RunTest(
    const FString& Parameters)
{
    UTAVehicleDefinition* Definition =
        NewObject<UTAVehicleDefinition>();

    Definition->Tire.LoadSensitivityExponent =
        0.11;

    Definition->Tire.LongitudinalStiffnessN =
        101000.0;

    Definition->Tire.CorneringStiffnessNPerRad =
        93000.0;

    Definition->Tire.CamberStiffnessNPerRad =
        7100.0;

    Definition->Tire.SurfaceThermalMassJPerC =
        5100.0;

    Definition->Tire.CarcassThermalMassJPerC =
        14500.0;

    Definition->Tire.DynamicBlendStartMps =
        1.5;

    Definition->Tire.DynamicBlendEndMps =
        4.5;

    Definition->Tire.HydroReferenceOnsetSpeedMps =
        29.0;

    FTAVehicleCompiledConfig Config;
    FTAValidationResult Validation;

    TestTrue(
        TEXT("Complete tire calibration compiles"),
        Definition->BuildCompiledConfig(
            Config,
            Validation));

    const FTATireRuntimeConfig& Tire =
        Config.VehicleRuntime.Tires[0];

    TestTrue(
        TEXT("Load sensitivity reaches runtime"),
        FMath::IsNearlyEqual(
            Tire.LoadSensitivityExponent,
            0.11,
            1.0e-12));

    TestTrue(
        TEXT("Longitudinal stiffness reaches runtime"),
        FMath::IsNearlyEqual(
            Tire.LongitudinalStiffnessN,
            101000.0,
            1.0e-9));

    TestTrue(
        TEXT("Cornering stiffness reaches runtime"),
        FMath::IsNearlyEqual(
            Tire.CorneringStiffnessNPerRad,
            93000.0,
            1.0e-9));

    TestTrue(
        TEXT("Camber stiffness reaches runtime"),
        FMath::IsNearlyEqual(
            Tire.CamberStiffnessNPerRad,
            7100.0,
            1.0e-9));

    TestTrue(
        TEXT("Surface thermal mass reaches runtime"),
        FMath::IsNearlyEqual(
            Tire.SurfaceThermalMassJPerC,
            5100.0,
            1.0e-9));

    TestTrue(
        TEXT("Carcass thermal mass reaches runtime"),
        FMath::IsNearlyEqual(
            Tire.CarcassThermalMassJPerC,
            14500.0,
            1.0e-9));

    TestTrue(
        TEXT("Dynamic blend range reaches runtime"),
        FMath::IsNearlyEqual(
            Tire.DynamicBlendStartMps,
            1.5,
            1.0e-12)
        && FMath::IsNearlyEqual(
            Tire.DynamicBlendEndMps,
            4.5,
            1.0e-12));

    TestTrue(
        TEXT("Hydro reference speed reaches runtime"),
        FMath::IsNearlyEqual(
            Tire.HydroReferenceOnsetSpeedMps,
            29.0,
            1.0e-12));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDefinitionCompletePowertrainAuthoringTest,
    "TorqueAtlas.Vehicle.Definition.CompilesCompletePowertrainCalibration",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDefinitionCompletePowertrainAuthoringTest::RunTest(
    const FString& Parameters)
{
    UTAVehicleDefinition* Definition =
        NewObject<UTAVehicleDefinition>();

    Definition->Drivetrain.FrictionConstantNm =
        11.0;

    Definition->Drivetrain.FrictionLinearNms =
        0.031;

    Definition->Drivetrain.StarterTorqueNm =
        120.0;

    Definition->Drivetrain.ClutchCouplingStiffnessNms =
        27.0;

    Definition->Drivetrain.ClutchWearEnergyCapacityJ =
        1.5e8;

    Definition->Drivetrain.DrivelineTorsionalStiffnessNmPerRad =
        3200.0;

    Definition->EngineThermal.EffectiveThermalMassJPerC =
        82000.0;

    Definition->EngineThermal.CoolingCapacityWPerC =
        1710.0;

    TestTrue(
        TEXT("Default authored torque curve is populated"),
        Definition->Drivetrain.TorqueCurve.Num() >= 2);

    Definition->Drivetrain.TorqueCurve[3].TorqueNm =
        455.0;

    FTAVehicleCompiledConfig Config;
    FTAValidationResult Validation;

    TestTrue(
        TEXT("Complete powertrain calibration compiles"),
        Definition->BuildCompiledConfig(
            Config,
            Validation));

    const FTAVehicleRuntimeConfig& Runtime =
        Config.VehicleRuntime;

    TestTrue(
        TEXT("Engine friction reaches runtime"),
        FMath::IsNearlyEqual(
            Runtime.Engine.FrictionConstantNm,
            11.0,
            1.0e-12)
        && FMath::IsNearlyEqual(
            Runtime.Engine.FrictionLinearNms,
            0.031,
            1.0e-12));

    TestTrue(
        TEXT("Starter torque reaches runtime"),
        FMath::IsNearlyEqual(
            Runtime.Engine.StarterTorqueNm,
            120.0,
            1.0e-12));

    TestEqual(
        TEXT("Authored torque curve point count is preserved"),
        Runtime.Engine.TorqueCurve.Num(),
        Definition->Drivetrain.TorqueCurve.Num());

    TestTrue(
        TEXT("Authored torque curve drives runtime solver"),
        FMath::IsNearlyEqual(
            TAPowertrainSolver::EvaluateTorqueCurveNm(
                Runtime.Engine,
                4000.0),
            455.0,
            1.0e-9));

    TestTrue(
        TEXT("Clutch coupling stiffness reaches runtime"),
        FMath::IsNearlyEqual(
            Runtime.Clutch.CouplingStiffnessNms,
            27.0,
            1.0e-12));

    TestTrue(
        TEXT("Clutch wear capacity reaches runtime"),
        FMath::IsNearlyEqual(
            Runtime.Clutch.WearEnergyCapacityJ,
            1.5e8,
            1.0e-3));

    TestTrue(
        TEXT("Driveline compliance reaches runtime"),
        FMath::IsNearlyEqual(
            Runtime.Driveline.TorsionalStiffnessNmPerRad,
            3200.0,
            1.0e-9));

    TestTrue(
        TEXT("Engine thermal mass reaches runtime"),
        FMath::IsNearlyEqual(
            Runtime.EngineThermal.EffectiveThermalMassJPerC,
            82000.0,
            1.0e-9));

    TestTrue(
        TEXT("Cooling capacity reaches runtime"),
        FMath::IsNearlyEqual(
            Runtime.EngineThermal.CoolingCapacityWPerC,
            1710.0,
            1.0e-9));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDefinitionCalibrationHashCoverageTest,
    "TorqueAtlas.Vehicle.Definition.TirePowertrainCalibrationChangesHash",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDefinitionCalibrationHashCoverageTest::RunTest(
    const FString& Parameters)
{
    UTAVehicleDefinition* Definition =
        NewObject<UTAVehicleDefinition>();

    FTAVehicleCompiledConfig Baseline;
    FTAValidationResult BaselineValidation;

    TestTrue(
        TEXT("Baseline vehicle compiles"),
        Definition->BuildCompiledConfig(
            Baseline,
            BaselineValidation));

    Definition->Tire.SurfaceThermalMassJPerC +=
        123.0;

    FTAVehicleCompiledConfig TireModified;
    FTAValidationResult TireValidation;

    TestTrue(
        TEXT("Tire-modified vehicle compiles"),
        Definition->BuildCompiledConfig(
            TireModified,
            TireValidation));

    TestTrue(
        TEXT("Tire thermal calibration changes physics hash"),
        TireModified.PhysicsConfigHash
            != Baseline.PhysicsConfigHash);

    Definition->Drivetrain.FrictionConstantNm +=
        2.0;

    FTAVehicleCompiledConfig EngineModified;
    FTAValidationResult EngineValidation;

    TestTrue(
        TEXT("Engine-modified vehicle compiles"),
        Definition->BuildCompiledConfig(
            EngineModified,
            EngineValidation));

    TestTrue(
        TEXT("Engine friction calibration changes physics hash"),
        EngineModified.PhysicsConfigHash
            != TireModified.PhysicsConfigHash);

    Definition->Drivetrain.TorqueCurve[2].TorqueNm +=
        7.0;

    FTAVehicleCompiledConfig CurveModified;
    FTAValidationResult CurveValidation;

    TestTrue(
        TEXT("Torque-curve-modified vehicle compiles"),
        Definition->BuildCompiledConfig(
            CurveModified,
            CurveValidation));

    TestTrue(
        TEXT("Torque curve changes physics hash"),
        CurveModified.PhysicsConfigHash
            != EngineModified.PhysicsConfigHash);

    Definition->EngineThermal.CoolingCapacityWPerC +=
        50.0;

    FTAVehicleCompiledConfig ThermalModified;
    FTAValidationResult ThermalValidation;

    TestTrue(
        TEXT("Thermal-modified vehicle compiles"),
        Definition->BuildCompiledConfig(
            ThermalModified,
            ThermalValidation));

    TestTrue(
        TEXT("Engine thermal calibration changes physics hash"),
        ThermalModified.PhysicsConfigHash
            != CurveModified.PhysicsConfigHash);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDefinitionRejectsInvalidTireCalibrationTest,
    "TorqueAtlas.Vehicle.Definition.RejectsInvalidTireCalibration",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDefinitionRejectsInvalidTireCalibrationTest::RunTest(
    const FString& Parameters)
{
    UTAVehicleDefinition* Definition =
        NewObject<UTAVehicleDefinition>();

    Definition->Tire.DynamicBlendStartMps =
        5.0;

    Definition->Tire.DynamicBlendEndMps =
        2.0;

    FTAVehicleCompiledConfig Config;
    FTAValidationResult Validation;

    TestFalse(
        TEXT("Reversed dynamic-blend range is rejected"),
        Definition->BuildCompiledConfig(
            Config,
            Validation));

    TestTrue(
        TEXT("Invalid tire calibration emits errors"),
        Validation.HasErrors());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDefinitionRejectsInvalidTorqueCurveTest,
    "TorqueAtlas.Vehicle.Definition.RejectsInvalidTorqueCurve",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDefinitionRejectsInvalidTorqueCurveTest::RunTest(
    const FString& Parameters)
{
    UTAVehicleDefinition* Definition =
        NewObject<UTAVehicleDefinition>();

    TestTrue(
        TEXT("Default torque curve has enough points"),
        Definition->Drivetrain.TorqueCurve.Num() >= 2);

    Definition->Drivetrain.TorqueCurve[1].RPM =
        Definition->Drivetrain.TorqueCurve[0].RPM;

    FTAVehicleCompiledConfig Config;
    FTAValidationResult Validation;

    TestFalse(
        TEXT("Duplicate/non-increasing torque-curve RPM is rejected"),
        Definition->BuildCompiledConfig(
            Config,
            Validation));

    TestTrue(
        TEXT("Invalid torque curve emits errors"),
        Validation.HasErrors());

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDefinitionSuspensionStopAndCoolingAuthoringTest,
    "TorqueAtlas.Vehicle.Definition.CompilesSuspensionStopsAndCooling",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDefinitionSuspensionStopAndCoolingAuthoringTest::RunTest(
    const FString& Parameters)
{
    UTAVehicleDefinition* Definition =
        NewObject<UTAVehicleDefinition>();

    Definition->FrontSuspension.BumpStopRateNPerM =
        210000.0;

    Definition->FrontSuspension.DroopStopRateNPerM =
        135000.0;

    Definition->RearSuspension.BumpStopRateNPerM =
        195000.0;

    Definition->RearSuspension.DroopStopRateNPerM =
        125000.0;

    Definition->Cooling.InitialCoolantMassKg =
        7.2;

    Definition->Cooling.PunctureThresholdEnergyJ =
        3200.0;

    Definition->Cooling.FullLeakEnergyJ =
        24500.0;

    Definition->Cooling.MaxLeakAreaMm2 =
        24.0;

    Definition->Cooling.LeakMassFlowKgPerSecPerMm2 =
        0.004;

    Definition->Cooling.MinimumAirflowEfficiency01 =
        0.18;

    FTAVehicleCompiledConfig Config;
    FTAValidationResult Validation;

    TestTrue(
        TEXT("Suspension stop and cooling calibration compiles"),
        Definition->BuildCompiledConfig(
            Config,
            Validation));

    TestTrue(
        TEXT("Front bump-stop rate reaches runtime"),
        FMath::IsNearlyEqual(
            Config.FourWheelRuntime.FrontAxle
                .LeftSuspension.BumpStopRateNPerM,
            210000.0,
            1.0e-9));

    TestTrue(
        TEXT("Front droop-stop rate reaches runtime"),
        FMath::IsNearlyEqual(
            Config.FourWheelRuntime.FrontAxle
                .LeftSuspension.DroopStopRateNPerM,
            135000.0,
            1.0e-9));

    TestTrue(
        TEXT("Rear bump-stop rate reaches runtime"),
        FMath::IsNearlyEqual(
            Config.FourWheelRuntime.RearAxle
                .LeftSuspension.BumpStopRateNPerM,
            195000.0,
            1.0e-9));

    TestTrue(
        TEXT("Rear droop-stop rate reaches runtime"),
        FMath::IsNearlyEqual(
            Config.FourWheelRuntime.RearAxle
                .LeftSuspension.DroopStopRateNPerM,
            125000.0,
            1.0e-9));

    TestTrue(
        TEXT("Coolant mass reaches radiator runtime"),
        FMath::IsNearlyEqual(
            Config.VehicleRuntime.Radiator.InitialCoolantMassKg,
            7.2,
            1.0e-12));

    TestTrue(
        TEXT("Radiator puncture threshold reaches runtime"),
        FMath::IsNearlyEqual(
            Config.VehicleRuntime.Radiator.PunctureThresholdEnergyJ,
            3200.0,
            1.0e-9));

    TestTrue(
        TEXT("Radiator full-leak threshold reaches runtime"),
        FMath::IsNearlyEqual(
            Config.VehicleRuntime.Radiator.FullLeakEnergyJ,
            24500.0,
            1.0e-9));

    TestTrue(
        TEXT("Radiator leak calibration reaches runtime"),
        FMath::IsNearlyEqual(
            Config.VehicleRuntime.Radiator.LeakMassFlowKgPerSecPerMm2,
            0.004,
            1.0e-12));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDefinitionCoolingAndStopHashCoverageTest,
    "TorqueAtlas.Vehicle.Definition.CoolingAndStopsChangePhysicsHash",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDefinitionCoolingAndStopHashCoverageTest::RunTest(
    const FString& Parameters)
{
    UTAVehicleDefinition* Definition =
        NewObject<UTAVehicleDefinition>();

    FTAVehicleCompiledConfig Baseline;
    FTAValidationResult BaselineValidation;

    TestTrue(
        TEXT("Baseline vehicle compiles"),
        Definition->BuildCompiledConfig(
            Baseline,
            BaselineValidation));

    Definition->FrontSuspension.BumpStopRateNPerM +=
        1000.0;

    FTAVehicleCompiledConfig StopModified;
    FTAValidationResult StopValidation;

    TestTrue(
        TEXT("Stop-modified vehicle compiles"),
        Definition->BuildCompiledConfig(
            StopModified,
            StopValidation));

    TestTrue(
        TEXT("Suspension stop calibration changes physics hash"),
        StopModified.PhysicsConfigHash
            != Baseline.PhysicsConfigHash);

    Definition->Cooling.PunctureThresholdEnergyJ +=
        250.0;

    FTAVehicleCompiledConfig CoolingModified;
    FTAValidationResult CoolingValidation;

    TestTrue(
        TEXT("Cooling-modified vehicle compiles"),
        Definition->BuildCompiledConfig(
            CoolingModified,
            CoolingValidation));

    TestTrue(
        TEXT("Radiator damage calibration changes physics hash"),
        CoolingModified.PhysicsConfigHash
            != StopModified.PhysicsConfigHash);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDefinitionRejectsInvalidCoolingCalibrationTest,
    "TorqueAtlas.Vehicle.Definition.RejectsInvalidCoolingCalibration",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDefinitionRejectsInvalidCoolingCalibrationTest::RunTest(
    const FString& Parameters)
{
    UTAVehicleDefinition* Definition =
        NewObject<UTAVehicleDefinition>();

    Definition->Cooling.PunctureThresholdEnergyJ =
        5000.0;

    Definition->Cooling.FullLeakEnergyJ =
        4000.0;

    FTAVehicleCompiledConfig Config;
    FTAValidationResult Validation;

    TestFalse(
        TEXT("Full leak below puncture threshold is rejected"),
        Definition->BuildCompiledConfig(
            Config,
            Validation));

    TestTrue(
        TEXT("Invalid cooling calibration emits errors"),
        Validation.HasErrors());

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDefinitionDerivedKinematicCacheTest,
    "TorqueAtlas.Vehicle.Definition.DerivesSuspensionKinematicCaches",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDefinitionDerivedKinematicCacheTest::RunTest(
    const FString& Parameters)
{
    UTAVehicleDefinition* Definition =
        NewObject<UTAVehicleDefinition>();

    FTAVehicleCompiledConfig Config;
    FTAValidationResult Validation;

    TestTrue(
        TEXT("Vehicle compiles with derived kinematic caches"),
        Definition->BuildCompiledConfig(
            Config,
            Validation));

    const FTASuspensionRuntimeConfig& FrontRightCache =
        Config.FourWheelRuntime.FrontAxle.RightSuspension;

    const FTASuspensionRuntimeConfig& FrontLeftCache =
        Config.FourWheelRuntime.FrontAxle.LeftSuspension;

    const FTASuspensionRuntimeConfig& RearRightCache =
        Config.FourWheelRuntime.RearAxle.RightSuspension;

    const FTASuspensionRuntimeConfig& RearLeftCache =
        Config.FourWheelRuntime.RearAxle.LeftSuspension;

    TestEqual(
        TEXT("Front-right cache has 17 derived samples"),
        FrontRightCache.KinematicSamples.Num(),
        17);

    TestEqual(
        TEXT("Front-left cache has 17 derived samples"),
        FrontLeftCache.KinematicSamples.Num(),
        17);

    TestEqual(
        TEXT("Rear-right cache has 17 derived samples"),
        RearRightCache.KinematicSamples.Num(),
        17);

    TestEqual(
        TEXT("Rear-left cache has 17 derived samples"),
        RearLeftCache.KinematicSamples.Num(),
        17);

    TestTrue(
        TEXT("Front cache spans complete authored travel"),
        FMath::IsNearlyEqual(
            FrontRightCache.KinematicSamples[0].TravelM,
            Config.FourWheelRuntime.FrontAxle
                .RightGeometry.MinTravelM,
            1.0e-12)
        && FMath::IsNearlyEqual(
            FrontRightCache.KinematicSamples.Last().TravelM,
            Config.FourWheelRuntime.FrontAxle
                .RightGeometry.MaxTravelM,
            1.0e-12));

    TestTrue(
        TEXT("Rear cache spans complete authored travel"),
        FMath::IsNearlyEqual(
            RearRightCache.KinematicSamples[0].TravelM,
            Config.FourWheelRuntime.RearAxle
                .RightGeometry.MinTravelM,
            1.0e-12)
        && FMath::IsNearlyEqual(
            RearRightCache.KinematicSamples.Last().TravelM,
            Config.FourWheelRuntime.RearAxle
                .RightGeometry.MaxTravelM,
            1.0e-12));

    const double FrontProbeTravelM =
        0.5
        * (FrontRightCache.KinematicSamples[8].TravelM
            + FrontRightCache.KinematicSamples[9].TravelM);

    FTASuspensionRuntimeState CachedFrontState;

    TestTrue(
        TEXT("Front derived cache interpolates"),
        TASuspensionRuntime::EvaluateKinematicCache(
            FrontRightCache,
            FrontProbeTravelM,
            CachedFrontState));

    FTADoubleWishboneState DirectFrontState;
    FTADoubleWishboneSolveInput DirectFrontInput;
    DirectFrontInput.TravelM =
        FrontProbeTravelM;

    FTADoubleWishboneSolveOutput DirectFrontOutput;

    TestTrue(
        TEXT("Direct front geometry solves at cache probe"),
        TADoubleWishboneSolver::Solve(
            Config.FourWheelRuntime.FrontAxle.RightGeometry,
            DirectFrontInput,
            DirectFrontState,
            DirectFrontOutput));

    const FVector3d DirectFrontOffsetM =
        DirectFrontOutput.WheelCenterLocalM
        - Config.FourWheelRuntime.FrontAxle
            .RightGeometry.Hardpoints.WheelCenterReference;

    TestTrue(
        TEXT("Interpolated front wheel-center offset remains close to direct solver"),
        (CachedFrontState.WheelCenterOffsetM
            - DirectFrontOffsetM).Length()
            < 0.002);

    TestTrue(
        TEXT("Interpolated front camber remains close to direct solver"),
        FMath::Abs(
            CachedFrontState.CamberRad
            - DirectFrontOutput.CamberRad)
            < FMath::DegreesToRadians(0.25));

    TestTrue(
        TEXT("Interpolated front toe remains close to direct solver"),
        FMath::Abs(
            CachedFrontState.ToeRad
            - DirectFrontOutput.ToeRad)
            < FMath::DegreesToRadians(0.25));

    const double RearProbeTravelM =
        0.5
        * (RearRightCache.KinematicSamples[8].TravelM
            + RearRightCache.KinematicSamples[9].TravelM);

    FTASuspensionRuntimeState CachedRearState;

    TestTrue(
        TEXT("Rear derived cache interpolates"),
        TASuspensionRuntime::EvaluateKinematicCache(
            RearRightCache,
            RearProbeTravelM,
            CachedRearState));

    FTAMultiLinkRuntimeState DirectRearState;
    FTAMultiLinkSolveInput DirectRearInput;
    DirectRearInput.TravelM =
        RearProbeTravelM;

    FTAMultiLinkSolveOutput DirectRearOutput;

    TestTrue(
        TEXT("Direct rear geometry solves at cache probe"),
        TAMultiLinkSolver::Solve(
            Config.FourWheelRuntime.RearAxle.RightGeometry,
            DirectRearInput,
            DirectRearState,
            DirectRearOutput));

    const FVector3d DirectRearOffsetM =
        DirectRearOutput.WheelCenterLocalM
        - Config.FourWheelRuntime.RearAxle
            .RightGeometry.WheelCenterReference;

    TestTrue(
        TEXT("Interpolated rear wheel-center offset remains close to direct solver"),
        (CachedRearState.WheelCenterOffsetM
            - DirectRearOffsetM).Length()
            < 0.003);

    TestTrue(
        TEXT("Interpolated rear camber remains close to direct solver"),
        FMath::Abs(
            CachedRearState.CamberRad
            - DirectRearOutput.CamberRad)
            < FMath::DegreesToRadians(0.35));

    TestTrue(
        TEXT("Interpolated rear toe remains close to direct solver"),
        FMath::Abs(
            CachedRearState.ToeRad
            - DirectRearOutput.ToeRad)
            < FMath::DegreesToRadians(0.35));

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDefinitionStructureGravityOwnershipTest,
    "TorqueAtlas.Vehicle.Definition.StructureDoesNotDoubleCountGravity",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDefinitionStructureGravityOwnershipTest::RunTest(
    const FString& Parameters)
{
    UTAVehicleDefinition* Definition =
        NewObject<UTAVehicleDefinition>();

    FTAStructureNodeAuthoringDefinition Node;
    Node.PositionVehicleLocalM =
        FVector(0.0, 0.0, 0.0);
    Node.MassKg =
        10.0;

    Definition->Structure.Nodes.Add(
        Node);

    FTAVehicleCompiledConfig Config;
    FTAValidationResult Validation;

    TestTrue(
        TEXT("Vehicle with internal structure compiles"),
        Definition->BuildCompiledConfig(
            Config,
            Validation));

    TestTrue(
        TEXT("Vehicle-attached structure uses zero local gravity"),
        Config.StructureRuntime.Solver.GravityMps2
            .IsNearlyZero(1.0e-12));

    TestTrue(
        TEXT("Rigid chassis retains world gravity"),
        Config.VehicleRuntime.Chassis.GravityWorldMps2.Z
            < -9.0);

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDefinitionVersionPolicyTest,
    "TorqueAtlas.Vehicle.Definition.VersionPolicy",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDefinitionVersionPolicyTest::RunTest(
    const FString& Parameters)
{
    UTAVehicleDefinition* Current =
        NewObject<UTAVehicleDefinition>();

    TestEqual(
        TEXT("New asset defaults to current schema version"),
        Current->Version.SchemaVersion,
        TAVersion::CurrentSchemaVersion);

    TestEqual(
        TEXT("New asset defaults to current physics version"),
        Current->Version.PhysicsVersion,
        TAVersion::CurrentPhysicsVersion);

    TestEqual(
        TEXT("New asset defaults to current damage-model version"),
        Current->Version.DamageModelVersion,
        TAVersion::CurrentDamageModelVersion);

    FTAVehicleCompiledConfig CurrentConfig;
    FTAValidationResult CurrentValidation;

    TestTrue(
        TEXT("Current-version asset compiles"),
        Current->BuildCompiledConfig(
            CurrentConfig,
            CurrentValidation));

    UTAVehicleDefinition* Legacy =
        NewObject<UTAVehicleDefinition>();

    Legacy->Version.SchemaVersion =
        1;

    Legacy->Version.PhysicsVersion =
        1;

    Legacy->Version.DamageModelVersion =
        1;

    FTAVehicleCompiledConfig LegacyConfig;
    FTAValidationResult LegacyValidation;

    TestTrue(
        TEXT("Legacy v1 asset remains loadable"),
        Legacy->BuildCompiledConfig(
            LegacyConfig,
            LegacyValidation));

    bool bHasLegacyWarning = false;

    for (const FTAValidationMessage& Message :
         LegacyValidation.Messages)
    {
        bHasLegacyWarning |=
            Message.Code ==
                TEXT("Vehicle.LegacyVersion")
            && Message.Severity ==
                ETAValidationSeverity::Warning;
    }

    TestTrue(
        TEXT("Legacy asset emits explicit upgrade/review warning"),
        bHasLegacyWarning);

    UTAVehicleDefinition* Future =
        NewObject<UTAVehicleDefinition>();

    Future->Version.PhysicsVersion =
        TAVersion::CurrentPhysicsVersion + 1;

    FTAVehicleCompiledConfig FutureConfig;
    FTAValidationResult FutureValidation;

    TestFalse(
        TEXT("Unsupported future-version asset is rejected"),
        Future->BuildCompiledConfig(
            FutureConfig,
            FutureValidation));

    TestTrue(
        TEXT("Future-version rejection emits validation errors"),
        FutureValidation.HasErrors());

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDefinitionSuspensionDamageRoutesTest,
    "TorqueAtlas.Vehicle.Definition.CompilesSuspensionDamageRoutes",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDefinitionSuspensionDamageRoutesTest::RunTest(
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

    FTAVehicleDamageRouteAuthoringDefinition SuspensionRoute;
    SuspensionRoute.TargetComponentIndex =
        10;
    SuspensionRoute.Consumer =
        ETAVehicleDamageConsumerAuthoringType::SuspensionCorner;
    SuspensionRoute.WheelIndex =
        0;
    SuspensionRoute.MinimumSpringEfficiency01 =
        0.25;
    SuspensionRoute.MinimumDampingEfficiency01 =
        0.10;
    SuspensionRoute.MinimumStopEfficiency01 =
        0.80;

    Definition->Structure.DamageRoutes.Add(
        SuspensionRoute);

    FTAVehicleDamageRouteAuthoringDefinition AntiRollRoute;
    AntiRollRoute.TargetComponentIndex =
        11;
    AntiRollRoute.Consumer =
        ETAVehicleDamageConsumerAuthoringType::AntiRollLink;
    AntiRollRoute.WheelIndex =
        1;
    AntiRollRoute.MinimumAntiRollLinkEfficiency01 =
        0.0;

    Definition->Structure.DamageRoutes.Add(
        AntiRollRoute);

    FTAVehicleCompiledConfig Config;
    FTAValidationResult Validation;

    TestTrue(
        TEXT("Suspension damage routes compile"),
        Definition->BuildCompiledConfig(
            Config,
            Validation));

    TestEqual(
        TEXT("Two suspension-related routes compile"),
        Config.StructureRuntime.DamageRouting.Routes.Num(),
        2);

    const FTAVehicleDamageRoute& CompiledSuspension =
        Config.StructureRuntime.DamageRouting.Routes[0];

    TestTrue(
        TEXT("Suspension-corner consumer type is preserved"),
        CompiledSuspension.Consumer
            == ETAVehicleDamageConsumerType::SuspensionCorner);

    TestEqual(
        TEXT("Suspension-corner wheel index is preserved"),
        CompiledSuspension.WheelIndex,
        0);

    TestTrue(
        TEXT("Suspension spring/damping/stop minima are preserved"),
        FMath::IsNearlyEqual(
            CompiledSuspension.MinimumSpringEfficiency01,
            0.25,
            1.0e-9)
        && FMath::IsNearlyEqual(
            CompiledSuspension.MinimumDampingEfficiency01,
            0.10,
            1.0e-9)
        && FMath::IsNearlyEqual(
            CompiledSuspension.MinimumStopEfficiency01,
            0.80,
            1.0e-9));

    const FTAVehicleDamageRoute& CompiledAntiRoll =
        Config.StructureRuntime.DamageRouting.Routes[1];

    TestTrue(
        TEXT("Anti-roll-link consumer type is preserved"),
        CompiledAntiRoll.Consumer
            == ETAVehicleDamageConsumerType::AntiRollLink);

    TestEqual(
        TEXT("Anti-roll-link wheel index is preserved"),
        CompiledAntiRoll.WheelIndex,
        1);

    TestTrue(
        TEXT("Anti-roll-link minimum efficiency is preserved"),
        FMath::IsNearlyEqual(
            CompiledAntiRoll.MinimumAntiRollLinkEfficiency01,
            0.0,
            1.0e-9));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDefinitionSuspensionDamageHashSensitivityTest,
    "TorqueAtlas.Vehicle.Definition.SuspensionDamageCalibrationChangesHash",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDefinitionSuspensionDamageHashSensitivityTest::RunTest(
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
        10;
    Route.Consumer =
        ETAVehicleDamageConsumerAuthoringType::SuspensionCorner;
    Route.WheelIndex =
        0;
    Route.MinimumSpringEfficiency01 =
        0.30;

    Definition->Structure.DamageRoutes.Add(
        Route);

    FTAVehicleCompiledConfig Baseline;
    FTAValidationResult BaselineValidation;

    TestTrue(
        TEXT("Baseline suspension route compiles"),
        Definition->BuildCompiledConfig(
            Baseline,
            BaselineValidation));

    Definition->Structure.DamageRoutes[0]
        .MinimumSpringEfficiency01 =
        0.45;

    FTAVehicleCompiledConfig SpringModified;
    FTAValidationResult SpringValidation;

    TestTrue(
        TEXT("Modified spring minimum compiles"),
        Definition->BuildCompiledConfig(
            SpringModified,
            SpringValidation));

    TestTrue(
        TEXT("Spring damage calibration changes physics hash"),
        SpringModified.PhysicsConfigHash
            != Baseline.PhysicsConfigHash);

    Definition->Structure.DamageRoutes[0]
        .Consumer =
        ETAVehicleDamageConsumerAuthoringType::AntiRollLink;

    Definition->Structure.DamageRoutes[0]
        .MinimumAntiRollLinkEfficiency01 =
        0.35;

    FTAVehicleCompiledConfig AntiRollModified;
    FTAValidationResult AntiRollValidation;

    TestTrue(
        TEXT("Anti-roll route compiles"),
        Definition->BuildCompiledConfig(
            AntiRollModified,
            AntiRollValidation));

    TestTrue(
        TEXT("Changing consumer/calibration changes physics hash"),
        AntiRollModified.PhysicsConfigHash
            != SpringModified.PhysicsConfigHash);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDefinitionRejectsInvalidSuspensionRouteWheelTest,
    "TorqueAtlas.Vehicle.Definition.RejectsInvalidSuspensionRouteWheel",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDefinitionRejectsInvalidSuspensionRouteWheelTest::RunTest(
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
        10;
    Route.Consumer =
        ETAVehicleDamageConsumerAuthoringType::SuspensionCorner;
    Route.WheelIndex =
        4;
    Route.MinimumSpringEfficiency01 =
        0.20;

    Definition->Structure.DamageRoutes.Add(
        Route);

    FTAVehicleCompiledConfig Config;
    FTAValidationResult Validation;

    TestFalse(
        TEXT("Out-of-range suspension route wheel fails asset compilation"),
        Definition->BuildCompiledConfig(
            Config,
            Validation));

    TestTrue(
        TEXT("Invalid suspension route emits validation errors"),
        Validation.HasErrors());

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDefinitionBrakeThermalAuthoringTest,
    "TorqueAtlas.Vehicle.Definition.CompilesBrakeThermalCalibration",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDefinitionBrakeThermalAuthoringTest::RunTest(
    const FString& Parameters)
{
    UTAVehicleDefinition* Definition =
        NewObject<UTAVehicleDefinition>();

    Definition->Wheel.BrakeThermalMassJPerC =
        42000.0;

    Definition->Wheel.BrakeCoolingWPerC =
        95.0;

    Definition->Wheel.BrakeHeatFraction01 =
        0.90;

    Definition->Wheel.BrakeFadeStartTemperatureC =
        475.0;

    Definition->Wheel.BrakeFadeEndTemperatureC =
        775.0;

    Definition->Wheel.MinimumBrakeFadeTorqueFactor01 =
        0.30;

    Definition->Wheel.BrakeWearEnergyCapacityJ =
        5.0e8;

    Definition->Wheel.BrakeWearTorqueLossAtEnd01 =
        0.35;

    FTAVehicleCompiledConfig Config;
    FTAValidationResult Validation;

    TestTrue(
        TEXT("Brake thermal calibration compiles"),
        Definition->BuildCompiledConfig(
            Config,
            Validation));

    const FTABrakeThermalConfig& Brake =
        Config.VehicleRuntime.Wheels[0].BrakeThermal;

    TestTrue(
        TEXT("Brake thermal mass reaches runtime"),
        FMath::IsNearlyEqual(
            Brake.ThermalMassJPerC,
            42000.0,
            1.0e-9));

    TestTrue(
        TEXT("Brake cooling reaches runtime"),
        FMath::IsNearlyEqual(
            Brake.CoolingWPerC,
            95.0,
            1.0e-9));

    TestTrue(
        TEXT("Brake fade range reaches runtime"),
        FMath::IsNearlyEqual(
            Brake.FadeStartTemperatureC,
            475.0,
            1.0e-9)
        && FMath::IsNearlyEqual(
            Brake.FadeEndTemperatureC,
            775.0,
            1.0e-9));

    TestTrue(
        TEXT("Brake wear torque loss reaches runtime"),
        FMath::IsNearlyEqual(
            Brake.WearTorqueLossAtEnd01,
            0.35,
            1.0e-12));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDefinitionBrakeThermalHashTest,
    "TorqueAtlas.Vehicle.Definition.BrakeThermalChangesPhysicsHash",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDefinitionBrakeThermalHashTest::RunTest(
    const FString& Parameters)
{
    UTAVehicleDefinition* Definition =
        NewObject<UTAVehicleDefinition>();

    FTAVehicleCompiledConfig Baseline;
    FTAValidationResult BaselineValidation;

    TestTrue(
        TEXT("Baseline vehicle compiles"),
        Definition->BuildCompiledConfig(
            Baseline,
            BaselineValidation));

    Definition->Wheel.BrakeFadeStartTemperatureC +=
        25.0;

    FTAVehicleCompiledConfig Modified;
    FTAValidationResult ModifiedValidation;

    TestTrue(
        TEXT("Brake-modified vehicle compiles"),
        Definition->BuildCompiledConfig(
            Modified,
            ModifiedValidation));

    TestTrue(
        TEXT("Brake thermal calibration changes physics hash"),
        Modified.PhysicsConfigHash
            != Baseline.PhysicsConfigHash);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDefinitionRejectsInvalidBrakeThermalTest,
    "TorqueAtlas.Vehicle.Definition.RejectsInvalidBrakeThermalCalibration",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDefinitionRejectsInvalidBrakeThermalTest::RunTest(
    const FString& Parameters)
{
    UTAVehicleDefinition* Definition =
        NewObject<UTAVehicleDefinition>();

    Definition->Wheel.BrakeFadeStartTemperatureC =
        700.0;

    Definition->Wheel.BrakeFadeEndTemperatureC =
        600.0;

    FTAVehicleCompiledConfig Config;
    FTAValidationResult Validation;

    TestFalse(
        TEXT("Reversed brake fade range is rejected"),
        Definition->BuildCompiledConfig(
            Config,
            Validation));

    TestTrue(
        TEXT("Invalid brake calibration emits validation errors"),
        Validation.HasErrors());

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDefinitionFuelElectricalRoutesTest,
    "TorqueAtlas.Vehicle.Definition.CompilesFuelElectricalDamageRoutes",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDefinitionFuelElectricalRoutesTest::RunTest(
    const FString& Parameters)
{
    UTAVehicleDefinition* Definition =
        NewObject<UTAVehicleDefinition>();

    FTAStructureNodeAuthoringDefinition Node;
    Node.PositionVehicleLocalM =
        FVector(1.10, 0.20, -0.25);
    Node.MassKg =
        12.0;

    Definition->Structure.Nodes.Add(
        Node);

    FTAVehicleDamageRouteAuthoringDefinition Electrical;
    Electrical.TargetComponentIndex =
        12;
    Electrical.Consumer =
        ETAVehicleDamageConsumerAuthoringType::ElectricalBus;
    Electrical.bAcceptImpactEnergy =
        false;
    Electrical.bAcceptStructuralDisplacement =
        false;
    Electrical.bAcceptElectricalDisconnection =
        true;
    Electrical.MinimumStarterEfficiency01 =
        0.15;
    Electrical.MinimumEngineControlEfficiency01 =
        0.25;

    Definition->Structure.DamageRoutes.Add(
        Electrical);

    FTAVehicleDamageRouteAuthoringDefinition Fuel;
    Fuel.TargetComponentIndex =
        13;
    Fuel.Consumer =
        ETAVehicleDamageConsumerAuthoringType::FuelDelivery;
    Fuel.bAcceptImpactEnergy =
        false;
    Fuel.bAcceptStructuralDisplacement =
        false;
    Fuel.bAcceptFluidPressureLoss =
        true;
    Fuel.MinimumFuelDeliveryEfficiency01 =
        0.10;

    Definition->Structure.DamageRoutes.Add(
        Fuel);

    FTAVehicleCompiledConfig Config;
    FTAValidationResult Validation;

    TestTrue(
        TEXT("Fuel/electrical damage routes compile"),
        Definition->BuildCompiledConfig(
            Config,
            Validation));

    TestEqual(
        TEXT("Two fuel/electrical routes compile"),
        Config.StructureRuntime.DamageRouting.Routes.Num(),
        2);

    const FTAVehicleDamageRoute& CompiledElectrical =
        Config.StructureRuntime.DamageRouting.Routes[0];

    TestTrue(
        TEXT("Electrical consumer type is preserved"),
        CompiledElectrical.Consumer
            == ETAVehicleDamageConsumerType::ElectricalBus);

    TestTrue(
        TEXT("Electrical disconnection acceptance is preserved"),
        CompiledElectrical.bAcceptElectricalDisconnection);

    TestTrue(
        TEXT("Electrical minima are preserved"),
        FMath::IsNearlyEqual(
            CompiledElectrical.MinimumStarterEfficiency01,
            0.15,
            1.0e-9)
        && FMath::IsNearlyEqual(
            CompiledElectrical.MinimumEngineControlEfficiency01,
            0.25,
            1.0e-9));

    const FTAVehicleDamageRoute& CompiledFuel =
        Config.StructureRuntime.DamageRouting.Routes[1];

    TestTrue(
        TEXT("Fuel-delivery consumer type is preserved"),
        CompiledFuel.Consumer
            == ETAVehicleDamageConsumerType::FuelDelivery);

    TestTrue(
        TEXT("Fluid-pressure-loss acceptance is preserved"),
        CompiledFuel.bAcceptFluidPressureLoss);

    TestTrue(
        TEXT("Fuel-delivery minimum is preserved"),
        FMath::IsNearlyEqual(
            CompiledFuel.MinimumFuelDeliveryEfficiency01,
            0.10,
            1.0e-9));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDefinitionFuelElectricalHashTest,
    "TorqueAtlas.Vehicle.Definition.FuelElectricalCalibrationChangesHash",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDefinitionFuelElectricalHashTest::RunTest(
    const FString& Parameters)
{
    UTAVehicleDefinition* Definition =
        NewObject<UTAVehicleDefinition>();

    FTAStructureNodeAuthoringDefinition Node;
    Node.PositionVehicleLocalM =
        FVector(1.10, 0.20, -0.25);
    Node.MassKg =
        12.0;

    Definition->Structure.Nodes.Add(
        Node);

    FTAVehicleDamageRouteAuthoringDefinition Route;
    Route.TargetComponentIndex =
        12;
    Route.Consumer =
        ETAVehicleDamageConsumerAuthoringType::ElectricalBus;
    Route.bAcceptElectricalDisconnection =
        true;
    Route.MinimumStarterEfficiency01 =
        0.20;

    Definition->Structure.DamageRoutes.Add(
        Route);

    FTAVehicleCompiledConfig Baseline;
    FTAValidationResult BaselineValidation;

    TestTrue(
        TEXT("Baseline electrical route compiles"),
        Definition->BuildCompiledConfig(
            Baseline,
            BaselineValidation));

    Definition->Structure.DamageRoutes[0]
        .MinimumStarterEfficiency01 =
        0.35;

    FTAVehicleCompiledConfig StarterModified;
    FTAValidationResult StarterValidation;

    TestTrue(
        TEXT("Starter-calibration-modified route compiles"),
        Definition->BuildCompiledConfig(
            StarterModified,
            StarterValidation));

    TestTrue(
        TEXT("Starter damage calibration changes physics hash"),
        StarterModified.PhysicsConfigHash
            != Baseline.PhysicsConfigHash);

    Definition->Structure.DamageRoutes[0]
        .Consumer =
        ETAVehicleDamageConsumerAuthoringType::FuelDelivery;

    Definition->Structure.DamageRoutes[0]
        .bAcceptElectricalDisconnection =
        false;

    Definition->Structure.DamageRoutes[0]
        .bAcceptFluidPressureLoss =
        true;

    Definition->Structure.DamageRoutes[0]
        .MinimumFuelDeliveryEfficiency01 =
        0.40;

    FTAVehicleCompiledConfig FuelModified;
    FTAValidationResult FuelValidation;

    TestTrue(
        TEXT("Fuel route compiles"),
        Definition->BuildCompiledConfig(
            FuelModified,
            FuelValidation));

    TestTrue(
        TEXT("Consumer signal flags and fuel calibration change physics hash"),
        FuelModified.PhysicsConfigHash
            != StarterModified.PhysicsConfigHash);

    return true;
}

#endif
