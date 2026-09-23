#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TAStructureDamageBridge.h"

namespace
{
    TArray<FTAStructureNode> MakeDamageBridgeNodes()
    {
        TArray<FTAStructureNode> Nodes;
        Nodes.SetNum(4);

        Nodes[0].PositionM = FVector3d(0.0, 0.0, 0.0);
        Nodes[1].PositionM = FVector3d(1.0, 0.0, 0.0);
        Nodes[2].PositionM = FVector3d(0.0, 1.0, 0.0);
        Nodes[3].PositionM = FVector3d(0.0, 0.0, 1.0);

        TAStructureSolver::InitializeReferencePositionsFromCurrent(
            Nodes);

        return Nodes;
    }

    FTAStructureDamageBridgeConfig MakeBridgeConfig()
    {
        FTAStructureDamageBridgeConfig Config;

        Config.ConstraintTargetComponentIndices =
        {
            10,
            11
        };

        FTAStructureMountDamageBinding Mount;
        Mount.TargetComponentIndex = 42;
        Mount.NodeIndices = { 0, 2 };
        Mount.Weights = { 1.0, 1.0 };
        Mount.DisplacementThresholdsM =
        {
            0.005,
            0.015,
            0.030
        };

        Config.MountBindings.Add(Mount);
        Config.ImpactTargetComponentIndex = 99;

        return Config;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAStructureFractureSingleEventTest,
    "TorqueAtlas.Damage.StructureBridge.FractureEmitsOnce",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAStructureFractureSingleEventTest::RunTest(
    const FString& Parameters)
{
    TArray<FTAStructureNode> Nodes =
        MakeDamageBridgeNodes();

    TArray<FTADistanceConstraint> Constraints;
    Constraints.SetNum(2);

    Constraints[0].NodeA = 0;
    Constraints[0].NodeB = 1;
    Constraints[0].RestLengthM = 1.0;
    Constraints[0].bBroken = true;

    Constraints[1].NodeA = 0;
    Constraints[1].NodeB = 2;
    Constraints[1].RestLengthM = 1.0;

    const FTAStructureDamageBridgeConfig Config =
        MakeBridgeConfig();

    FTAStructureDamageBridgeState State;

    TestTrue(
        TEXT("Bridge state initializes"),
        TAStructureDamageBridge::InitializeState(
            Config,
            Constraints.Num(),
            State));

    FTADamageEventQueue Queue;
    Queue.Initialize(16);

    TestTrue(
        TEXT("First fracture extraction succeeds"),
        TAStructureDamageBridge::EmitStructureStateSignals(
            Config,
            100,
            0,
            MakeArrayView(Nodes),
            MakeArrayView(Constraints),
            State,
            Queue));

    TestEqual(
        TEXT("One fracture event emitted"),
        Queue.Num(),
        1);

    const FTADamageSignal First =
        Queue.GetEvents()[0];

    TestEqual(
        TEXT("Fracture type is structural fracture"),
        static_cast<uint8>(First.Type),
        static_cast<uint8>(
            ETADamageSignalType::StructuralFracture));

    TestEqual(
        TEXT("Constraint target component routes through config"),
        First.TargetComponentIndex,
        10);

    TestEqual(
        TEXT("Constraint index is preserved as source"),
        First.SourceElementIndex,
        0);

    TestTrue(
        TEXT("Repeated extraction succeeds"),
        TAStructureDamageBridge::EmitStructureStateSignals(
            Config,
            101,
            0,
            MakeArrayView(Nodes),
            MakeArrayView(Constraints),
            State,
            Queue));

    TestEqual(
        TEXT("Already-reported fracture is not emitted again"),
        Queue.Num(),
        1);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAStructureMountThresholdTest,
    "TorqueAtlas.Damage.StructureBridge.MountThresholdCrossings",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAStructureMountThresholdTest::RunTest(
    const FString& Parameters)
{
    TArray<FTAStructureNode> Nodes =
        MakeDamageBridgeNodes();

    TArray<FTADistanceConstraint> Constraints;
    Constraints.SetNum(2);

    const FTAStructureDamageBridgeConfig Config =
        MakeBridgeConfig();

    FTAStructureDamageBridgeState State;

    TestTrue(
        TEXT("Bridge state initializes"),
        TAStructureDamageBridge::InitializeState(
            Config,
            Constraints.Num(),
            State));

    FTADamageEventQueue Queue;
    Queue.Initialize(16);

    Nodes[0].PositionM.Y += 0.006;
    Nodes[2].PositionM.Y += 0.006;

    TestTrue(
        TEXT("First displacement threshold extraction succeeds"),
        TAStructureDamageBridge::EmitStructureStateSignals(
            Config,
            200,
            0,
            MakeArrayView(Nodes),
            MakeArrayView(Constraints),
            State,
            Queue));

    TestEqual(
        TEXT("First displacement crossing emits one event"),
        Queue.Num(),
        1);

    TestEqual(
        TEXT("Mount displacement routes target component"),
        Queue.GetEvents()[0].TargetComponentIndex,
        42);

    TestTrue(
        TEXT("Same displacement level does not spam another event"),
        TAStructureDamageBridge::EmitStructureStateSignals(
            Config,
            201,
            0,
            MakeArrayView(Nodes),
            MakeArrayView(Constraints),
            State,
            Queue));

    TestEqual(
        TEXT("No duplicate event at same threshold level"),
        Queue.Num(),
        1);

    Nodes[0].PositionM.Y =
        Nodes[0].ReferencePositionM.Y + 0.020;

    Nodes[2].PositionM.Y =
        Nodes[2].ReferencePositionM.Y + 0.020;

    TestTrue(
        TEXT("Higher displacement threshold extraction succeeds"),
        TAStructureDamageBridge::EmitStructureStateSignals(
            Config,
            202,
            0,
            MakeArrayView(Nodes),
            MakeArrayView(Constraints),
            State,
            Queue));

    TestEqual(
        TEXT("Crossing a higher displacement level emits one new event"),
        Queue.Num(),
        2);

    TestTrue(
        TEXT("New event contains actual displacement vector"),
        Queue.GetEvents()[1].VectorValue.Y > 0.019);

    TestTrue(
        TEXT("Signal sequence IDs advance deterministically"),
        Queue.GetEvents()[1].SequenceId
            == Queue.GetEvents()[0].SequenceId + 1);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAStructureImpactEnergySignalTest,
    "TorqueAtlas.Damage.StructureBridge.ImpactEnergy",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAStructureImpactEnergySignalTest::RunTest(
    const FString& Parameters)
{
    const FTAStructureDamageBridgeConfig Config =
        MakeBridgeConfig();

    FTAStructureDamageBridgeState State;

    TestTrue(
        TEXT("Bridge state initializes"),
        TAStructureDamageBridge::InitializeState(
            Config,
            0,
            State));

    FTADamageEventQueue Queue;
    Queue.Initialize(4);

    FTAStructureImpactOutput Impact;
    Impact.bApplied = true;
    Impact.InjectedDeformationKineticEnergyJ = 4200.0;

    TestTrue(
        TEXT("Impact energy signal emits"),
        TAStructureDamageBridge::EmitImpactEnergySignal(
            Config,
            300,
            2,
            Impact,
            State,
            Queue));

    TestEqual(
        TEXT("One impact event emitted"),
        Queue.Num(),
        1);

    const FTADamageSignal Signal =
        Queue.GetEvents()[0];

    TestEqual(
        TEXT("Impact signal type"),
        static_cast<uint8>(Signal.Type),
        static_cast<uint8>(
            ETADamageSignalType::ImpactEnergy));

    TestTrue(
        TEXT("Impact signal carries deformation energy"),
        FMath::IsNearlyEqual(
            Signal.ScalarValue,
            4200.0,
            1.0e-9));

    TestEqual(
        TEXT("Impact target component routes through config"),
        Signal.TargetComponentIndex,
        99);

    return true;
}

#endif
