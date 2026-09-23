#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TAFourWheelVehicleRuntime.h"

namespace
{
    FTADoubleWishboneSolverConfig MakeFrontRightGeometry()
    {
        FTADoubleWishboneSolverConfig Config;
        FTADoubleWishboneHardpoints& H = Config.Hardpoints;

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

        Config.MaxIterations = 80;
        Config.PositionToleranceM = 0.0005;

        return Config;
    }

    FTAMultiLinkSolverConfig MakeRearRightGeometry()
    {
        FTAMultiLinkSolverConfig Config;

        const FVector3d Chassis[TARearMultiLinkCount] =
        {
            FVector3d(-1.15, 0.35, -0.18),
            FVector3d(-1.50, 0.36, -0.20),
            FVector3d(-1.10, 0.34, -0.46),
            FVector3d(-1.53, 0.33, -0.47),
            FVector3d(-1.50, 0.38, -0.36)
        };

        const FVector3d Upright[TARearMultiLinkCount] =
        {
            FVector3d(-1.20, 0.68, -0.24),
            FVector3d(-1.42, 0.69, -0.25),
            FVector3d(-1.18, 0.72, -0.49),
            FVector3d(-1.45, 0.73, -0.50),
            FVector3d(-1.48, 0.71, -0.38)
        };

        for (int32 Index = 0;
             Index < TARearMultiLinkCount;
             ++Index)
        {
            Config.Links[Index].ChassisPickupReference =
                Chassis[Index];

            Config.Links[Index].UprightPickupReference =
                Upright[Index];
        }

        Config.WheelCenterReference =
            FVector3d(-1.31, 0.765, -0.45);

        Config.DamperChassisReference =
            FVector3d(-1.31, 0.40, -0.05);

        Config.DamperUprightReference =
            FVector3d(-1.31, 0.64, -0.40);

        Config.WheelForwardReference =
            FVector3d(1.0, 0.0, 0.0);

        Config.WheelUpReference =
            FVector3d(0.0, 0.0, 1.0);

        Config.MaxIterations = 180;
        Config.PositionToleranceM = 0.001;

        return Config;
    }

    FTAFourWheelRuntimeConfig MakeFourWheelRuntimeConfig()
    {
        FTAFourWheelRuntimeConfig Config;

        Config.FrontAxle.RightGeometry =
            MakeFrontRightGeometry();

        Config.FrontAxle.LeftSuspension.SpringRateNPerM = 100000.0;
        Config.FrontAxle.LeftSuspension.StaticSpringCompressionM = 0.0810;
        Config.FrontAxle.LeftSuspension.BumpDampingNsPerM = 4500.0;
        Config.FrontAxle.LeftSuspension.ReboundDampingNsPerM = 6500.0;
        Config.FrontAxle.RightSuspension =
            Config.FrontAxle.LeftSuspension;

        Config.FrontAxle.AntiRollBar.CouplingRateNPerM = 12000.0;
        Config.FrontAxle.AntiRollBar.MaxTransferForceN = 3000.0;

        Config.FrontAxle.SteeringRack.MaxRackDisplacementM = 0.035;
        Config.FrontAxle.SteeringRack.InputExponent = 1.0;
        Config.FrontAxle.SteeringRack.SteeringSign = -1.0;

        Config.RearAxle.RightGeometry =
            MakeRearRightGeometry();

        Config.RearAxle.LeftSuspension.SpringRateNPerM = 50000.0;
        Config.RearAxle.LeftSuspension.StaticSpringCompressionM = 0.0987;
        Config.RearAxle.LeftSuspension.BumpDampingNsPerM = 3800.0;
        Config.RearAxle.LeftSuspension.ReboundDampingNsPerM = 5200.0;
        Config.RearAxle.RightSuspension =
            Config.RearAxle.LeftSuspension;

        Config.RearAxle.AntiRollBar.CouplingRateNPerM = 9000.0;
        Config.RearAxle.AntiRollBar.MaxTransferForceN = 2500.0;

        return Config;
    }

    FTAVehicleRuntimeConfig MakeVehicleConfig()
    {
        FTAVehicleRuntimeConfig Config;
        Config.ReferenceMassKg = 1420.0;
        Config.Chassis.MassKg = Config.ReferenceMassKg;

        Config.Wheels.SetNum(4);
        Config.Tires.SetNum(4);

        for (int32 Index = 0;
             Index < 4;
             ++Index)
        {
            Config.Wheels[Index].RadiusM =
                Config.Tires[Index].UnloadedRadiusM;

            Config.Wheels[Index].InertiaKgm2 = 1.20;
            Config.Wheels[Index].MaxBrakeTorqueNm = 2200.0;
        }

        Config.Wheels[
            static_cast<int32>(ETAPrototypeWheelIndex::RearLeft)]
            .bDriven = true;

        Config.Wheels[
            static_cast<int32>(ETAPrototypeWheelIndex::RearRight)]
            .bDriven = true;

        return Config;
    }

    FTARoadPlane MakeFlatRoad()
    {
        FTARoadPlane Road;
        Road.NormalWorld = FVector3d(0.0, 0.0, 1.0);
        Road.Surface.Material = ETASurfaceMaterial::FreshAsphalt;
        Road.Surface.TemperatureC = 20.0;
        return Road;
    }

    FTAFourWheelStepInput MakeFourWheelInput()
    {
        FTAFourWheelStepInput Input;

        Input.FrontLeftRoad = MakeFlatRoad();
        Input.FrontRightRoad = MakeFlatRoad();
        Input.RearLeftRoad = MakeFlatRoad();
        Input.RearRightRoad = MakeFlatRoad();

        return Input;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAFourWheelStaticSupportTest,
    "TorqueAtlas.Vehicle.FourWheel.StaticSelfSupport",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAFourWheelStaticSupportTest::RunTest(const FString& Parameters)
{
    const FTAVehicleRuntimeConfig VehicleConfig =
        MakeVehicleConfig();

    const FTAFourWheelRuntimeConfig RuntimeConfig =
        MakeFourWheelRuntimeConfig();

    FTAFourWheelRuntimeState State;

    TestTrue(
        TEXT("Four-wheel runtime initializes"),
        TAFourWheelVehicleRuntime::Initialize(
            VehicleConfig,
            State));

    State.Vehicle.Chassis.PositionWorldM =
        FVector3d(0.0, 0.0, 0.777);

    const FTAFourWheelStepInput Input =
        MakeFourWheelInput();

    FTAFourWheelStepOutput Output;

    TestTrue(
        TEXT("Self-support step succeeds"),
        TAFourWheelVehicleRuntime::Step(
            VehicleConfig,
            RuntimeConfig,
            Input,
            1.0 / 240.0,
            State,
            Output));

    TestTrue(
        TEXT("All four contacts solve"),
        Output.FrontAxle.LeftContact.bInContact
        && Output.FrontAxle.RightContact.bInContact
        && Output.RearAxle.LeftContact.bInContact
        && Output.RearAxle.RightContact.bInContact);

    const double TotalSupportN =
        Output.FrontAxle.LeftContact.VerticalLoadN
        + Output.FrontAxle.RightContact.VerticalLoadN
        + Output.RearAxle.LeftContact.VerticalLoadN
        + Output.RearAxle.RightContact.VerticalLoadN;

    const double WeightN =
        VehicleConfig.Chassis.MassKg
        * FMath::Abs(
            VehicleConfig.Chassis.GravityWorldMps2.Z);

    TestTrue(
        TEXT("Static suspension support is close to vehicle weight"),
        FMath::Abs(TotalSupportN - WeightN) < 750.0);

    TestTrue(
        TEXT("Front tires carry finite radial deflection"),
        Output.FrontAxle.LeftContact.TireRadialDeflectionM > 0.005
        && Output.FrontAxle.RightContact.TireRadialDeflectionM > 0.005);

    TestTrue(
        TEXT("Rear tires carry finite radial deflection"),
        Output.RearAxle.LeftContact.TireRadialDeflectionM > 0.005
        && Output.RearAxle.RightContact.TireRadialDeflectionM > 0.005);

    TestTrue(
        TEXT("Reference-height first step has small vertical velocity"),
        FMath::Abs(
            State.Vehicle.Chassis.LinearVelocityWorldMps.Z)
            < 0.01);

    TestTrue(
        TEXT("Symmetric COM/axle fixture has small pitch response at rest"),
        FMath::Abs(
            State.Vehicle.Chassis.AngularVelocityWorldRadPerSec.Y)
            < 0.01);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAFourWheelAccelerationTest,
    "TorqueAtlas.Vehicle.FourWheel.AcceleratesFromResolvedContacts",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAFourWheelAccelerationTest::RunTest(const FString& Parameters)
{
    const FTAVehicleRuntimeConfig VehicleConfig =
        MakeVehicleConfig();

    const FTAFourWheelRuntimeConfig RuntimeConfig =
        MakeFourWheelRuntimeConfig();

    FTAFourWheelRuntimeState State;

    TestTrue(
        TEXT("Runtime initializes"),
        TAFourWheelVehicleRuntime::Initialize(
            VehicleConfig,
            State));

    State.Vehicle.Chassis.PositionWorldM =
        FVector3d(0.0, 0.0, 0.777);

    FTAFourWheelStepInput Input =
        MakeFourWheelInput();

    Input.Controls.Throttle01 = 1.0;
    Input.Controls.ClutchEngagement01 = 0.25;
    Input.Controls.SelectedGear = 1;

    FTAFourWheelStepOutput Output;

    for (int32 StepIndex = 0;
         StepIndex < 240;
         ++StepIndex)
    {
        TestTrue(
            TEXT("Acceleration step succeeds"),
            TAFourWheelVehicleRuntime::Step(
                VehicleConfig,
                RuntimeConfig,
                Input,
                1.0 / 240.0,
                State,
                Output));
    }

    TestTrue(
        TEXT("Vehicle gains forward speed from its resolved rear contacts"),
        State.Vehicle.Chassis.LinearVelocityWorldMps.X > 0.1);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAFourWheelSteeringYawTest,
    "TorqueAtlas.Vehicle.FourWheel.SteeringCreatesYaw",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAFourWheelSteeringYawTest::RunTest(const FString& Parameters)
{
    const FTAVehicleRuntimeConfig VehicleConfig =
        MakeVehicleConfig();

    const FTAFourWheelRuntimeConfig RuntimeConfig =
        MakeFourWheelRuntimeConfig();

    FTAFourWheelRuntimeState State;

    TestTrue(
        TEXT("Runtime initializes"),
        TAFourWheelVehicleRuntime::Initialize(
            VehicleConfig,
            State));

    State.Vehicle.Chassis.PositionWorldM =
        FVector3d(0.0, 0.0, 0.777);

    State.Vehicle.Chassis.LinearVelocityWorldMps =
        FVector3d(15.0, 0.0, 0.0);

    for (int32 Index = 0;
         Index < State.Vehicle.Wheels.Num();
         ++Index)
    {
        State.Vehicle.Wheels[Index].AngularSpeedRadPerSec =
            15.0
            / VehicleConfig.Wheels[Index].RadiusM;
    }

    FTAFourWheelStepInput Input =
        MakeFourWheelInput();

    Input.Controls.Steering01 = 0.35;
    Input.Controls.SelectedGear = 0;
    Input.Controls.ClutchEngagement01 = 0.0;

    FTAFourWheelStepOutput Output;

    TestTrue(
        TEXT("Steering step succeeds"),
        TAFourWheelVehicleRuntime::Step(
            VehicleConfig,
            RuntimeConfig,
            Input,
            1.0 / 240.0,
            State,
            Output));

    TestTrue(
        TEXT("Shared rack creates positive right-turn wheel angles"),
        Output.FrontAxle.LeftSteeringAngleRad > 0.0
        && Output.FrontAxle.RightSteeringAngleRad > 0.0);

    TestTrue(
        TEXT("Steered front tires generate yaw response"),
        FMath::Abs(
            State.Vehicle.Chassis.AngularVelocityWorldRadPerSec.Z)
            > 0.0);

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAFourWheelStaticStabilityTest,
    "TorqueAtlas.Vehicle.FourWheel.StaticStability",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAFourWheelStaticStabilityTest::RunTest(const FString& Parameters)
{
    const FTAVehicleRuntimeConfig VehicleConfig =
        MakeVehicleConfig();

    const FTAFourWheelRuntimeConfig RuntimeConfig =
        MakeFourWheelRuntimeConfig();

    FTAFourWheelRuntimeState State;

    TestTrue(
        TEXT("Runtime initializes"),
        TAFourWheelVehicleRuntime::Initialize(
            VehicleConfig,
            State));

    State.Vehicle.Chassis.PositionWorldM =
        FVector3d(0.0, 0.0, 0.777);

    const FTAFourWheelStepInput Input =
        MakeFourWheelInput();

    FTAFourWheelStepOutput Output;

    for (int32 StepIndex = 0;
         StepIndex < 480;
         ++StepIndex)
    {
        TestTrue(
            TEXT("Static stability step succeeds"),
            TAFourWheelVehicleRuntime::Step(
                VehicleConfig,
                RuntimeConfig,
                Input,
                1.0 / 240.0,
                State,
                Output));
    }

    TestTrue(
        TEXT("Static chassis height remains bounded"),
        State.Vehicle.Chassis.PositionWorldM.Z > 0.65
        && State.Vehicle.Chassis.PositionWorldM.Z < 0.90);

    TestTrue(
        TEXT("Static vertical velocity remains bounded"),
        FMath::Abs(
            State.Vehicle.Chassis.LinearVelocityWorldMps.Z)
            < 1.0);

    TestTrue(
        TEXT("Static pitch rate remains bounded"),
        FMath::Abs(
            State.Vehicle.Chassis.AngularVelocityWorldRadPerSec.Y)
            < 1.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAFourWheelBrakingTest,
    "TorqueAtlas.Vehicle.FourWheel.BrakesFromResolvedContacts",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAFourWheelBrakingTest::RunTest(const FString& Parameters)
{
    const FTAVehicleRuntimeConfig VehicleConfig =
        MakeVehicleConfig();

    const FTAFourWheelRuntimeConfig RuntimeConfig =
        MakeFourWheelRuntimeConfig();

    FTAFourWheelRuntimeState State;

    TestTrue(
        TEXT("Runtime initializes"),
        TAFourWheelVehicleRuntime::Initialize(
            VehicleConfig,
            State));

    State.Vehicle.Chassis.PositionWorldM =
        FVector3d(0.0, 0.0, 0.777);

    State.Vehicle.Chassis.LinearVelocityWorldMps =
        FVector3d(15.0, 0.0, 0.0);

    for (int32 Index = 0;
         Index < State.Vehicle.Wheels.Num();
         ++Index)
    {
        State.Vehicle.Wheels[Index].AngularSpeedRadPerSec =
            15.0
            / VehicleConfig.Wheels[Index].RadiusM;
    }

    FTAFourWheelStepInput Input =
        MakeFourWheelInput();

    Input.Controls.Brake01 = 1.0;
    Input.Controls.SelectedGear = 0;
    Input.Controls.ClutchEngagement01 = 0.0;

    FTAFourWheelStepOutput Output;

    for (int32 StepIndex = 0;
         StepIndex < 120;
         ++StepIndex)
    {
        TestTrue(
            TEXT("Braking step succeeds"),
            TAFourWheelVehicleRuntime::Step(
                VehicleConfig,
                RuntimeConfig,
                Input,
                1.0 / 240.0,
                State,
                Output));
    }

    TestTrue(
        TEXT("Vehicle speed decreases under braking"),
        State.Vehicle.Chassis.LinearVelocityWorldMps.X
            < 15.0);

    for (int32 Index = 0;
         Index < State.Vehicle.Wheels.Num();
         ++Index)
    {
        TestTrue(
            TEXT("Brake integration does not reverse wheel"),
            State.Vehicle.Wheels[Index].AngularSpeedRadPerSec
                >= -1.0e-9);
    }

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAFourWheelSuspensionDamageLoadTest,
    "TorqueAtlas.Vehicle.FourWheel.SuspensionDamageReducesCornerSupport",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAFourWheelSuspensionDamageLoadTest::RunTest(
    const FString& Parameters)
{
    const FTAVehicleRuntimeConfig VehicleConfig =
        MakeVehicleConfig();

    const FTAFourWheelRuntimeConfig RuntimeConfig =
        MakeFourWheelRuntimeConfig();

    FTAFourWheelRuntimeState HealthyState;
    FTAFourWheelRuntimeState DamagedState;

    TestTrue(
        TEXT("Healthy fixture initializes"),
        TAFourWheelVehicleRuntime::Initialize(
            VehicleConfig,
            HealthyState));

    TestTrue(
        TEXT("Damaged fixture initializes"),
        TAFourWheelVehicleRuntime::Initialize(
            VehicleConfig,
            DamagedState));

    HealthyState.Vehicle.Chassis.PositionWorldM =
        FVector3d(0.0, 0.0, 0.777);

    DamagedState.Vehicle.Chassis.PositionWorldM =
        HealthyState.Vehicle.Chassis.PositionWorldM;

    const int32 FrontLeftIndex =
        static_cast<int32>(
            ETAPrototypeWheelIndex::FrontLeft);

    DamagedState.Vehicle.SuspensionDamage[
        FrontLeftIndex].SpringEfficiency01 =
        0.50;

    const FTAFourWheelStepInput Input =
        MakeFourWheelInput();

    FTAFourWheelStepOutput HealthyOutput;
    FTAFourWheelStepOutput DamagedOutput;

    TestTrue(
        TEXT("Healthy support step succeeds"),
        TAFourWheelVehicleRuntime::Step(
            VehicleConfig,
            RuntimeConfig,
            Input,
            1.0 / 240.0,
            HealthyState,
            HealthyOutput));

    TestTrue(
        TEXT("Damaged support step succeeds"),
        TAFourWheelVehicleRuntime::Step(
            VehicleConfig,
            RuntimeConfig,
            Input,
            1.0 / 240.0,
            DamagedState,
            DamagedOutput));

    TestTrue(
        TEXT("Reduced front-left spring efficiency reduces front-left normal load"),
        DamagedOutput.FrontAxle.LeftContact.VerticalLoadN
            < HealthyOutput.FrontAxle.LeftContact.VerticalLoadN);

    TestTrue(
        TEXT("Asymmetric suspension loss creates larger roll response than healthy fixture"),
        FMath::Abs(
            DamagedState.Vehicle.Chassis
                .AngularVelocityWorldRadPerSec.X)
            > FMath::Abs(
                HealthyState.Vehicle.Chassis
                    .AngularVelocityWorldRadPerSec.X));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAFourWheelAntiRollLinkFailureTest,
    "TorqueAtlas.Vehicle.FourWheel.AntiRollLinkFailureRemovesTransfer",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAFourWheelAntiRollLinkFailureTest::RunTest(
    const FString& Parameters)
{
    const FTAVehicleRuntimeConfig VehicleConfig =
        MakeVehicleConfig();

    FTAFourWheelRuntimeConfig RuntimeConfig =
        MakeFourWheelRuntimeConfig();

    RuntimeConfig.FrontAxle.AntiRollBar.CouplingRateNPerM =
        60000.0;

    RuntimeConfig.FrontAxle.AntiRollBar.MaxTransferForceN =
        5000.0;

    FTAFourWheelRuntimeState HealthyState;
    FTAFourWheelRuntimeState BrokenLinkState;

    TestTrue(
        TEXT("Healthy anti-roll fixture initializes"),
        TAFourWheelVehicleRuntime::Initialize(
            VehicleConfig,
            HealthyState));

    TestTrue(
        TEXT("Broken-link fixture initializes"),
        TAFourWheelVehicleRuntime::Initialize(
            VehicleConfig,
            BrokenLinkState));

    HealthyState.Vehicle.Chassis.PositionWorldM =
        FVector3d(0.0, 0.0, 0.777);

    BrokenLinkState.Vehicle.Chassis.PositionWorldM =
        HealthyState.Vehicle.Chassis.PositionWorldM;

    const int32 FrontLeftIndex =
        static_cast<int32>(
            ETAPrototypeWheelIndex::FrontLeft);

    BrokenLinkState.Vehicle.SuspensionDamage[
        FrontLeftIndex].AntiRollLinkEfficiency01 =
        0.0;

    FTAFourWheelStepInput Input =
        MakeFourWheelInput();

    Input.FrontLeftRoad.PointWorldM.Z =
        0.020;

    FTAFourWheelStepOutput HealthyOutput;
    FTAFourWheelStepOutput BrokenOutput;

    TestTrue(
        TEXT("Healthy asymmetric-road step succeeds"),
        TAFourWheelVehicleRuntime::Step(
            VehicleConfig,
            RuntimeConfig,
            Input,
            1.0 / 240.0,
            HealthyState,
            HealthyOutput));

    TestTrue(
        TEXT("Broken-link asymmetric-road step succeeds"),
        TAFourWheelVehicleRuntime::Step(
            VehicleConfig,
            RuntimeConfig,
            Input,
            1.0 / 240.0,
            BrokenLinkState,
            BrokenOutput));

    const double HealthyFrontLoadSplitN =
        FMath::Abs(
            HealthyOutput.FrontAxle.LeftContact.VerticalLoadN
            - HealthyOutput.FrontAxle.RightContact.VerticalLoadN);

    const double BrokenFrontLoadSplitN =
        FMath::Abs(
            BrokenOutput.FrontAxle.LeftContact.VerticalLoadN
            - BrokenOutput.FrontAxle.RightContact.VerticalLoadN);

    TestTrue(
        TEXT("Healthy anti-roll coupling creates more front load transfer than broken link"),
        HealthyFrontLoadSplitN
            > BrokenFrontLoadSplitN + 10.0);

    return true;
}

#endif
