#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "TAHvBattery.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAHvBatteryDischargeTest,
    "TorqueAtlas.Powertrain.EV.Battery.DischargeLowersSoc",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTAHvBatteryDischargeTest::RunTest(const FString&)
{
    FTAHvBatteryConfig C; FTAHvBatteryState S;
    const double Before=S.StateOfCharge01;
    FTAHvBatteryInput I; I.RequestedTerminalPowerW=50000.0; I.DeltaTimeSeconds=10.0;
    FTAHvBatteryOutput O;
    TestTrue(TEXT("Battery discharge succeeds"),TAHvBattery::Step(C,I,S,O));
    TestTrue(TEXT("Discharge current positive"),O.CurrentA>0.0);
    TestTrue(TEXT("SOC falls"),S.StateOfCharge01<Before);
    TestTrue(TEXT("Internal heat positive"),O.InternalHeatPowerW>0.0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAHvBatteryChargeTest,
    "TorqueAtlas.Powertrain.EV.Battery.RegenChargeRaisesSoc",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTAHvBatteryChargeTest::RunTest(const FString&)
{
    FTAHvBatteryConfig C; FTAHvBatteryState S; S.StateOfCharge01=0.5;
    const double Before=S.StateOfCharge01;
    FTAHvBatteryInput I; I.RequestedTerminalPowerW=-30000.0; I.DeltaTimeSeconds=10.0;
    FTAHvBatteryOutput O;
    TestTrue(TEXT("Battery charge succeeds"),TAHvBattery::Step(C,I,S,O));
    TestTrue(TEXT("Charge current negative"),O.CurrentA<0.0);
    TestTrue(TEXT("SOC rises"),S.StateOfCharge01>Before);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAHvBatteryCurrentLimitTest,
    "TorqueAtlas.Powertrain.EV.Battery.CurrentLimitBoundsPower",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTAHvBatteryCurrentLimitTest::RunTest(const FString&)
{
    FTAHvBatteryConfig C; C.MaxDischargeCurrentA=50.0;
    FTAHvBatteryState S; FTAHvBatteryInput I; I.RequestedTerminalPowerW=500000.0;
    FTAHvBatteryOutput O;
    TestTrue(TEXT("Limited battery solve succeeds"),TAHvBattery::Step(C,I,S,O));
    TestTrue(TEXT("Current is limited"),O.CurrentA<=50.0+1e-12);
    TestTrue(TEXT("Power-limited flag set"),O.bPowerLimited);
    return true;
}
#endif
