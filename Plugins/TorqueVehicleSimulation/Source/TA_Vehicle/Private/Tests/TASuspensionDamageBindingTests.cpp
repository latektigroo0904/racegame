#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TASuspensionDamageBinding.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTASuspensionDamageBindingTest,
    "TorqueAtlas.Suspension.DamageBinding.StructureToPickupOffset",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTASuspensionDamageBindingTest::RunTest(const FString& Parameters)
{
    TArray<FTAStructureNode> Nodes;
    Nodes.SetNum(6);

    for (int32 Index = 0; Index < Nodes.Num(); ++Index)
    {
        Nodes[Index].PositionM =
            FVector3d(
                static_cast<double>(Index),
                0.0,
                0.0);
    }

    TAStructureSolver::InitializeReferencePositionsFromCurrent(Nodes);

    Nodes[0].PositionM.Y -= 0.020;
    Nodes[1].PositionM.Y -= 0.018;
    Nodes[5].PositionM.Z += 0.010;

    FTADoubleWishboneStructuralBindings Bindings;

    Bindings.UpperInnerA.NodeIndices.Add(0);
    Bindings.UpperInnerA.Weights.Add(1.0);

    Bindings.UpperInnerB.NodeIndices.Add(1);
    Bindings.UpperInnerB.Weights.Add(1.0);

    Bindings.LowerInnerA.NodeIndices.Add(2);
    Bindings.LowerInnerA.Weights.Add(1.0);

    Bindings.LowerInnerB.NodeIndices.Add(3);
    Bindings.LowerInnerB.Weights.Add(1.0);

    Bindings.TieRodInner.NodeIndices.Add(4);
    Bindings.TieRodInner.Weights.Add(1.0);

    Bindings.DamperChassis.NodeIndices.Add(5);
    Bindings.DamperChassis.Weights.Add(1.0);

    FTADoubleWishboneDamageOffsets Offsets;

    TestTrue(
        TEXT("Bindings resolve"),
        TASuspensionDamageBinding::ResolveDoubleWishboneDamageOffsets(
            MakeArrayView(Nodes),
            Bindings,
            Offsets));

    TestTrue(
        TEXT("Upper A displacement follows bound structure node"),
        FMath::IsNearlyEqual(
            Offsets.UpperInnerA.Y,
            -0.020,
            1.0e-9));

    TestTrue(
        TEXT("Upper B displacement follows bound structure node"),
        FMath::IsNearlyEqual(
            Offsets.UpperInnerB.Y,
            -0.018,
            1.0e-9));

    TestTrue(
        TEXT("Undeformed lower pickup remains at zero offset"),
        Offsets.LowerInnerA.IsNearlyZero());

    TestTrue(
        TEXT("Damper chassis displacement follows structural node"),
        FMath::IsNearlyEqual(
            Offsets.DamperChassis.Z,
            0.010,
            1.0e-9));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTASuspensionDamageBindingWeightedTest,
    "TorqueAtlas.Suspension.DamageBinding.WeightedNodeAverage",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTASuspensionDamageBindingWeightedTest::RunTest(const FString& Parameters)
{
    TArray<FTAStructureNode> Nodes;
    Nodes.SetNum(2);

    Nodes[0].PositionM = FVector3d::ZeroVector;
    Nodes[1].PositionM = FVector3d(1.0, 0.0, 0.0);

    TAStructureSolver::InitializeReferencePositionsFromCurrent(Nodes);

    Nodes[0].PositionM.Z += 0.010;
    Nodes[1].PositionM.Z += 0.030;

    FTAStructureDisplacementBinding Binding;
    Binding.NodeIndices = { 0, 1 };
    Binding.Weights = { 1.0, 3.0 };

    FVector3d Displacement;

    TestTrue(
        TEXT("Weighted binding resolves"),
        TASuspensionDamageBinding::ResolveBindingDisplacement(
            MakeArrayView(Nodes),
            Binding,
            Displacement));

    TestTrue(
        TEXT("Weighted displacement is normalized average"),
        FMath::IsNearlyEqual(
            Displacement.Z,
            0.025,
            1.0e-9));

    return true;
}

#endif
