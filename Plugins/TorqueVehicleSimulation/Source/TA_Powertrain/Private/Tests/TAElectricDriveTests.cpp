#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TAElectricDrive.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAElectricDriveMotoringTest,
    "TorqueAtlas.Powertrain.EV.Drive.MotoringUsesBattery",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAElectricDriveMotoringTest::RunTest(const FString&)
{
    FTAElectricDriveConfig Config;
    FTAElectricDriveState State;
    TAElectricDrive::InitializeState(State);

    const double BeforeSoc =
        State.Battery.StateOfCharge01;

    FTAElectricDriveInput Input;
    Input.RequestedMotorTorqueNm = 200.0;
    Input.MotorAngularSpeedRadPerSec = 400.0;
    Input.DeltaTimeSeconds = 1.0;

    FTAElectricDriveOutput Output;

    TestTrue(TEXT("EV drive motoring step succeeds"),
        TAElectricDrive::Step(Config, Input, State, Output));

    TestTrue(TEXT("Motor produces positive torque"),
        Output.ActualMotorTorqueNm > 0.0);

    TestTrue(TEXT("Battery supplies positive terminal power"),
        Output.ActualElectricalPowerW > 0.0);

    TestTrue(TEXT("SOC falls while motoring"),
        State.Battery.StateOfCharge01 < BeforeSoc);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAElectricDriveRegenTest,
    "TorqueAtlas.Powertrain.EV.Drive.RegenChargesBattery",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAElectricDriveRegenTest::RunTest(const FString&)
{
    FTAElectricDriveConfig Config;
    FTAElectricDriveState State;
    TAElectricDrive::InitializeState(State);
    State.Battery.StateOfCharge01 = 0.50;

    const double BeforeSoc =
        State.Battery.StateOfCharge01;

    FTAElectricDriveInput Input;
    Input.RequestedMotorTorqueNm = -100.0;
    Input.MotorAngularSpeedRadPerSec = 400.0;
    Input.DeltaTimeSeconds = 1.0;

    FTAElectricDriveOutput Output;

    TestTrue(TEXT("EV drive regen step succeeds"),
        TAElectricDrive::Step(Config, Input, State, Output));

    TestTrue(TEXT("Regen produces negative torque"),
        Output.ActualMotorTorqueNm < 0.0);

    TestTrue(TEXT("Battery terminal power is negative in regen"),
        Output.ActualElectricalPowerW < 0.0);

    TestTrue(TEXT("SOC rises during regen"),
        State.Battery.StateOfCharge01 > BeforeSoc);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAElectricDriveBatteryLimitTest,
    "TorqueAtlas.Powertrain.EV.Drive.BatteryCurrentLimitReducesMotorTorque",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAElectricDriveBatteryLimitTest::RunTest(const FString&)
{
    FTAElectricDriveConfig Limited;
    Limited.Battery.MaxDischargeCurrentA = 50.0;

    FTAElectricDriveConfig Strong =
        Limited;
    Strong.Battery.MaxDischargeCurrentA = 600.0;

    FTAElectricDriveState LimitedState;
    FTAElectricDriveState StrongState;
    TAElectricDrive::InitializeState(LimitedState);
    TAElectricDrive::InitializeState(StrongState);

    FTAElectricDriveInput Input;
    Input.RequestedMotorTorqueNm = 400.0;
    Input.MotorAngularSpeedRadPerSec = 500.0;

    FTAElectricDriveOutput LimitedOutput;
    FTAElectricDriveOutput StrongOutput;

    TestTrue(TEXT("Limited EV solve succeeds"),
        TAElectricDrive::Step(
            Limited, Input, LimitedState, LimitedOutput));

    TestTrue(TEXT("Strong EV solve succeeds"),
        TAElectricDrive::Step(
            Strong, Input, StrongState, StrongOutput));

    TestTrue(TEXT("Battery current limit reduces actual motor torque"),
        FMath::Abs(LimitedOutput.ActualMotorTorqueNm)
            < FMath::Abs(StrongOutput.ActualMotorTorqueNm));

    TestTrue(TEXT("Battery-limited state is reported"),
        LimitedOutput.bBatteryLimited);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAElectricDriveFullBatteryRegenTest,
    "TorqueAtlas.Powertrain.EV.Drive.FullBatteryRejectsRegen",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAElectricDriveFullBatteryRegenTest::RunTest(const FString&)
{
    FTAElectricDriveConfig Config;
    FTAElectricDriveState State;
    TAElectricDrive::InitializeState(State);
    State.Battery.StateOfCharge01 = 1.0;

    FTAElectricDriveInput Input;
    Input.RequestedMotorTorqueNm = -100.0;
    Input.MotorAngularSpeedRadPerSec = 400.0;

    FTAElectricDriveOutput Output;

    TestTrue(TEXT("Full-battery regen solve succeeds"),
        TAElectricDrive::Step(Config, Input, State, Output));

    TestTrue(TEXT("Full battery rejects motor regen torque"),
        FMath::IsNearlyZero(Output.ActualMotorTorqueNm, 1.0e-9));

    TestTrue(TEXT("Regen rejection reports battery limit"),
        Output.bBatteryLimited);

    return true;
}

#endif
