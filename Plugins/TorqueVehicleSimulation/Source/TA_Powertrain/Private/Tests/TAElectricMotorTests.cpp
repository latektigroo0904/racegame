#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "TAElectricMotor.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
 FTAElectricMotorMotoringTest,
 "TorqueAtlas.Powertrain.EV.Motor.MotoringConsumesElectricalPower",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FTAElectricMotorMotoringTest::RunTest(const FString&)
{
 FTAElectricMotorConfig C; FTAElectricMotorInput I; I.RequestedTorqueNm=200; I.AngularSpeedRadPerSec=500;
 FTAElectricMotorOutput O;
 TestTrue(TEXT("Motor solve succeeds"),TAElectricMotor::Calculate(C,I,O));
 TestTrue(TEXT("Mechanical power positive"),O.MechanicalPowerW>0);
 TestTrue(TEXT("Electrical draw exceeds mechanical output"),O.ElectricalPowerW>O.MechanicalPowerW);
 return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
 FTAElectricMotorRegenTest,
 "TorqueAtlas.Powertrain.EV.Motor.RegenReturnsElectricalPower",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FTAElectricMotorRegenTest::RunTest(const FString&)
{
 FTAElectricMotorConfig C; FTAElectricMotorInput I; I.RequestedTorqueNm=-100; I.AngularSpeedRadPerSec=500;
 FTAElectricMotorOutput O;
 TestTrue(TEXT("Regen solve succeeds"),TAElectricMotor::Calculate(C,I,O));
 TestTrue(TEXT("Regen mechanical power negative"),O.MechanicalPowerW<0);
 TestTrue(TEXT("Regen electrical power negative"),O.ElectricalPowerW<0);
 TestTrue(TEXT("Regen returns less magnitude than mechanical absorption"),FMath::Abs(O.ElectricalPowerW)<FMath::Abs(O.MechanicalPowerW));
 return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
 FTAElectricMotorPowerLimitTest,
 "TorqueAtlas.Powertrain.EV.Motor.PowerLimitReducesHighSpeedTorque",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FTAElectricMotorPowerLimitTest::RunTest(const FString&)
{
 FTAElectricMotorConfig C; C.MaxMotoringMechanicalPowerW=100000;
 FTAElectricMotorInput I; I.RequestedTorqueNm=450; I.AngularSpeedRadPerSec=1000;
 FTAElectricMotorOutput O;
 TestTrue(TEXT("High-speed motor solve succeeds"),TAElectricMotor::Calculate(C,I,O));
 TestTrue(TEXT("Power limit reduces torque"),O.ActualTorqueNm<=100.0+1e-9);
 TestTrue(TEXT("Torque limit reported"),O.bTorqueLimited);
 return true;
}
#endif
