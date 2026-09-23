#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TACrashDamagePipeline.h"
#include "TASuspensionDamageBinding.h"

namespace
{
    FTADoubleWishboneSolverConfig MakeCrashFrontRightGeometry()
    {
        FTADoubleWishboneSolverConfig Config;
        FTADoubleWishboneHardpoints& H =
            Config.Hardpoints;

        H.UpperInnerA = FVector3d(1.48, 0.38, -0.22);
        H.UpperInnerB = FVector3d(1.14, 0.38, -0.22);
        H.LowerInnerA = FVector3d(1.50, 0.35, -0.48);
        H.LowerInnerB = FVector3d(1.12, 0.35, -0.48);
        H.TieRodInner = FVector3d(1.12, 0.35, -0.38);

        H.DamperChassis = FVector3d(1.31, 0.40, -0.05);
        H.DamperLowerArmReference = FVector3d(1.31, 0.52, -0.43);

        H.UpperBallJointReference = FVector3d(1.31, 0.70, -0.25);
        H.LowerBallJointReference = FVector3d(1.31, 0.73, -0.50);
        H.TieRodOuterReference = FVector3d(1.12, 0.71, -0.39);
        H.WheelCenterReference = FVector3d(1.31, 0.775, -0.45);

        H.WheelForwardReference = FVector3d(1.0, 0.0, 0.0);
        H.WheelUpReference = FVector3d(0.0, 0.0, 1.0);
        H.SteeringRackAxisLocal = FVector3d(0.0, 1.0, 0.0);
        H.SideSign = 1.0;

        Config.MaxIterations = 100;
        Config.PositionToleranceM = 0.00075;

        return Config;
    }

    TArray<FTAStructureNode> MakeCrashStructureNodes()
    {
        TArray<FTAStructureNode> Nodes;
        Nodes.SetNum(8);

        // Two nodes coincide with the front-right lower-arm chassis pickups.
        Nodes[0].PositionM = FVector3d(1.50, 0.35, -0.48);
        Nodes[1].PositionM = FVector3d(1.12, 0.35, -0.48);

        // Nearby structural support nodes provide a non-degenerate impact volume.
        Nodes[2].PositionM = FVector3d(1.55, 0.55, -0.25);
        Nodes[3].PositionM = FVector3d(1.15, 0.55, -0.25);
        Nodes[4].PositionM = FVector3d(1.55, 0.20, -0.05);
        Nodes[5].PositionM = FVector3d(1.15, 0.20, -0.05);
        Nodes[6].PositionM = FVector3d(1.35, 0.60,  0.10);
        Nodes[7].PositionM = FVector3d(1.35, 0.10,  0.10);

        for (FTAStructureNode& Node : Nodes)
        {
            Node.InverseMassPerKg = 1.0 / 18.0;
        }

        TAStructureSolver::InitializeReferencePositionsFromCurrent(
            Nodes);

        return Nodes;
    }

    FTACrashDamagePipelineConfig MakeCrashPipelineConfig()
    {
        FTACrashDamagePipelineConfig Config;

        Config.CollisionCoupling.StructureImpact.DistributionRadiusM = 0.85;
        Config.CollisionCoupling.StructureImpact.DistanceFalloffExponent = 2.0;
        Config.CollisionCoupling.StructureImpact.DeformationImpulseFraction01 = 0.45;
        Config.CollisionCoupling.StructureImpact.MaxNodeDeltaVelocityMps = 8.0;

        Config.StructureSolver.GravityMps2 = FVector3d::ZeroVector;
        Config.StructureSolver.ConstraintIterations = 8;

        Config.DamageBridge.ImpactTargetComponentIndex = 7;

        FTAStructureMountDamageBinding Mount;
        Mount.TargetComponentIndex = 42;
        Mount.NodeIndices = { 0 };
        Mount.Weights = { 1.0 };
        Mount.DisplacementThresholdsM = { 0.001, 0.005, 0.015 };

        Config.DamageBridge.MountBindings.Add(Mount);

        FTAVehicleDamageRoute RadiatorRoute;
        RadiatorRoute.TargetComponentIndex = 7;
        RadiatorRoute.Consumer = ETAVehicleDamageConsumerType::Radiator;
        RadiatorRoute.ImpactEnergyScale = 1.0;
        RadiatorRoute.FullCrushDisplacementM = 0.10;

        Config.DamageRouting.Routes.Add(RadiatorRoute);

        return Config;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTACrashToAlignmentAndRadiatorTest,
    "TorqueAtlas.Crash.EndToEnd.AlignmentAndRadiator",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTACrashToAlignmentAndRadiatorTest::RunTest(
    const FString& Parameters)
{
    FTAVehicleRuntimeConfig VehicleConfig;
    VehicleConfig.Chassis.MassKg = 1420.0;
    VehicleConfig.Chassis.PrincipalInertiaBodyKgm2 =
        FVector3d(650.0, 1800.0, 1900.0);

    // Lower thresholds keep this synthetic unit fixture compact.
    VehicleConfig.Radiator.PunctureThresholdEnergyJ = 50.0;
    VehicleConfig.Radiator.FullLeakEnergyJ = 500.0;

    FTAVehicleRuntimeState VehicleState;
    TADamage::InitializeRadiatorState(
        VehicleConfig.Radiator,
        VehicleState.Radiator);

    FTAChassisState ChassisState;

    TArray<FTAStructureNode> Nodes =
        MakeCrashStructureNodes();

    TArray<FTADistanceConstraint> Constraints;

    const FTACrashDamagePipelineConfig PipelineConfig =
        MakeCrashPipelineConfig();

    FTAStructureDamageBridgeState BridgeState;

    TestTrue(
        TEXT("Damage bridge initializes"),
        TAStructureDamageBridge::InitializeState(
            PipelineConfig.DamageBridge,
            Constraints.Num(),
            BridgeState));

    FTAStructureImpactScratch Scratch;
    Scratch.Initialize(32);

    FTADamageEventQueue Queue;
    Queue.Initialize(32);

    FTACrashDamagePipelineInput CrashInput;
    CrashInput.SimulationTick = 500;
    CrashInput.Substep = 1;
    CrashInput.StructureDeltaTimeSeconds = 1.0 / 120.0;

    CrashInput.Collision.ContactPointWorldM =
        FVector3d(1.62, 0.70, -0.42);

    // Frontal/right-corner barrier impulse.
    CrashInput.Collision.CollisionImpulseWorldNs =
        FVector3d(-4200.0, -650.0, 120.0);

    FTACrashDamagePipelineOutput CrashOutput;

    TestTrue(
        TEXT("Crash pipeline succeeds"),
        TACrashDamagePipeline::ProcessImpact(
            PipelineConfig,
            VehicleConfig,
            CrashInput,
            ChassisState,
            VehicleState,
            Nodes,
            Constraints,
            Scratch,
            BridgeState,
            Queue,
            CrashOutput));

    TestTrue(
        TEXT("Crash changes chassis velocity"),
        ChassisState.LinearVelocityWorldMps.Length() > 0.0);

    TestTrue(
        TEXT("Crash injects internal structural deformation"),
        CrashOutput.Collision.StructureImpact
            .InjectedDeformationKineticEnergyJ > 0.0);

    TestTrue(
        TEXT("Crash produces at least impact and/or mount damage signals"),
        CrashOutput.EmittedSignalCount >= 1);

    TestTrue(
        TEXT("Routed crash damages radiator"),
        VehicleState.Radiator.bPunctured
        || VehicleState.Radiator.AirflowEfficiency01 < 1.0);

    FTADoubleWishboneStructuralBindings SuspensionBindings;
    SuspensionBindings.LowerInnerA.NodeIndices = { 0 };
    SuspensionBindings.LowerInnerA.Weights = { 1.0 };
    SuspensionBindings.LowerInnerB.NodeIndices = { 1 };
    SuspensionBindings.LowerInnerB.Weights = { 1.0 };

    FTADoubleWishboneDamageOffsets DamageOffsets;

    TestTrue(
        TEXT("Structure displacement resolves into wishbone pickup offsets"),
        TASuspensionDamageBinding::ResolveDoubleWishboneDamageOffsets(
            MakeArrayView(Nodes),
            SuspensionBindings,
            DamageOffsets));

    TestTrue(
        TEXT("Crash physically moves at least one lower-arm pickup"),
        DamageOffsets.LowerInnerA.Length() > 0.001
        || DamageOffsets.LowerInnerB.Length() > 0.001);

    const FTADoubleWishboneSolverConfig Geometry =
        MakeCrashFrontRightGeometry();

    FTADoubleWishboneState ReferenceState;
    FTADoubleWishboneSolveInput ReferenceInput;
    FTADoubleWishboneSolveOutput ReferenceOutput;

    TestTrue(
        TEXT("Reference wishbone solves"),
        TADoubleWishboneSolver::Solve(
            Geometry,
            ReferenceInput,
            ReferenceState,
            ReferenceOutput));

    FTADoubleWishboneState DamagedState;
    FTADoubleWishboneSolveInput DamagedInput;
    DamagedInput.Damage = DamageOffsets;

    FTADoubleWishboneSolveOutput DamagedOutput;

    TestTrue(
        TEXT("Crash-damaged wishbone geometry solves"),
        TADoubleWishboneSolver::Solve(
            Geometry,
            DamagedInput,
            DamagedState,
            DamagedOutput));

    const double AlignmentChangeRad =
        FMath::Abs(
            DamagedOutput.CamberRad
            - ReferenceOutput.CamberRad)
        + FMath::Abs(
            DamagedOutput.ToeRad
            - ReferenceOutput.ToeRad);

    TestTrue(
        TEXT("Crash-induced pickup motion changes physical wheel alignment"),
        AlignmentChangeRad
            > FMath::DegreesToRadians(0.10));

    return true;
}

#endif
