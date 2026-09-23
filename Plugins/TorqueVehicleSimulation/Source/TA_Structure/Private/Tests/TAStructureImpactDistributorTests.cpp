#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TAStructureImpactDistributor.h"

namespace
{
    TArray<FTAStructureNode> MakeCubeNodes()
    {
        TArray<FTAStructureNode> Nodes;
        Nodes.Reserve(8);

        for (int32 XSign : { -1, 1 })
        {
            for (int32 YSign : { -1, 1 })
            {
                for (int32 ZSign : { -1, 1 })
                {
                    FTAStructureNode Node;
                    Node.PositionM =
                        FVector3d(
                            0.5 * XSign,
                            0.5 * YSign,
                            0.5 * ZSign);

                    Node.PreviousPositionM =
                        Node.PositionM;

                    Node.ReferencePositionM =
                        Node.PositionM;

                    // 10 kg per structural node.
                    Node.InverseMassPerKg = 0.1;

                    Nodes.Add(Node);
                }
            }
        }

        return Nodes;
    }

    FVector3d SumNodeMomentumDelta(
        const TArray<FTAStructureNode>& Before,
        const TArray<FTAStructureNode>& After)
    {
        FVector3d Sum =
            FVector3d::ZeroVector;

        for (int32 Index = 0;
             Index < Before.Num();
             ++Index)
        {
            const double MassKg =
                1.0
                / Before[Index].InverseMassPerKg;

            Sum +=
                (After[Index].VelocityMps
                    - Before[Index].VelocityMps)
                * MassKg;
        }

        return Sum;
    }

    FVector3d SumNodeAngularMomentumDelta(
        const TArray<FTAStructureNode>& Before,
        const TArray<FTAStructureNode>& After)
    {
        FVector3d CenterOfMass =
            FVector3d::ZeroVector;

        double TotalMassKg = 0.0;

        for (const FTAStructureNode& Node : Before)
        {
            const double MassKg =
                1.0 / Node.InverseMassPerKg;

            CenterOfMass +=
                Node.PositionM * MassKg;

            TotalMassKg +=
                MassKg;
        }

        CenterOfMass /=
            TotalMassKg;

        FVector3d Sum =
            FVector3d::ZeroVector;

        for (int32 Index = 0;
             Index < Before.Num();
             ++Index)
        {
            const double MassKg =
                1.0
                / Before[Index].InverseMassPerKg;

            const FVector3d DeltaMomentumNs =
                (After[Index].VelocityMps
                    - Before[Index].VelocityMps)
                * MassKg;

            Sum +=
                FVector3d::CrossProduct(
                    Before[Index].PositionM
                        - CenterOfMass,
                    DeltaMomentumNs);
        }

        return Sum;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAStructureImpactMomentumNeutralityTest,
    "TorqueAtlas.Structure.Impact.InternalModesPreserveRigidMomentum",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAStructureImpactMomentumNeutralityTest::RunTest(
    const FString& Parameters)
{
    TArray<FTAStructureNode> Nodes =
        MakeCubeNodes();

    const TArray<FTAStructureNode> Before =
        Nodes;

    FTAStructureImpactConfig Config;
    Config.DistributionRadiusM = 2.0;
    Config.DistanceFalloffExponent = 2.0;
    Config.DeformationImpulseFraction01 = 0.35;
    Config.MaxNodeDeltaVelocityMps = 100.0;

    FTAStructureImpactInput Input;
    Input.ContactPointLocalM =
        FVector3d(0.65, 0.15, 0.10);

    Input.CollisionImpulseLocalNs =
        FVector3d(-1200.0, 250.0, 100.0);

    FTAStructureImpactScratch Scratch;
    Scratch.Initialize(16);

    FTAStructureImpactOutput Output;

    TestTrue(
        TEXT("Internal impact distribution succeeds"),
        TAStructureImpactDistributor::DistributeImpactAsInternalDeformation(
            Config,
            Input,
            Nodes,
            Scratch,
            Output));

    TestTrue(
        TEXT("Multiple structural nodes receive deformation mode"),
        Output.AffectedNodeCount >= 4);

    const FVector3d LinearMomentumDelta =
        SumNodeMomentumDelta(
            Before,
            Nodes);

    const FVector3d AngularMomentumDelta =
        SumNodeAngularMomentumDelta(
            Before,
            Nodes);

    TestTrue(
        TEXT("Internal deformation injects near-zero net linear momentum"),
        LinearMomentumDelta.Length() < 1.0e-7);

    TestTrue(
        TEXT("Internal deformation injects near-zero net angular momentum"),
        AngularMomentumDelta.Length() < 1.0e-7);

    TestTrue(
        TEXT("Distributor telemetry reports near-zero residual linear impulse"),
        Output.ResidualLinearImpulseNs.Length() < 1.0e-7);

    TestTrue(
        TEXT("Distributor telemetry reports near-zero residual angular impulse"),
        Output.ResidualAngularImpulseNms.Length() < 1.0e-7);

    TestTrue(
        TEXT("Impact still injects non-zero internal deformation energy"),
        Output.InjectedDeformationKineticEnergyJ > 0.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAStructureImpactSpatialWeightingTest,
    "TorqueAtlas.Structure.Impact.SpatialWeightingCreatesDeformation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAStructureImpactSpatialWeightingTest::RunTest(
    const FString& Parameters)
{
    TArray<FTAStructureNode> Nodes =
        MakeCubeNodes();

    FTAStructureImpactConfig Config;
    Config.DistributionRadiusM = 1.5;
    Config.DistanceFalloffExponent = 3.0;
    Config.DeformationImpulseFraction01 = 0.5;
    Config.MaxNodeDeltaVelocityMps = 100.0;

    FTAStructureImpactInput Input;
    Input.ContactPointLocalM =
        FVector3d(0.65, 0.45, 0.45);

    Input.CollisionImpulseLocalNs =
        FVector3d(-900.0, 0.0, 0.0);

    FTAStructureImpactScratch Scratch;
    Scratch.Initialize(16);

    FTAStructureImpactOutput Output;

    TestTrue(
        TEXT("Spatially weighted impact distributes"),
        TAStructureImpactDistributor::DistributeImpactAsInternalDeformation(
            Config,
            Input,
            Nodes,
            Scratch,
            Output));

    double MinVelocityMps =
        TNumericLimits<double>::Max();

    double MaxVelocityMps = 0.0;

    for (const FTAStructureNode& Node : Nodes)
    {
        const double SpeedMps =
            Node.VelocityMps.Length();

        MinVelocityMps =
            FMath::Min(
                MinVelocityMps,
                SpeedMps);

        MaxVelocityMps =
            FMath::Max(
                MaxVelocityMps,
                SpeedMps);
    }

    TestTrue(
        TEXT("Spatial weighting creates non-uniform internal node motion"),
        MaxVelocityMps - MinVelocityMps > 0.01);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAStructureImpactVelocityLimitTest,
    "TorqueAtlas.Structure.Impact.UniformVelocityLimit",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAStructureImpactVelocityLimitTest::RunTest(
    const FString& Parameters)
{
    TArray<FTAStructureNode> Nodes =
        MakeCubeNodes();

    FTAStructureImpactConfig Config;
    Config.DistributionRadiusM = 2.0;
    Config.DeformationImpulseFraction01 = 1.0;
    Config.MaxNodeDeltaVelocityMps = 2.0;

    FTAStructureImpactInput Input;
    Input.ContactPointLocalM =
        FVector3d(0.7, 0.3, 0.2);

    Input.CollisionImpulseLocalNs =
        FVector3d(-100000.0, 5000.0, 1000.0);

    FTAStructureImpactScratch Scratch;
    Scratch.Initialize(16);

    FTAStructureImpactOutput Output;

    TestTrue(
        TEXT("Large impact distributes"),
        TAStructureImpactDistributor::DistributeImpactAsInternalDeformation(
            Config,
            Input,
            Nodes,
            Scratch,
            Output));

    for (const FTAStructureNode& Node : Nodes)
    {
        TestTrue(
            TEXT("Uniform scaling respects node delta-velocity ceiling"),
            Node.VelocityMps.Length()
                <= Config.MaxNodeDeltaVelocityMps
                + 1.0e-9);
    }

    TestTrue(
        TEXT("Velocity limiting preserves zero residual linear impulse"),
        Output.ResidualLinearImpulseNs.Length()
            < 1.0e-7);

    TestTrue(
        TEXT("Velocity limiting preserves zero residual angular impulse"),
        Output.ResidualAngularImpulseNms.Length()
            < 1.0e-7);

    return true;
}

#endif
