#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TAStarterMotor.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAStarterMotorEngagementTest,
    "TorqueAtlas.Powertrain.Starter.EngagementProducesCrankTorque",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAStarterMotorEngagementTest::RunTest(
    const FString& Parameters)
{
    FTAStarterMotorConfig Config;

    FTAStarterMotorInput Input;
    Input.BusVoltageV = 12.0;
    Input.bEngaged = true;

    FTAStarterMotorOutput Output;

    TestTrue(
        TEXT("Starter calculation succeeds"),
        TAStarterMotor::Calculate(
            Config,
            Input,
            Output));

    TestTrue(
        TEXT("Engaged starter draws current"),
        Output.CurrentA > 0.0);

    TestTrue(
        TEXT("Engaged starter produces crankshaft torque"),
        Output.CrankshaftTorqueNm > 0.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAStarterMotorBackEmfTest,
    "TorqueAtlas.Powertrain.Starter.BackEmfReducesCurrentWithSpeed",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAStarterMotorBackEmfTest::RunTest(
    const FString& Parameters)
{
    FTAStarterMotorConfig Config;

    FTAStarterMotorInput Slow;
    Slow.BusVoltageV = 12.0;
    Slow.EngineAngularSpeedRadPerSec = 0.0;
    Slow.bEngaged = true;

    FTAStarterMotorInput Fast =
        Slow;
    Fast.EngineAngularSpeedRadPerSec = 50.0;

    FTAStarterMotorOutput SlowOutput;
    FTAStarterMotorOutput FastOutput;

    TestTrue(
        TEXT("Slow starter solve succeeds"),
        TAStarterMotor::Calculate(
            Config,
            Slow,
            SlowOutput));

    TestTrue(
        TEXT("Fast starter solve succeeds"),
        TAStarterMotor::Calculate(
            Config,
            Fast,
            FastOutput));

    TestTrue(
        TEXT("Back EMF rises with engine speed"),
        FastOutput.BackEmfVoltageV
            > SlowOutput.BackEmfVoltageV);

    TestTrue(
        TEXT("Back EMF reduces starter current"),
        FastOutput.CurrentA
            < SlowOutput.CurrentA);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAStarterMotorVoltageTest,
    "TorqueAtlas.Powertrain.Starter.LowVoltageReducesTorque",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAStarterMotorVoltageTest::RunTest(
    const FString& Parameters)
{
    FTAStarterMotorConfig Config;

    FTAStarterMotorInput High;
    High.BusVoltageV = 12.0;
    High.bEngaged = true;

    FTAStarterMotorInput Low =
        High;
    Low.BusVoltageV = 7.0;

    FTAStarterMotorOutput HighOutput;
    FTAStarterMotorOutput LowOutput;

    TestTrue(
        TEXT("High-voltage starter solve succeeds"),
        TAStarterMotor::Calculate(
            Config,
            High,
            HighOutput));

    TestTrue(
        TEXT("Low-voltage starter solve succeeds"),
        TAStarterMotor::Calculate(
            Config,
            Low,
            LowOutput));

    TestTrue(
        TEXT("Low bus voltage reduces crankshaft torque"),
        LowOutput.CrankshaftTorqueNm
            < HighOutput.CrankshaftTorqueNm);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAStarterMotorDisengagedTest,
    "TorqueAtlas.Powertrain.Starter.DisengagedIsZeroLoad",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAStarterMotorDisengagedTest::RunTest(
    const FString& Parameters)
{
    FTAStarterMotorConfig Config;

    FTAStarterMotorInput Input;
    Input.BusVoltageV = 12.0;
    Input.bEngaged = false;

    FTAStarterMotorOutput Output;

    TestTrue(
        TEXT("Disengaged starter solve succeeds"),
        TAStarterMotor::Calculate(
            Config,
            Input,
            Output));

    TestEqual(
        TEXT("Disengaged starter current is zero"),
        Output.CurrentA,
        0.0);

    TestEqual(
        TEXT("Disengaged starter torque is zero"),
        Output.CrankshaftTorqueNm,
        0.0);

    return true;
}

#endif
