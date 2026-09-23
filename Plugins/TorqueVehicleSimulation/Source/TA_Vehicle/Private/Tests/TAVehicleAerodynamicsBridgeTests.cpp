#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TAVehicleAerodynamicsBridge.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleAeroBridgeAddsForceTest,
    "TorqueAtlas.Vehicle.AerodynamicsBridge.AddsForce",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleAeroBridgeAddsForceTest::RunTest(const FString& Parameters)
{
    FTAVehicleRuntimeConfig Config;
    Config.Aerodynamics.ReferenceAreaM2 = 2.0;
    Config.Aerodynamics.DragCoefficient = 0.30;
    Config.Aerodynamics.LiftCoefficient = -0.10;

    FTAVehicleStepInput Input;
    Input.AerodynamicsEnvironment.AirDensityKgPerM3 = 1.225;
    Input.AerodynamicsEnvironment.WindVelocityWorldMps = FVector3d::ZeroVector;

    FTAChassisState Chassis;
    Chassis.OrientationWorld = FQuat4d::Identity;
    Chassis.LinearVelocityWorldMps = FVector3d(30.0, 0.0, 0.0);

    FTAChassisForceAccumulator Accumulator;
    FTAVehicleStepOutput Output;

    TestTrue(TEXT("Bridge accepts valid vehicle aero data"),
        TAVehicleAerodynamicsBridge::AddToChassis(
            Config, Input, Chassis, Accumulator, Output));

    TestTrue(TEXT("Drag opposes forward vehicle motion"),
        Output.Aerodynamics.ForceWorldN.X < 0.0);
    TestTrue(TEXT("Negative Cl produces downforce"),
        Output.Aerodynamics.ForceWorldN.Z < 0.0);
    TestTrue(TEXT("Aero force reaches shared chassis accumulator"),
        Accumulator.ForceWorldN.Equals(Output.Aerodynamics.ForceWorldN, 1.0e-9));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleAeroBridgeUsesEnvironmentTest,
    "TorqueAtlas.Vehicle.AerodynamicsBridge.UsesStepEnvironment",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleAeroBridgeUsesEnvironmentTest::RunTest(const FString& Parameters)
{
    FTAVehicleRuntimeConfig Config;
    Config.Aerodynamics.ReferenceAreaM2 = 2.0;
    Config.Aerodynamics.DragCoefficient = 0.30;
    Config.Aerodynamics.LiftCoefficient = 0.0;

    FTAChassisState Chassis;
    Chassis.OrientationWorld = FQuat4d::Identity;
    Chassis.LinearVelocityWorldMps = FVector3d(20.0, 0.0, 0.0);

    FTAVehicleStepInput CalmInput;
    CalmInput.AerodynamicsEnvironment.AirDensityKgPerM3 = 1.225;

    FTAVehicleStepInput HeadwindInput = CalmInput;
    HeadwindInput.AerodynamicsEnvironment.WindVelocityWorldMps = FVector3d(-10.0, 0.0, 0.0);

    FTAChassisForceAccumulator CalmAccumulator;
    FTAChassisForceAccumulator HeadwindAccumulator;
    FTAVehicleStepOutput CalmOutput;
    FTAVehicleStepOutput HeadwindOutput;

    TestTrue(TEXT("Calm-air bridge solve succeeds"),
        TAVehicleAerodynamicsBridge::AddToChassis(
            Config, CalmInput, Chassis, CalmAccumulator, CalmOutput));
    TestTrue(TEXT("Headwind bridge solve succeeds"),
        TAVehicleAerodynamicsBridge::AddToChassis(
            Config, HeadwindInput, Chassis, HeadwindAccumulator, HeadwindOutput));

    TestTrue(TEXT("Headwind increases drag magnitude"),
        HeadwindOutput.Aerodynamics.DragForceN > CalmOutput.Aerodynamics.DragForceN);

    return true;
}

#endif
