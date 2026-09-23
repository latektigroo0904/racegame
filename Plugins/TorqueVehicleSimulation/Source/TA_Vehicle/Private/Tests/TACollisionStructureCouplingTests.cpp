#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TACollisionStructureCoupling.h"

namespace
{
    TArray<FTAStructureNode> MakeCollisionStructureNodes()
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

                    Node.ReferencePositionM =
                        Node.PositionM;

                    Node.PreviousPositionM =
                        Node.PositionM;

                    Node.InverseMassPerKg = 0.1;
                    Nodes.Add(Node);
                }
            }
        }

        return Nodes;
    }

    FVector3d CalculateStructureMomentum(
        const TArray<FTAStructureNode>& Nodes)
    {
        FVector3d Momentum =
            FVector3d::ZeroVector;

        for (const FTAStructureNode& Node : Nodes)
        {
            const double MassKg =
                1.0 / Node.InverseMassPerKg;

            Momentum +=
                Node.VelocityMps * MassKg;
        }

        return Momentum;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTACollisionStructureCouplingTest,
    "TorqueAtlas.Collision.ChassisAndStructure.NoMomentumDoubleCount",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTACollisionStructureCouplingTest::RunTest(
    const FString& Parameters)
{
    FTAChassisConfig ChassisConfig;
    ChassisConfig.MassKg = 1000.0;
    ChassisConfig.PrincipalInertiaBodyKgm2 =
        FVector3d(500.0, 800.0, 900.0);

    FTAChassisState ChassisState;

    TArray<FTAStructureNode> Nodes =
        MakeCollisionStructureNodes();

    FTAStructureImpactScratch Scratch;
    Scratch.Initialize(16);

    FTACollisionStructureCouplingConfig Config;
    Config.StructureImpact.DistributionRadiusM = 2.0;
    Config.StructureImpact.DeformationImpulseFraction01 = 0.35;
    Config.StructureImpact.MaxNodeDeltaVelocityMps = 100.0;

    FTACollisionStructureInput Input;
    Input.ContactPointWorldM =
        FVector3d(0.0, 1.0, 0.0);

    Input.CollisionImpulseWorldNs =
        FVector3d(1000.0, 0.0, 0.0);

    FTACollisionStructureOutput Output;

    TestTrue(
        TEXT("Collision coupling succeeds"),
        TACollisionStructureCoupling::ApplyCollisionImpact(
            Config,
            ChassisConfig,
            Input,
            ChassisState,
            Nodes,
            Scratch,
            Output));

    TestTrue(
        TEXT("Full external impulse changes chassis linear velocity"),
        FMath::IsNearlyEqual(
            ChassisState.LinearVelocityWorldMps.X,
            1.0,
            1.0e-9));

    TestTrue(
        TEXT("Off-center external impulse changes chassis yaw rate"),
        ChassisState.AngularVelocityWorldRadPerSec.Z < 0.0);

    TestTrue(
        TEXT("Internal deformation mode is also generated"),
        Output.bInternalDeformationApplied);

    TestTrue(
        TEXT("Internal structure adds no duplicate net linear momentum"),
        CalculateStructureMomentum(Nodes).Length()
            < 1.0e-7);

    TestTrue(
        TEXT("Internal impact contains deformation energy"),
        Output.StructureImpact.InjectedDeformationKineticEnergyJ
            > 0.0);

    TestTrue(
        TEXT("Structure distributor residual linear impulse is near zero"),
        Output.StructureImpact.ResidualLinearImpulseNs.Length()
            < 1.0e-7);

    TestTrue(
        TEXT("Structure distributor residual angular impulse is near zero"),
        Output.StructureImpact.ResidualAngularImpulseNms.Length()
            < 1.0e-7);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAChassisCenterImpulseTest,
    "TorqueAtlas.Collision.Chassis.CenterImpulse",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAChassisCenterImpulseTest::RunTest(
    const FString& Parameters)
{
    FTAChassisConfig Config;
    Config.MassKg = 1200.0;

    FTAChassisState State;
    State.PositionWorldM =
        FVector3d(2.0, -1.0, 0.5);

    TestTrue(
        TEXT("Center impulse applies"),
        TAChassisDynamics::ApplyImpulseAtWorldPoint(
            Config,
            FVector3d(600.0, 0.0, 0.0),
            State.PositionWorldM,
            State));

    TestTrue(
        TEXT("Center impulse changes linear velocity by J/m"),
        FMath::IsNearlyEqual(
            State.LinearVelocityWorldMps.X,
            0.5,
            1.0e-9));

    TestTrue(
        TEXT("Center impulse creates no angular velocity"),
        State.AngularVelocityWorldRadPerSec.IsNearlyZero());

    return true;
}

#endif
