#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TAVehicleSimulation.h"
#include "TAWheelContactResolver.h"

namespace
{
    FTAVehicleRuntimeConfig MakePrototypeRuntimeConfig()
    {
        FTAVehicleRuntimeConfig Config;

        Config.Chassis.GravityWorldMps2 = FVector3d::ZeroVector;
        Config.Chassis.MassKg = Config.ReferenceMassKg;

        Config.Wheels.SetNum(4);
        Config.Tires.SetNum(4);

        for (int32 Index = 0; Index < 4; ++Index)
        {
            Config.Wheels[Index].RadiusM = Config.Tires[Index].UnloadedRadiusM;
            Config.Wheels[Index].InertiaKgm2 = 1.20;
            Config.Wheels[Index].MaxBrakeTorqueNm = 2200.0;
        }

        Config.Wheels[2].bDriven = true;
        Config.Wheels[3].bDriven = true;

        return Config;
    }

    FTADoubleWishboneSolverConfig MakeIntegratedFrontRightGeometry()
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

    FTASuspensionRuntimeConfig MakeIntegratedFrontSuspension()
    {
        FTASuspensionRuntimeConfig Config;
        Config.SpringRateNPerM = 95000.0;
        Config.StaticSpringCompressionM = 0.08;
        Config.BumpDampingNsPerM = 4500.0;
        Config.ReboundDampingNsPerM = 6500.0;
        return Config;
    }

    FTAVehicleStepInput MakeStaticContactInput()
    {
        FTAVehicleStepInput Input;
        Input.WheelContacts.SetNum(4);

        for (FTAWheelContactInput& Contact : Input.WheelContacts)
        {
            Contact.VerticalLoadN = 3500.0;
            Contact.LongitudinalVelocityMps = 0.0;
            Contact.LateralVelocityMps = 0.0;
        }

        Input.WheelContacts[0].ContactPointWorldM = FVector3d( 1.31, -0.775, -0.45);
        Input.WheelContacts[1].ContactPointWorldM = FVector3d( 1.31,  0.775, -0.45);
        Input.WheelContacts[2].ContactPointWorldM = FVector3d(-1.31, -0.765, -0.45);
        Input.WheelContacts[3].ContactPointWorldM = FVector3d(-1.31,  0.765, -0.45);

        return Input;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleRuntimeDriveTest,
    "TorqueAtlas.Vehicle.Runtime.PowertrainToDrivenWheels",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleRuntimeDriveTest::RunTest(const FString& Parameters)
{
    const FTAVehicleRuntimeConfig Config = MakePrototypeRuntimeConfig();

    FTAVehicleRuntimeState State;
    TestTrue(
        TEXT("Prototype runtime initializes"),
        TAVehicleSimulation::Initialize(Config, State));

    FTAVehicleStepInput Input = MakeStaticContactInput();
    Input.Controls.Throttle01 = 1.0;
    Input.Controls.ClutchEngagement01 = 1.0;
    Input.Controls.SelectedGear = 1;

    FTAVehicleStepOutput Output;

    TestTrue(
        TEXT("Integrated vehicle step succeeds"),
        TAVehicleSimulation::Step(
            Config,
            Input,
            1.0 / 240.0,
            State,
            Output));

    TestTrue(
        TEXT("First gear transmits positive torque to left driven wheel"),
        Output.LeftDrivenWheelTorqueNm > 0.0);

    TestTrue(
        TEXT("First gear transmits positive torque to right driven wheel"),
        Output.RightDrivenWheelTorqueNm > 0.0);

    TestTrue(
        TEXT("Left driven wheel accelerates from rest"),
        State.Wheels[2].AngularSpeedRadPerSec > 0.0);

    TestTrue(
        TEXT("Right driven wheel accelerates from rest"),
        State.Wheels[3].AngularSpeedRadPerSec > 0.0);

    TestEqual(
        TEXT("Simulation tick increments"),
        State.SimulationTick,
        static_cast<uint64>(1));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleRuntimeNeutralTest,
    "TorqueAtlas.Vehicle.Runtime.NeutralDecouplesWheels",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleRuntimeNeutralTest::RunTest(const FString& Parameters)
{
    const FTAVehicleRuntimeConfig Config = MakePrototypeRuntimeConfig();

    FTAVehicleRuntimeState State;
    TestTrue(
        TEXT("Runtime initializes"),
        TAVehicleSimulation::Initialize(Config, State));

    FTAVehicleStepInput Input = MakeStaticContactInput();
    Input.Controls.Throttle01 = 1.0;
    Input.Controls.ClutchEngagement01 = 1.0;
    Input.Controls.SelectedGear = 0;

    FTAVehicleStepOutput Output;

    TestTrue(
        TEXT("Neutral step succeeds"),
        TAVehicleSimulation::Step(
            Config,
            Input,
            1.0 / 240.0,
            State,
            Output));

    TestTrue(
        TEXT("Neutral provides no left drive torque"),
        FMath::IsNearlyZero(Output.LeftDrivenWheelTorqueNm));

    TestTrue(
        TEXT("Neutral provides no right drive torque"),
        FMath::IsNearlyZero(Output.RightDrivenWheelTorqueNm));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleRuntimeBrakeTest,
    "TorqueAtlas.Vehicle.Runtime.BrakeDoesNotReverseWheel",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleRuntimeBrakeTest::RunTest(const FString& Parameters)
{
    FTAWheelRuntimeConfig Wheel;
    Wheel.InertiaKgm2 = 1.0;
    Wheel.MaxBrakeTorqueNm = 3000.0;

    const double Result =
        TAVehicleSimulation::IntegrateWheelAngularSpeed(
            Wheel,
            0.2,
            0.0,
            0.0,
            1.0,
            1.0 / 60.0);

    TestTrue(
        TEXT("Strong brake clamps wheel to zero instead of reversing it"),
        FMath::IsNearlyZero(Result));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleRuntimeSplitMuTest,
    "TorqueAtlas.Vehicle.Runtime.OpenDifferentialSplitGrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleRuntimeSplitMuTest::RunTest(const FString& Parameters)
{
    const FTAVehicleRuntimeConfig Config = MakePrototypeRuntimeConfig();

    FTAVehicleRuntimeState State;
    TestTrue(
        TEXT("Runtime initializes"),
        TAVehicleSimulation::Initialize(Config, State));

    FTAVehicleStepInput Input = MakeStaticContactInput();
    Input.Controls.Throttle01 = 1.0;
    Input.Controls.ClutchEngagement01 = 1.0;
    Input.Controls.SelectedGear = 1;

    // Rear-left driven wheel on near-ice, rear-right on dry asphalt.
    Input.WheelContacts[2].Surface.IceFraction01 = 1.0;
    Input.WheelContacts[3].Surface.IceFraction01 = 0.0;

    FTAVehicleStepOutput Output;

    TestTrue(
        TEXT("Split-mu step succeeds"),
        TAVehicleSimulation::Step(
            Config,
            Input,
            1.0 / 240.0,
            State,
            Output));

    TestTrue(
        TEXT("Open differential side torques remain equal"),
        FMath::IsNearlyEqual(
            Output.LeftDrivenWheelTorqueNm,
            Output.RightDrivenWheelTorqueNm,
            1.0e-6));

    const double DryOnlyCapacityNm =
        Config.Tires[3].DryPeakMu
        * Input.WheelContacts[3].VerticalLoadN
        * Config.Wheels[3].RadiusM;

    TestTrue(
        TEXT("Low-grip side limits torque below dry-only capacity"),
        FMath::Abs(Output.RightDrivenWheelTorqueNm) < DryOnlyCapacityNm);

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleRuntimeCoolingDamageTest,
    "TorqueAtlas.Vehicle.Runtime.RadiatorDamageCausesThermalDerate",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleRuntimeCoolingDamageTest::RunTest(const FString& Parameters)
{
    FTAVehicleRuntimeConfig Config = MakePrototypeRuntimeConfig();

    // Accelerated calibration for a short deterministic regression test.
    Config.Radiator.LeakMassFlowKgPerSecPerMm2 = 0.05;
    Config.EngineThermal.EffectiveThermalMassJPerC = 5000.0;
    Config.EngineThermal.CoolingCapacityWPerC = 200.0;

    FTAVehicleRuntimeState State;
    TestTrue(
        TEXT("Runtime initializes"),
        TAVehicleSimulation::Initialize(Config, State));

    TADamage::ApplyRadiatorImpact(
        Config.Radiator,
        Config.Radiator.FullLeakEnergyJ,
        1.0,
        State.Radiator);

    FTAVehicleStepInput Input = MakeStaticContactInput();
    Input.Controls.Throttle01 = 1.0;
    Input.Controls.ClutchEngagement01 = 0.0;
    Input.Controls.SelectedGear = 0;

    const double InitialCoolantMass = State.Radiator.CoolantMassKg;
    const double InitialTemperature = State.EngineThermal.CoolantTemperatureC;

    FTAVehicleStepOutput Output;

    for (int32 StepIndex = 0; StepIndex < 600; ++StepIndex)
    {
        TestTrue(
            TEXT("Thermal integration step succeeds"),
            TAVehicleSimulation::Step(
                Config,
                Input,
                1.0 / 60.0,
                State,
                Output));
    }

    TestTrue(
        TEXT("Damaged radiator loses coolant"),
        State.Radiator.CoolantMassKg < InitialCoolantMass);

    TestTrue(
        TEXT("Cooling efficiency falls after leak/crush"),
        State.Radiator.CoolingEfficiency01 < 1.0);

    TestTrue(
        TEXT("Coolant temperature rises under degraded cooling"),
        State.EngineThermal.CoolantTemperatureC > InitialTemperature);

    TestTrue(
        TEXT("High coolant temperature reduces engine thermal torque factor"),
        State.Engine.ThermalTorqueFactor < 1.0);

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleRuntimeTireForceMovesChassisTest,
    "TorqueAtlas.Vehicle.Runtime.TireForceMovesChassis",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleRuntimeTireForceMovesChassisTest::RunTest(const FString& Parameters)
{
    const FTAVehicleRuntimeConfig Config = MakePrototypeRuntimeConfig();

    FTAVehicleRuntimeState State;
    TestTrue(
        TEXT("Runtime initializes"),
        TAVehicleSimulation::Initialize(Config, State));

    FTAVehicleStepInput Input = MakeStaticContactInput();
    Input.Controls.SelectedGear = 0;
    Input.Controls.ClutchEngagement01 = 0.0;

    const double GroundSpeedMps = 15.0;

    for (int32 Index = 0; Index < 4; ++Index)
    {
        Input.WheelContacts[Index].LongitudinalVelocityMps = GroundSpeedMps;
        State.Wheels[Index].AngularSpeedRadPerSec =
            20.0 / Config.Wheels[Index].RadiusM;
    }

    FTAVehicleStepOutput Output;

    TestTrue(
        TEXT("Vehicle step succeeds"),
        TAVehicleSimulation::Step(
            Config,
            Input,
            1.0 / 240.0,
            State,
            Output));

    TestTrue(
        TEXT("Positive tire slip produces forward chassis acceleration"),
        State.Chassis.LinearVelocityWorldMps.X > 0.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleRuntimeAsymmetricGripYawTest,
    "TorqueAtlas.Vehicle.Runtime.AsymmetricGripCreatesYaw",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleRuntimeAsymmetricGripYawTest::RunTest(const FString& Parameters)
{
    const FTAVehicleRuntimeConfig Config = MakePrototypeRuntimeConfig();

    FTAVehicleRuntimeState State;
    TestTrue(
        TEXT("Runtime initializes"),
        TAVehicleSimulation::Initialize(Config, State));

    FTAVehicleStepInput Input = MakeStaticContactInput();
    Input.Controls.SelectedGear = 0;
    Input.Controls.ClutchEngagement01 = 0.0;

    for (int32 Index = 0; Index < 4; ++Index)
    {
        Input.WheelContacts[Index].LongitudinalVelocityMps = 15.0;
        State.Wheels[Index].AngularSpeedRadPerSec =
            15.0 / Config.Wheels[Index].RadiusM;
    }

    // Add substantial positive slip only at rear-left.
    State.Wheels[2].AngularSpeedRadPerSec =
        24.0 / Config.Wheels[2].RadiusM;

    FTAVehicleStepOutput Output;

    TestTrue(
        TEXT("Vehicle step succeeds"),
        TAVehicleSimulation::Step(
            Config,
            Input,
            1.0 / 240.0,
            State,
            Output));

    TestTrue(
        TEXT("Asymmetric longitudinal tire force creates yaw response"),
        FMath::Abs(State.Chassis.AngularVelocityWorldRadPerSec.Z) > 0.0);

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleResolvedSuspensionContactTest,
    "TorqueAtlas.Vehicle.Runtime.ResolvedSuspensionContactFeedsTireAndChassis",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleResolvedSuspensionContactTest::RunTest(const FString& Parameters)
{
    const FTAVehicleRuntimeConfig Config =
        MakePrototypeRuntimeConfig();

    FTAVehicleRuntimeState State;
    TestTrue(
        TEXT("Runtime initializes"),
        TAVehicleSimulation::Initialize(
            Config,
            State));

    State.Chassis.PositionWorldM =
        FVector3d(0.0, 0.0, 0.777);

    State.Chassis.LinearVelocityWorldMps =
        FVector3d(10.0, 0.0, 0.0);

    FTARoadPlane Road;
    Road.PointWorldM = FVector3d::ZeroVector;
    Road.NormalWorld = FVector3d(0.0, 0.0, 1.0);
    Road.Surface.Material =
        ETASurfaceMaterial::FreshAsphalt;

    FTADoubleWishboneState GeometryState;
    FTASuspensionRuntimeState SuspensionState;
    FTAResolvedWheelContact Resolved;

    TestTrue(
        TEXT("Front-right suspension/road contact resolves"),
        TAWheelContactResolver::ResolveDoubleWishboneRoadContact(
            State.Chassis,
            MakeIntegratedFrontRightGeometry(),
            MakeIntegratedFrontSuspension(),
            Config.Wheels[1].RadiusM,
            0.0,
            FTADoubleWishboneDamageOffsets{},
            Road,
            1.0 / 240.0,
            GeometryState,
            SuspensionState,
            Resolved));

    TestTrue(
        TEXT("Resolved contact provides its own positive vertical load"),
        Resolved.VerticalLoadN > 2500.0);

    TestTrue(
        TEXT("Resolved contact derives forward speed from chassis"),
        FMath::IsNearlyEqual(
            Resolved.LongitudinalVelocityMps,
            10.0,
            0.05));

    FTAVehicleStepInput Input;
    Input.WheelContacts.SetNum(4);
    Input.Controls.SelectedGear = 0;
    Input.Controls.ClutchEngagement01 = 0.0;

    Input.WheelContacts[1] =
        TAWheelContactResolver::BuildVehicleWheelContactInput(
            Resolved);

    State.Wheels[1].AngularSpeedRadPerSec =
        14.0 / Config.Wheels[1].RadiusM;

    const double BeforeVelocityX =
        State.Chassis.LinearVelocityWorldMps.X;

    FTAVehicleStepOutput Output;

    TestTrue(
        TEXT("Vehicle step accepts resolved contact"),
        TAVehicleSimulation::Step(
            Config,
            Input,
            1.0 / 240.0,
            State,
            Output));

    TestTrue(
        TEXT("Resolved tire slip accelerates chassis"),
        State.Chassis.LinearVelocityWorldMps.X
            > BeforeVelocityX);

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleRuntimeHubDriveEfficiencyTest,
    "TorqueAtlas.Vehicle.Runtime.HubDamageReducesDriveTorque",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleRuntimeHubDriveEfficiencyTest::RunTest(
    const FString& Parameters)
{
    const FTAVehicleRuntimeConfig Config =
        MakePrototypeRuntimeConfig();

    FTAVehicleRuntimeState HealthyState;
    FTAVehicleRuntimeState DamagedState;

    TestTrue(
        TEXT("Healthy runtime initializes"),
        TAVehicleSimulation::Initialize(
            Config,
            HealthyState));

    TestTrue(
        TEXT("Damaged runtime initializes"),
        TAVehicleSimulation::Initialize(
            Config,
            DamagedState));

    DamagedState.WheelHubDamage[2].DriveEfficiency01 =
        0.50;

    FTAVehicleStepInput Input =
        MakeStaticContactInput();

    Input.Controls.Throttle01 =
        1.0;

    Input.Controls.ClutchEngagement01 =
        1.0;

    Input.Controls.SelectedGear =
        1;

    FTAVehicleStepOutput HealthyOutput;
    FTAVehicleStepOutput DamagedOutput;

    TestTrue(
        TEXT("Healthy drive step succeeds"),
        TAVehicleSimulation::Step(
            Config,
            Input,
            1.0 / 240.0,
            HealthyState,
            HealthyOutput));

    TestTrue(
        TEXT("Damaged drive step succeeds"),
        TAVehicleSimulation::Step(
            Config,
            Input,
            1.0 / 240.0,
            DamagedState,
            DamagedOutput));

    TestTrue(
        TEXT("Healthy left driven wheel receives positive torque"),
        HealthyOutput.LeftDrivenWheelTorqueNm > 0.0);

    TestTrue(
        TEXT("Damaged hub transmits half commanded left drive torque"),
        FMath::IsNearlyEqual(
            DamagedOutput.LeftDrivenWheelTorqueNm,
            0.5 * HealthyOutput.LeftDrivenWheelTorqueNm,
            1.0e-6));

    TestTrue(
        TEXT("Undamaged right hub retains its drive torque"),
        FMath::IsNearlyEqual(
            DamagedOutput.RightDrivenWheelTorqueNm,
            HealthyOutput.RightDrivenWheelTorqueNm,
            1.0e-6));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleRuntimeHubBrakeEfficiencyTest,
    "TorqueAtlas.Vehicle.Runtime.HubDamageReducesBrakeTorque",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleRuntimeHubBrakeEfficiencyTest::RunTest(
    const FString& Parameters)
{
    const FTAVehicleRuntimeConfig Config =
        MakePrototypeRuntimeConfig();

    FTAVehicleRuntimeState HealthyState;
    FTAVehicleRuntimeState DamagedState;

    TestTrue(
        TEXT("Healthy runtime initializes"),
        TAVehicleSimulation::Initialize(
            Config,
            HealthyState));

    TestTrue(
        TEXT("Damaged runtime initializes"),
        TAVehicleSimulation::Initialize(
            Config,
            DamagedState));

    DamagedState.WheelHubDamage[0].BrakeEfficiency01 =
        0.25;

    HealthyState.Wheels[0].AngularSpeedRadPerSec =
        30.0;

    DamagedState.Wheels[0].AngularSpeedRadPerSec =
        30.0;

    FTAVehicleStepInput Input =
        MakeStaticContactInput();

    for (FTAWheelContactInput& Contact :
         Input.WheelContacts)
    {
        Contact.VerticalLoadN =
            0.0;
    }

    Input.Controls.Brake01 =
        1.0;

    Input.Controls.SelectedGear =
        0;

    Input.Controls.ClutchEngagement01 =
        0.0;

    FTAVehicleStepOutput HealthyOutput;
    FTAVehicleStepOutput DamagedOutput;

    TestTrue(
        TEXT("Healthy braking step succeeds"),
        TAVehicleSimulation::Step(
            Config,
            Input,
            1.0 / 240.0,
            HealthyState,
            HealthyOutput));

    TestTrue(
        TEXT("Damaged braking step succeeds"),
        TAVehicleSimulation::Step(
            Config,
            Input,
            1.0 / 240.0,
            DamagedState,
            DamagedOutput));

    TestTrue(
        TEXT("Healthy hub slows wheel more strongly"),
        HealthyState.Wheels[0].AngularSpeedRadPerSec
            < DamagedState.Wheels[0].AngularSpeedRadPerSec);

    TestTrue(
        TEXT("Damaged brake still opposes rotation"),
        DamagedState.Wheels[0].AngularSpeedRadPerSec
            < 30.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleRuntimeHubBearingDragTest,
    "TorqueAtlas.Vehicle.Runtime.HubDamageAddsBearingDrag",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleRuntimeHubBearingDragTest::RunTest(
    const FString& Parameters)
{
    const FTAVehicleRuntimeConfig Config =
        MakePrototypeRuntimeConfig();

    FTAVehicleRuntimeState HealthyState;
    FTAVehicleRuntimeState DamagedState;

    TestTrue(
        TEXT("Healthy runtime initializes"),
        TAVehicleSimulation::Initialize(
            Config,
            HealthyState));

    TestTrue(
        TEXT("Damaged runtime initializes"),
        TAVehicleSimulation::Initialize(
            Config,
            DamagedState));

    HealthyState.Wheels[0].AngularSpeedRadPerSec =
        20.0;

    DamagedState.Wheels[0].AngularSpeedRadPerSec =
        20.0;

    DamagedState.WheelHubDamage[0].BearingDragTorqueNm =
        120.0;

    FTAVehicleStepInput Input =
        MakeStaticContactInput();

    for (FTAWheelContactInput& Contact :
         Input.WheelContacts)
    {
        Contact.VerticalLoadN =
            0.0;
    }

    Input.Controls.SelectedGear =
        0;

    Input.Controls.ClutchEngagement01 =
        0.0;

    FTAVehicleStepOutput HealthyOutput;
    FTAVehicleStepOutput DamagedOutput;

    TestTrue(
        TEXT("Healthy coast step succeeds"),
        TAVehicleSimulation::Step(
            Config,
            Input,
            1.0 / 240.0,
            HealthyState,
            HealthyOutput));

    TestTrue(
        TEXT("Damaged coast step succeeds"),
        TAVehicleSimulation::Step(
            Config,
            Input,
            1.0 / 240.0,
            DamagedState,
            DamagedOutput));

    TestTrue(
        TEXT("Bearing drag reduces damaged wheel speed"),
        DamagedState.Wheels[0].AngularSpeedRadPerSec
            < HealthyState.Wheels[0].AngularSpeedRadPerSec);

    return true;
}

#endif
