#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TACompiledDamageRuntime.h"
#include "TAVehicleDefinition.h"

namespace
{
    void AddCompactFrontStructure(
        UTAVehicleDefinition& Definition)
    {
        const FVector Positions[] =
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

        for (const FVector& Position : Positions)
        {
            FTAStructureNodeAuthoringDefinition Node;
            Node.PositionVehicleLocalM = Position;
            Node.MassKg = 18.0;
            Definition.Structure.Nodes.Add(Node);
        }

        Definition.Structure.ImpactDistributionRadiusM = 0.85;
        Definition.Structure.ImpactDistanceFalloffExponent = 2.0;
        Definition.Structure.DeformationImpulseFraction01 = 0.35;
        Definition.Structure.MaxNodeDeltaVelocityMps = 1.5;

        Definition.Structure
            .FrontRightSuspensionBindings
            .LowerInnerA.NodeIndices = { 0 };

        Definition.Structure
            .FrontRightSuspensionBindings
            .LowerInnerA.Weights = { 1.0 };

        Definition.Structure
            .FrontRightSuspensionBindings
            .LowerInnerB.NodeIndices = { 1 };

        Definition.Structure
            .FrontRightSuspensionBindings
            .LowerInnerB.Weights = { 1.0 };

        FTAStructureMountDamageAuthoringDefinition Mount;
        Mount.TargetComponentIndex = 42;
        Mount.NodeIndices = { 0 };
        Mount.Weights = { 1.0 };
        Mount.DisplacementThresholdsM = { 0.0005, 0.003, 0.010 };
        Definition.Structure.MountDamageBindings.Add(Mount);
    }

    FTARoadPlane MakeFlatRoad()
    {
        FTARoadPlane Road;
        Road.NormalWorld =
            FVector3d(0.0, 0.0, 1.0);
        Road.Surface.Material =
            ETASurfaceMaterial::FreshAsphalt;
        return Road;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTACompiledDamageRuntimeInjectionTest,
    "TorqueAtlas.Crash.CompiledDamageRuntime.InjectsSuspensionOffsets",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTACompiledDamageRuntimeInjectionTest::RunTest(
    const FString& Parameters)
{
    UTAVehicleDefinition* Definition =
        NewObject<UTAVehicleDefinition>();

    AddCompactFrontStructure(
        *Definition);

    FTAVehicleCompiledConfig Config;
    FTAValidationResult Validation;

    TestTrue(
        TEXT("Structured vehicle definition compiles"),
        Definition->BuildCompiledConfig(
            Config,
            Validation));

    FTAFourWheelRuntimeState VehicleState;

    TestTrue(
        TEXT("Four-wheel runtime initializes"),
        TAFourWheelVehicleRuntime::Initialize(
            Config.VehicleRuntime,
            VehicleState));

    FTACompiledDamageRuntimeState DamageState;

    TestTrue(
        TEXT("Compiled damage runtime initializes"),
        TACompiledDamageRuntime::Initialize(
            Config,
            64,
            DamageState));

    FTACrashDamagePipelineInput CrashInput;
    CrashInput.SimulationTick = 900;
    CrashInput.Substep = 0;
    CrashInput.StructureDeltaTimeSeconds = 1.0 / 240.0;
    CrashInput.Collision.ContactPointWorldM =
        FVector3d(1.62, 0.70, -0.42);

    CrashInput.Collision.CollisionImpulseWorldNs =
        FVector3d(-2600.0, -450.0, 80.0);

    FTACrashDamagePipelineOutput CrashOutput;

    TestTrue(
        TEXT("Persistent compiled damage runtime processes crash"),
        TACompiledDamageRuntime::ProcessCrash(
            Config,
            CrashInput,
            VehicleState,
            DamageState,
            CrashOutput));

    FTAFourWheelStepInput DamagedStepInput;

    DamagedStepInput.FrontLeftRoad =
        MakeFlatRoad();

    DamagedStepInput.FrontRightRoad =
        MakeFlatRoad();

    DamagedStepInput.RearLeftRoad =
        MakeFlatRoad();

    DamagedStepInput.RearRightRoad =
        MakeFlatRoad();

    TestTrue(
        TEXT("Current structure maps into next four-wheel step"),
        TACompiledDamageRuntime::ApplyCurrentStructureToFourWheelInput(
            Config,
            DamageState,
            DamagedStepInput));

    TestTrue(
        TEXT("Crash-derived front-right lower-arm offsets are non-zero"),
        DamagedStepInput.FrontRightDamage.LowerInnerA.Length()
            > 0.0005
        || DamagedStepInput.FrontRightDamage.LowerInnerB.Length()
            > 0.0005);

    // Compare undamaged and damaged alignment at the same clean chassis pose.
    FTAFrontAxleRuntimeState UndamagedAxleState;
    FTATireRuntimeState UndamagedLeftTire;
    FTATireRuntimeState UndamagedRightTire;
    FTAFrontAxleSolveOutput UndamagedOutput;

    FTAFrontAxleSolveInput UndamagedInput;
    UndamagedInput.LeftRoad = MakeFlatRoad();
    UndamagedInput.RightRoad = MakeFlatRoad();

    FTAChassisState ReferenceChassis;
    ReferenceChassis.PositionWorldM =
        FVector3d(0.0, 0.0, 0.777);

    TestTrue(
        TEXT("Undamaged reference front axle solves"),
        TAFrontAxleRuntime::ResolveWithTireCompliance(
            ReferenceChassis,
            Config.FourWheelRuntime.FrontAxle,
            UndamagedInput,
            Config.VehicleRuntime.Tires[0],
            Config.VehicleRuntime.Tires[1],
            1.0 / 240.0,
            UndamagedAxleState,
            UndamagedLeftTire,
            UndamagedRightTire,
            UndamagedOutput));

    FTAFrontAxleRuntimeState DamagedAxleState;
    FTATireRuntimeState DamagedLeftTire;
    FTATireRuntimeState DamagedRightTire;
    FTAFrontAxleSolveOutput DamagedOutput;

    FTAFrontAxleSolveInput DamagedInput;
    DamagedInput.LeftRoad = MakeFlatRoad();
    DamagedInput.RightRoad = MakeFlatRoad();
    DamagedInput.LeftDamage =
        DamagedStepInput.FrontLeftDamage;
    DamagedInput.RightDamage =
        DamagedStepInput.FrontRightDamage;

    TestTrue(
        TEXT("Crash-damaged front axle solves from persistent structure state"),
        TAFrontAxleRuntime::ResolveWithTireCompliance(
            ReferenceChassis,
            Config.FourWheelRuntime.FrontAxle,
            DamagedInput,
            Config.VehicleRuntime.Tires[0],
            Config.VehicleRuntime.Tires[1],
            1.0 / 240.0,
            DamagedAxleState,
            DamagedLeftTire,
            DamagedRightTire,
            DamagedOutput));

    const double RightAlignmentDeltaRad =
        FMath::Abs(
            DamagedOutput.RightContact.Geometry.CamberRad
            - UndamagedOutput.RightContact.Geometry.CamberRad)
        + FMath::Abs(
            DamagedOutput.RightContact.Geometry.ToeRad
            - UndamagedOutput.RightContact.Geometry.ToeRad);

    TestTrue(
        TEXT("Persistent crash deformation changes next-step wheel alignment"),
        RightAlignmentDeltaRad
            > FMath::DegreesToRadians(0.05));

    return true;
}

#endif
