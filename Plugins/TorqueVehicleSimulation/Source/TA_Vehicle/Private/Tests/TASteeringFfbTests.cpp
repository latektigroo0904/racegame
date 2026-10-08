#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TASteeringFfb.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTASteeringFfbRackForceSignTest,
    "TorqueAtlas.Vehicle.Steering.FFB.RackForceProducesOpposingTorque",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTASteeringFfbRackForceSignTest::RunTest(
    const FString& Parameters)
{
    FTASteeringFfbConfig Config;
    Config.ColumnViscousDampingNmsPerRad = 0.0;
    Config.ColumnCoulombFrictionNm = 0.0;

    FTASteeringFfbInput Input;
    Input.RackForceN = 1000.0;

    FTASteeringFfbOutput Output;

    TestTrue(
        TEXT("FFB rack-force solve succeeds"),
        TASteeringFfb::CalculatePhysicalTorque(
            Config,
            Input,
            Output));

    TestTrue(
        TEXT("Positive rack force produces opposing steering torque"),
        Output.PhysicalSteeringWheelTorqueNm < 0.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTASteeringFfbDampingTest,
    "TorqueAtlas.Vehicle.Steering.FFB.DampingOpposesMotion",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTASteeringFfbDampingTest::RunTest(
    const FString& Parameters)
{
    FTASteeringFfbConfig Config;
    Config.EffectivePinionRadiusM = 0.0;
    Config.ColumnCoulombFrictionNm = 0.0;
    Config.ColumnViscousDampingNmsPerRad = 0.5;

    FTASteeringFfbInput Input;
    Input.SteeringWheelAngularVelocityRadPerSec = 2.0;

    FTASteeringFfbOutput Output;

    TestTrue(
        TEXT("FFB damping solve succeeds"),
        TASteeringFfb::CalculatePhysicalTorque(
            Config,
            Input,
            Output));

    TestTrue(
        TEXT("Damping opposes positive steering-wheel velocity"),
        Output.DampingTorqueNm < 0.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTASteeringFfbAssistTest,
    "TorqueAtlas.Vehicle.Steering.FFB.AssistActsAsPhysicalColumnTorque",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTASteeringFfbAssistTest::RunTest(
    const FString& Parameters)
{
    FTASteeringFfbConfig Config;
    Config.ColumnViscousDampingNmsPerRad = 0.0;
    Config.ColumnCoulombFrictionNm = 0.0;

    FTASteeringFfbInput Unassisted;
    Unassisted.RackForceN = 1000.0;

    FTASteeringFfbInput Assisted =
        Unassisted;

    Assisted.AssistTorqueNm = 5.0;

    FTASteeringFfbOutput UnassistedOutput;
    FTASteeringFfbOutput AssistedOutput;

    TestTrue(
        TEXT("Unassisted solve succeeds"),
        TASteeringFfb::CalculatePhysicalTorque(
            Config,
            Unassisted,
            UnassistedOutput));

    TestTrue(
        TEXT("Assisted solve succeeds"),
        TASteeringFfb::CalculatePhysicalTorque(
            Config,
            Assisted,
            AssistedOutput));

    TestTrue(
        TEXT("Positive assist reduces magnitude of negative rack reaction"),
        FMath::Abs(
            AssistedOutput.PhysicalSteeringWheelTorqueNm)
            < FMath::Abs(
                UnassistedOutput.PhysicalSteeringWheelTorqueNm));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTASteeringFfbDeviceClippingTest,
    "TorqueAtlas.Vehicle.Steering.FFB.DeviceClippingDoesNotChangePhysicsTorque",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTASteeringFfbDeviceClippingTest::RunTest(
    const FString& Parameters)
{
    FTAFfbDeviceConfig Config;
    Config.DeviceMaxTorqueNm = 8.0;
    Config.UserStrength01 = 1.0;

    FTAFfbDeviceOutput Output;

    TestTrue(
        TEXT("FFB device scaling succeeds"),
        TASteeringFfb::ScaleForDevice(
            Config,
            20.0,
            Output));

    TestEqual(
        TEXT("Physical request is preserved for telemetry"),
        Output.RequestedPhysicalTorqueNm,
        20.0);

    TestEqual(
        TEXT("Device command is clipped to device capability"),
        Output.DeviceCommandTorqueNm,
        8.0);

    TestTrue(
        TEXT("Device clipping is reported"),
        Output.bClipped);

    return true;
}

#endif
