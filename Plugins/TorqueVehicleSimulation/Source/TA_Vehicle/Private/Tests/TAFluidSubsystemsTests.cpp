#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TAFluidSubsystems.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAFuelPumpAuthorityTest,
    "TorqueAtlas.Vehicle.Fluids.FuelPumpAuthorityControlsRailPressure",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAFuelPumpAuthorityTest::RunTest(
    const FString& Parameters)
{
    FTAFuelSystemConfig Config;

    FTAFuelSystemState Powered;
    FTAFuelSystemState Unpowered;
    TAFluidSubsystems::InitializeFuelState(Config, Powered);
    TAFluidSubsystems::InitializeFuelState(Config, Unpowered);

    FTAFuelSystemInput PoweredInput;
    PoweredInput.PumpAuthority01 = 1.0;
    PoweredInput.DeltaTimeSeconds = 0.5;

    FTAFuelSystemInput UnpoweredInput =
        PoweredInput;
    UnpoweredInput.PumpAuthority01 = 0.0;

    FTAFuelSystemOutput PoweredOutput;
    FTAFuelSystemOutput UnpoweredOutput;

    TestTrue(
        TEXT("Powered fuel step succeeds"),
        TAFluidSubsystems::StepFuel(
            Config,
            PoweredInput,
            Powered,
            PoweredOutput));

    TestTrue(
        TEXT("Unpowered fuel step succeeds"),
        TAFluidSubsystems::StepFuel(
            Config,
            UnpoweredInput,
            Unpowered,
            UnpoweredOutput));

    TestTrue(
        TEXT("Powered pump creates higher rail pressure"),
        PoweredOutput.RailPressurePa
            > UnpoweredOutput.RailPressurePa);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAFuelConsumptionTest,
    "TorqueAtlas.Vehicle.Fluids.FuelConsumptionReducesReservoirMass",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAFuelConsumptionTest::RunTest(
    const FString& Parameters)
{
    FTAFuelSystemConfig Config;
    FTAFuelSystemState State;
    TAFluidSubsystems::InitializeFuelState(Config, State);

    const double InitialMassKg =
        State.Reservoir.MassKg;

    FTAFuelSystemInput Input;
    Input.EngineFuelDemandKgPerSec = 0.01;
    Input.DeltaTimeSeconds = 10.0;

    FTAFuelSystemOutput Output;

    TestTrue(
        TEXT("Fuel consumption step succeeds"),
        TAFluidSubsystems::StepFuel(
            Config,
            Input,
            State,
            Output));

    TestTrue(
        TEXT("Engine fuel demand reduces reservoir mass"),
        Output.FuelMassKg < InitialMassKg);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAOilPressureTest,
    "TorqueAtlas.Vehicle.Fluids.OilPressureDependsOnRpmPumpAndMass",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAOilPressureTest::RunTest(
    const FString& Parameters)
{
    FTAOilSystemConfig Config;

    FTAOilSystemState Healthy;
    FTAOilSystemState Damaged;
    TAFluidSubsystems::InitializeOilState(Config, Healthy);
    TAFluidSubsystems::InitializeOilState(Config, Damaged);

    FTAOilSystemInput HealthyInput;
    HealthyInput.EngineRPM = 3000.0;
    HealthyInput.PumpHealth01 = 1.0;
    HealthyInput.DeltaTimeSeconds = 0.5;

    FTAOilSystemInput DamagedInput =
        HealthyInput;
    DamagedInput.PumpHealth01 = 0.25;

    FTAOilSystemOutput HealthyOutput;
    FTAOilSystemOutput DamagedOutput;

    TestTrue(
        TEXT("Healthy oil step succeeds"),
        TAFluidSubsystems::StepOil(
            Config,
            HealthyInput,
            Healthy,
            HealthyOutput));

    TestTrue(
        TEXT("Damaged oil-pump step succeeds"),
        TAFluidSubsystems::StepOil(
            Config,
            DamagedInput,
            Damaged,
            DamagedOutput));

    TestTrue(
        TEXT("Pump damage lowers gallery pressure"),
        DamagedOutput.GalleryPressurePa
            < HealthyOutput.GalleryPressurePa);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTACoolantAuthorityTest,
    "TorqueAtlas.Vehicle.Fluids.CoolingAuthorityRequiresCirculationAndRadiator",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTACoolantAuthorityTest::RunTest(
    const FString& Parameters)
{
    FTACoolantSystemConfig Config;

    FTACoolantSystemState Healthy;
    FTACoolantSystemState Damaged;
    TAFluidSubsystems::InitializeCoolantState(Config, Healthy);
    TAFluidSubsystems::InitializeCoolantState(Config, Damaged);

    FTACoolantSystemInput HealthyInput;
    HealthyInput.EngineRPM = 3000.0;
    HealthyInput.PumpHealth01 = 1.0;
    HealthyInput.RadiatorHealth01 = 1.0;
    HealthyInput.AirflowAuthority01 = 1.0;

    FTACoolantSystemInput DamagedInput =
        HealthyInput;
    DamagedInput.RadiatorHealth01 = 0.2;

    FTACoolantSystemOutput HealthyOutput;
    FTACoolantSystemOutput DamagedOutput;

    TestTrue(
        TEXT("Healthy coolant step succeeds"),
        TAFluidSubsystems::StepCoolant(
            Config,
            HealthyInput,
            Healthy,
            HealthyOutput));

    TestTrue(
        TEXT("Damaged radiator coolant step succeeds"),
        TAFluidSubsystems::StepCoolant(
            Config,
            DamagedInput,
            Damaged,
            DamagedOutput));

    TestTrue(
        TEXT("Radiator damage lowers cooling authority"),
        DamagedOutput.CoolingAuthority01
            < HealthyOutput.CoolingAuthority01);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAFluidSubsystemLeakTest,
    "TorqueAtlas.Vehicle.Fluids.CoolantLeakLowersMassAndCoolingAuthority",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAFluidSubsystemLeakTest::RunTest(
    const FString& Parameters)
{
    FTACoolantSystemConfig Config;

    FTACoolantSystemState State;
    TAFluidSubsystems::InitializeCoolantState(Config, State);

    const double InitialMassKg =
        State.Reservoir.MassKg;

    FTACoolantSystemInput Input;
    Input.EngineRPM = 3000.0;
    Input.LeakAreaM2 = 1.0e-5;
    Input.SystemPressurePa = 250000.0;
    Input.DeltaTimeSeconds = 5.0;

    FTACoolantSystemOutput Output;

    TestTrue(
        TEXT("Leaking coolant step succeeds"),
        TAFluidSubsystems::StepCoolant(
            Config,
            Input,
            State,
            Output));

    TestTrue(
        TEXT("Coolant leak reduces mass"),
        Output.CoolantMassKg < InitialMassKg);

    TestTrue(
        TEXT("Leak reports positive lost mass"),
        Output.Leak.LeakedMassThisStepKg > 0.0);

    return true;
}

#endif
