#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TAVehicleSimulation.h"

namespace
{
    FTAVehicleRuntimeConfig MakePrototypeRuntimeConfig()
    {
        FTAVehicleRuntimeConfig Config;

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

#endif
