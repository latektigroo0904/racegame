#include "Misc/AutomationTest.h"
#include "TAVehicleSimulation.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
    FTAVehicleRuntimeConfig MakeAeroStepConfig()
    {
        FTAVehicleRuntimeConfig Config;
        Config.Wheels.SetNum(2);
        Config.Tires.SetNum(2);
        Config.Wheels[0].bDriven = true;
        Config.Wheels[1].bDriven = true;
        Config.Chassis.MassKg = 1000.0;
        Config.Chassis.PrincipalInertiaBodyKgm2 = FVector3d(500.0, 900.0, 1000.0);
        Config.Aerodynamics.ReferenceAreaM2 = 2.4;
        Config.Aerodynamics.DragCoefficient = 0.41;
        Config.Aerodynamics.LiftCoefficient = -0.30;
        Config.Aerodynamics.ApplicationPointBodyM = FVector3d(0.75, 0.0, 0.35);
        return Config;
    }

    FTAVehicleStepInput MakeAeroStepInput()
    {
        FTAVehicleStepInput Input;
        Input.WheelContacts.SetNum(2);
        Input.AerodynamicsEnvironment.AirDensityKgPerM3 = 1.225;
        Input.AerodynamicsEnvironment.WindVelocityWorldMps = FVector3d::ZeroVector;
        return Input;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleFullStepAeroTest,
    "TorqueAtlas.Vehicle.Aerodynamics.FullStepChangesChassis",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleFullStepAeroTest::RunTest(const FString& Parameters)
{
    FTAVehicleRuntimeConfig Config = MakeAeroStepConfig();
    FTAVehicleRuntimeState State;
    TestTrue(TEXT("Initialize"), TAVehicleSimulation::Initialize(Config, State));
    State.Chassis.LinearVelocityWorldMps = FVector3d(30.0, 0.0, 0.0);

    FTAVehicleStepInput Input = MakeAeroStepInput();
    FTAVehicleStepOutput Output;
    const double InitialVx = State.Chassis.LinearVelocityWorldMps.X;

    TestTrue(TEXT("Step"), TAVehicleSimulation::Step(Config, Input, 0.01, State, Output));
    TestTrue(TEXT("Applied drag is reported"), Output.Aerodynamics.DragForceN > 0.0);
    TestTrue(TEXT("Applied downforce is reported"), Output.Aerodynamics.ForceWorldN.Z < 0.0);
    TestTrue(TEXT("Drag decelerates chassis"), State.Chassis.LinearVelocityWorldMps.X < InitialVx);
    TestTrue(TEXT("Downforce changes vertical velocity"), State.Chassis.LinearVelocityWorldMps.Z < 0.0);
    TestTrue(TEXT("Off-COM aero produces pitch response"), FMath::Abs(State.Chassis.AngularVelocityBodyRadPerSec.Y) > 1.0e-9);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleFullStepZeroAirTest,
    "TorqueAtlas.Vehicle.Aerodynamics.FullStepZeroAirIsNeutral",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleFullStepZeroAirTest::RunTest(const FString& Parameters)
{
    FTAVehicleRuntimeConfig Config = MakeAeroStepConfig();
    FTAVehicleRuntimeState State;
    TestTrue(TEXT("Initialize"), TAVehicleSimulation::Initialize(Config, State));
    State.Chassis.LinearVelocityWorldMps = FVector3d(30.0, 0.0, 0.0);

    FTAVehicleStepInput Input = MakeAeroStepInput();
    Input.AerodynamicsEnvironment.AirDensityKgPerM3 = 0.0;
    FTAVehicleStepOutput Output;
    TestTrue(TEXT("Step"), TAVehicleSimulation::Step(Config, Input, 0.01, State, Output));
    TestEqual(TEXT("Zero dynamic pressure"), Output.Aerodynamics.DynamicPressurePa, 0.0);
    TestTrue(TEXT("No aero force"), Output.Aerodynamics.ForceWorldN.IsNearlyZero());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleFullStepHeadwindTest,
    "TorqueAtlas.Vehicle.Aerodynamics.FullStepHeadwindIncreasesDrag",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleFullStepHeadwindTest::RunTest(const FString& Parameters)
{
    FTAVehicleRuntimeConfig Config = MakeAeroStepConfig();
    FTAVehicleStepInput CalmInput = MakeAeroStepInput();
    FTAVehicleStepInput HeadwindInput = CalmInput;
    HeadwindInput.AerodynamicsEnvironment.WindVelocityWorldMps = FVector3d(-10.0, 0.0, 0.0);

    FTAVehicleRuntimeState CalmState, HeadwindState;
    TestTrue(TEXT("Initialize calm"), TAVehicleSimulation::Initialize(Config, CalmState));
    TestTrue(TEXT("Initialize headwind"), TAVehicleSimulation::Initialize(Config, HeadwindState));
    CalmState.Chassis.LinearVelocityWorldMps = FVector3d(30.0, 0.0, 0.0);
    HeadwindState.Chassis.LinearVelocityWorldMps = CalmState.Chassis.LinearVelocityWorldMps;

    FTAVehicleStepOutput CalmOutput, HeadwindOutput;
    TestTrue(TEXT("Calm step"), TAVehicleSimulation::Step(Config, CalmInput, 0.01, CalmState, CalmOutput));
    TestTrue(TEXT("Headwind step"), TAVehicleSimulation::Step(Config, HeadwindInput, 0.01, HeadwindState, HeadwindOutput));
    TestTrue(TEXT("Headwind raises dynamic pressure"), HeadwindOutput.Aerodynamics.DynamicPressurePa > CalmOutput.Aerodynamics.DynamicPressurePa);
    TestTrue(TEXT("Headwind raises drag"), HeadwindOutput.Aerodynamics.DragForceN > CalmOutput.Aerodynamics.DragForceN);
    return true;
}

#endif
