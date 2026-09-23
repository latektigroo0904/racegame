#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TAExperimentalUnsprungCorner.h"

namespace
{
    FTAExperimentalUnsprungCornerConfig MakeDynamicCornerConfig(
        const bool bUseGravity)
    {
        FTAExperimentalUnsprungCornerConfig Config;

        FTADoubleWishboneHardpoints& H =
            Config.Geometry.Hardpoints;

        H.UpperInnerA =
            FVector3d(1.48, 0.38, -0.22);
        H.UpperInnerB =
            FVector3d(1.14, 0.38, -0.22);

        H.LowerInnerA =
            FVector3d(1.50, 0.35, -0.48);
        H.LowerInnerB =
            FVector3d(1.12, 0.35, -0.48);

        H.TieRodInner =
            FVector3d(1.12, 0.35, -0.38);

        H.DamperChassis =
            FVector3d(1.31, 0.40, -0.05);

        H.DamperLowerArmReference =
            FVector3d(1.31, 0.52, -0.43);

        H.UpperBallJointReference =
            FVector3d(1.31, 0.70, -0.25);

        H.LowerBallJointReference =
            FVector3d(1.31, 0.73, -0.50);

        H.TieRodOuterReference =
            FVector3d(1.12, 0.71, -0.39);

        H.WheelCenterReference =
            FVector3d(1.31, 0.775, -0.45);

        H.WheelForwardReference =
            FVector3d(1.0, 0.0, 0.0);

        H.WheelUpReference =
            FVector3d(0.0, 0.0, 1.0);

        H.SteeringRackAxisLocal =
            FVector3d(0.0, 1.0, 0.0);

        H.SideSign =
            1.0;

        Config.Geometry.MinTravelM =
            -0.07;

        Config.Geometry.MaxTravelM =
            0.09;

        Config.Geometry.MaxIterations =
            100;

        Config.Geometry.PositionToleranceM =
            0.0005;

        Config.Suspension.SpringRateNPerM =
            95000.0;

        Config.Suspension.StaticSpringCompressionM =
            0.08;

        Config.Suspension.BumpDampingNsPerM =
            4500.0;

        Config.Suspension.ReboundDampingNsPerM =
            6500.0;

        Config.Suspension.BumpStopTravelM =
            0.09;

        Config.Suspension.DroopStopTravelM =
            -0.07;

        Config.Unsprung.EffectiveMassKg =
            42.0;

        Config.Unsprung.MinTravelM =
            Config.Geometry.MinTravelM;

        Config.Unsprung.MaxTravelM =
            Config.Geometry.MaxTravelM;

        Config.Unsprung.TravelLimitRestitution01 =
            0.0;

        Config.Unsprung.MaxAbsTravelVelocityMps =
            8.0;

        Config.InternalSubsteps =
            4;

        Config.GravityWorldMps2 =
            bUseGravity
            ? FVector3d(0.0, 0.0, -9.80665)
            : FVector3d::ZeroVector;

        return Config;
    }

    FTAExperimentalUnsprungCornerInput MakeDynamicCornerInput()
    {
        FTAExperimentalUnsprungCornerInput Input;

        Input.Chassis.PositionWorldM =
            FVector3d(0.0, 0.0, 0.777);

        Input.Road.PointWorldM =
            FVector3d::ZeroVector;

        Input.Road.NormalWorld =
            FVector3d(0.0, 0.0, 1.0);

        Input.Road.Surface.Material =
            ETASurfaceMaterial::FreshAsphalt;

        return Input;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAExperimentalUnsprungQuasiStaticSeedTest,
    "TorqueAtlas.Suspension.UnsprungCorner.QuasiStaticSeed",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)

bool FTAExperimentalUnsprungQuasiStaticSeedTest::RunTest(
    const FString& Parameters)
{
    const FTAExperimentalUnsprungCornerConfig Config =
        MakeDynamicCornerConfig(false);

    const FTAExperimentalUnsprungCornerInput Input =
        MakeDynamicCornerInput();

    FTAExperimentalUnsprungCornerState State;
    FTAExperimentalUnsprungCornerOutput Output;

    TestTrue(
        TEXT("Dynamic corner initializes from canonical equilibrium"),
        TAExperimentalUnsprungCorner::InitializeFromQuasiStatic(
            Config,
            Input,
            1.0 / 240.0,
            State,
            Output));

    TestTrue(
        TEXT("Seeded corner is in contact"),
        Output.bInContact);

    TestTrue(
        TEXT("Seeded tire normal force is positive"),
        Output.TireNormalForceN > 100.0);

    TestTrue(
        TEXT("Seeded chassis suspension reaction is positive"),
        Output.ChassisSuspensionReactionN > 100.0);

    TestTrue(
        TEXT("Zero-gravity canonical seed starts close to force equilibrium"),
        FMath::Abs(
            Output.TireNormalForceN
            - Output.ChassisSuspensionReactionN)
            < 10.0);

    const double InitialTravelM =
        State.Unsprung.TravelM;

    TestTrue(
        TEXT("First dynamic step succeeds"),
        TAExperimentalUnsprungCorner::Step(
            Config,
            Input,
            1.0 / 240.0,
            State,
            Output));

    TestTrue(
        TEXT("Zero-gravity equilibrium has small initial acceleration"),
        FMath::Abs(
            Output.Unsprung.TravelAccelerationMps2)
            < 0.5);

    TestTrue(
        TEXT("Zero-gravity first step remains near quasi-static travel"),
        FMath::Abs(
            State.Unsprung.TravelM
            - InitialTravelM)
            < 0.001);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAExperimentalUnsprungRoadStepTest,
    "TorqueAtlas.Suspension.UnsprungCorner.RoadStep20mm",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)

bool FTAExperimentalUnsprungRoadStepTest::RunTest(
    const FString& Parameters)
{
    const FTAExperimentalUnsprungCornerConfig Config =
        MakeDynamicCornerConfig(false);

    FTAExperimentalUnsprungCornerInput Input =
        MakeDynamicCornerInput();

    FTAExperimentalUnsprungCornerState State;
    FTAExperimentalUnsprungCornerOutput Output;

    TestTrue(
        TEXT("Road-step fixture initializes"),
        TAExperimentalUnsprungCorner::InitializeFromQuasiStatic(
            Config,
            Input,
            1.0 / 240.0,
            State,
            Output));

    const double InitialTravelM =
        State.Unsprung.TravelM;

    Input.Road.PointWorldM.Z +=
        0.020;

    double MaximumTravelM =
        State.Unsprung.TravelM;

    double MaximumAbsVelocityMps =
        0.0;

    for (int32 StepIndex = 0;
         StepIndex < 480;
         ++StepIndex)
    {
        TestTrue(
            TEXT("Dynamic road-step integration succeeds"),
            TAExperimentalUnsprungCorner::Step(
                Config,
                Input,
                1.0 / 240.0,
                State,
                Output));

        MaximumTravelM =
            FMath::Max(
                MaximumTravelM,
                State.Unsprung.TravelM);

        MaximumAbsVelocityMps =
            FMath::Max(
                MaximumAbsVelocityMps,
                FMath::Abs(
                    State.Unsprung.TravelVelocityMps));
    }

    TestTrue(
        TEXT("20 mm road rise produces bump-direction unsprung response"),
        MaximumTravelM
            > InitialTravelM + 0.001);

    TestTrue(
        TEXT("Road-step response remains inside configured travel"),
        State.Unsprung.TravelM
            >= Config.Unsprung.MinTravelM
        && State.Unsprung.TravelM
            <= Config.Unsprung.MaxTravelM);

    TestTrue(
        TEXT("Road-step velocity remains bounded"),
        MaximumAbsVelocityMps
            <= Config.Unsprung.MaxAbsTravelVelocityMps);

    TestTrue(
        TEXT("Road-step transient damps toward low relative velocity"),
        FMath::Abs(
            State.Unsprung.TravelVelocityMps)
            < 0.5);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAExperimentalUnsprungForceOwnershipTest,
    "TorqueAtlas.Suspension.UnsprungCorner.NoNormalForceDoubleCount",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)

bool FTAExperimentalUnsprungForceOwnershipTest::RunTest(
    const FString& Parameters)
{
    const FTAExperimentalUnsprungCornerConfig Config =
        MakeDynamicCornerConfig(true);

    const FTAExperimentalUnsprungCornerInput Input =
        MakeDynamicCornerInput();

    FTAExperimentalUnsprungCornerState State;
    FTAExperimentalUnsprungCornerOutput Output;

    TestTrue(
        TEXT("Gravity fixture initializes"),
        TAExperimentalUnsprungCorner::InitializeFromQuasiStatic(
            Config,
            Input,
            1.0 / 240.0,
            State,
            Output));

    for (int32 StepIndex = 0;
         StepIndex < 1200;
         ++StepIndex)
    {
        TestTrue(
            TEXT("Gravity settle step succeeds"),
            TAExperimentalUnsprungCorner::Step(
                Config,
                Input,
                1.0 / 240.0,
                State,
                Output));
    }

    const double ExpectedUnsprungWeightN =
        Config.Unsprung.EffectiveMassKg
        * 9.80665;

    const double ObservedForceDifferenceN =
        Output.TireNormalForceN
        - Output.ChassisSuspensionReactionN;

    TestTrue(
        TEXT("Settled tire normal includes unsprung weight above chassis reaction"),
        ObservedForceDifferenceN > 100.0);

    TestTrue(
        TEXT("Settled tire-vs-chassis force difference approximates unsprung weight"),
        FMath::Abs(
            ObservedForceDifferenceN
            - ExpectedUnsprungWeightN)
            < 150.0);

    TestTrue(
        TEXT("Vehicle tire input receives full road normal load"),
        FMath::IsNearlyEqual(
            Output.VehicleContact.VerticalLoadN,
            Output.TireNormalForceN,
            1.0e-6));

    TestTrue(
        TEXT("Vehicle chassis input receives suspension reaction only"),
        FMath::IsNearlyEqual(
            Output.VehicleContact
                .SuspensionForceWorldN.Length(),
            Output.ChassisSuspensionReactionN,
            1.0e-6));

    TestTrue(
        TEXT("Chassis suspension force is not replaced by tire normal force"),
        FMath::Abs(
            Output.VehicleContact
                .SuspensionForceWorldN.Length()
            - Output.VehicleContact.VerticalLoadN)
            > 100.0);

    TestTrue(
        TEXT("Settled relative velocity remains bounded"),
        FMath::Abs(
            State.Unsprung.TravelVelocityMps)
            < 0.25);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAExperimentalUnsprungDeterminismTest,
    "TorqueAtlas.Suspension.UnsprungCorner.Deterministic",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)

bool FTAExperimentalUnsprungDeterminismTest::RunTest(
    const FString& Parameters)
{
    const FTAExperimentalUnsprungCornerConfig Config =
        MakeDynamicCornerConfig(false);

    FTAExperimentalUnsprungCornerInput Input =
        MakeDynamicCornerInput();

    FTAExperimentalUnsprungCornerState StateA;
    FTAExperimentalUnsprungCornerState StateB;

    FTAExperimentalUnsprungCornerOutput OutputA;
    FTAExperimentalUnsprungCornerOutput OutputB;

    TestTrue(
        TEXT("Determinism state A initializes"),
        TAExperimentalUnsprungCorner::InitializeFromQuasiStatic(
            Config,
            Input,
            1.0 / 240.0,
            StateA,
            OutputA));

    TestTrue(
        TEXT("Determinism state B initializes"),
        TAExperimentalUnsprungCorner::InitializeFromQuasiStatic(
            Config,
            Input,
            1.0 / 240.0,
            StateB,
            OutputB));

    Input.Road.PointWorldM.Z =
        0.012;

    for (int32 StepIndex = 0;
         StepIndex < 240;
         ++StepIndex)
    {
        TestTrue(
            TEXT("Determinism A step succeeds"),
            TAExperimentalUnsprungCorner::Step(
                Config,
                Input,
                1.0 / 240.0,
                StateA,
                OutputA));

        TestTrue(
            TEXT("Determinism B step succeeds"),
            TAExperimentalUnsprungCorner::Step(
                Config,
                Input,
                1.0 / 240.0,
                StateB,
                OutputB));
    }

    TestTrue(
        TEXT("Dynamic travel is deterministic"),
        FMath::IsNearlyEqual(
            StateA.Unsprung.TravelM,
            StateB.Unsprung.TravelM,
            1.0e-12));

    TestTrue(
        TEXT("Dynamic travel velocity is deterministic"),
        FMath::IsNearlyEqual(
            StateA.Unsprung.TravelVelocityMps,
            StateB.Unsprung.TravelVelocityMps,
            1.0e-12));

    TestTrue(
        TEXT("Tire normal force is deterministic"),
        FMath::IsNearlyEqual(
            OutputA.TireNormalForceN,
            OutputB.TireNormalForceN,
            1.0e-9));

    return true;
}

#endif
