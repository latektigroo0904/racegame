#include "TAPowerSupportStack.h"

bool TAPowerSupportStack::ValidateConfig(
    const FTAPowerSupportStackConfig& Config)
{
    return
        TAElectricalBus::ValidateConfig(
            Config.Electrical)
        && TAFluidSubsystems::ValidateFuelConfig(
            Config.Fuel)
        && TAFluidSubsystems::ValidateCoolantConfig(
            Config.Coolant)
        && FMath::IsFinite(
            Config.CoolingFanMaximumAirflowAuthority01)
        && Config.CoolingFanMaximumAirflowAuthority01 >= 0.0
        && Config.CoolingFanMaximumAirflowAuthority01 <= 1.0;
}

void TAPowerSupportStack::InitializeState(
    const FTAPowerSupportStackConfig& Config,
    FTAPowerSupportStackState& OutState)
{
    OutState =
        FTAPowerSupportStackState{};

    TAElectricalBus::InitializeState(
        OutState.Electrical);

    TAFluidSubsystems::InitializeFuelState(
        Config.Fuel,
        OutState.Fuel);

    TAFluidSubsystems::InitializeCoolantState(
        Config.Coolant,
        OutState.Coolant);
}

bool TAPowerSupportStack::Step(
    const FTAPowerSupportStackConfig& Config,
    const FTAPowerSupportStackInput& Input,
    FTAPowerSupportStackState& InOutState,
    FTAPowerSupportStackOutput& OutOutput)
{
    OutOutput =
        FTAPowerSupportStackOutput{};

    if (!ValidateConfig(Config)
        || !FMath::IsFinite(Input.EngineRPM)
        || Input.EngineRPM < 0.0
        || !FMath::IsFinite(Input.StarterRequestedCurrentA)
        || Input.StarterRequestedCurrentA < 0.0
        || !FMath::IsFinite(Input.AlternatorHealth01)
        || !FMath::IsFinite(Input.EngineFuelDemandKgPerSec)
        || Input.EngineFuelDemandKgPerSec < 0.0
        || !FMath::IsFinite(Input.FuelLeakAreaM2)
        || Input.FuelLeakAreaM2 < 0.0
        || !FMath::IsFinite(Input.CoolantPumpHealth01)
        || !FMath::IsFinite(Input.RadiatorHealth01)
        || !FMath::IsFinite(Input.RamAirflowAuthority01)
        || !FMath::IsFinite(Input.CoolantLeakAreaM2)
        || Input.CoolantLeakAreaM2 < 0.0
        || !FMath::IsFinite(Input.CoolantSystemPressurePa)
        || Input.CoolantSystemPressurePa < 0.0
        || !FMath::IsFinite(Input.AmbientPressurePa)
        || Input.AmbientPressurePa < 0.0
        || !FMath::IsFinite(Input.DeltaTimeSeconds)
        || Input.DeltaTimeSeconds <= 0.0)
    {
        return false;
    }

    FTAElectricalBusInput ElectricalInput;
    ElectricalInput.EngineRPM =
        Input.EngineRPM;

    ElectricalInput.AlternatorHealth01 =
        Input.AlternatorHealth01;

    ElectricalInput.bStarterEngaged =
        Input.bStarterEngaged;

    ElectricalInput.StarterRequestedCurrentA =
        Input.StarterRequestedCurrentA;

    ElectricalInput.DeltaTimeSeconds =
        Input.DeltaTimeSeconds;

    if (!TAElectricalBus::Step(
            Config.Electrical,
            ElectricalInput,
            InOutState.Electrical,
            OutOutput.Electrical))
    {
        return false;
    }

    const int32 EcuIndex =
        static_cast<int32>(
            ETAElectricalConsumer::EcuIgnition);

    const int32 FuelPumpIndex =
        static_cast<int32>(
            ETAElectricalConsumer::FuelPump);

    const int32 CoolingFanIndex =
        static_cast<int32>(
            ETAElectricalConsumer::CoolingFan);

    OutOutput.EngineControlAuthority01 =
        OutOutput.Electrical
            .ConsumerVoltageAuthority01[EcuIndex];

    OutOutput.FuelPumpAuthority01 =
        OutOutput.Electrical
            .ConsumerVoltageAuthority01[FuelPumpIndex];

    OutOutput.CoolingFanAuthority01 =
        OutOutput.Electrical
            .ConsumerVoltageAuthority01[CoolingFanIndex];

    OutOutput.StarterCurrentAuthority01 =
        Input.bStarterEngaged
        && Input.StarterRequestedCurrentA
            > UE_DOUBLE_SMALL_NUMBER
        ? FMath::Clamp(
            OutOutput.Electrical.StarterDeliveredCurrentA
                / Input.StarterRequestedCurrentA,
            0.0,
            1.0)
        : 1.0;

    FTAFuelSystemInput FuelInput;
    FuelInput.PumpAuthority01 =
        OutOutput.FuelPumpAuthority01;

    FuelInput.EngineFuelDemandKgPerSec =
        Input.EngineFuelDemandKgPerSec;

    FuelInput.LeakAreaM2 =
        Input.FuelLeakAreaM2;

    FuelInput.TankPressurePa =
        Input.AmbientPressurePa;

    FuelInput.AmbientPressurePa =
        Input.AmbientPressurePa;

    FuelInput.DeltaTimeSeconds =
        Input.DeltaTimeSeconds;

    if (!TAFluidSubsystems::StepFuel(
            Config.Fuel,
            FuelInput,
            InOutState.Fuel,
            OutOutput.Fuel))
    {
        return false;
    }

    const double FanAirflowAuthority01 =
        OutOutput.CoolingFanAuthority01
        * Config.CoolingFanMaximumAirflowAuthority01;

    OutOutput.EffectiveCoolingAirflowAuthority01 =
        FMath::Clamp(
            FMath::Max(
                Input.RamAirflowAuthority01,
                FanAirflowAuthority01),
            0.0,
            1.0);

    FTACoolantSystemInput CoolantInput;
    CoolantInput.EngineRPM =
        Input.EngineRPM;

    CoolantInput.PumpHealth01 =
        Input.CoolantPumpHealth01;

    CoolantInput.RadiatorHealth01 =
        Input.RadiatorHealth01;

    CoolantInput.AirflowAuthority01 =
        OutOutput.EffectiveCoolingAirflowAuthority01;

    CoolantInput.LeakAreaM2 =
        Input.CoolantLeakAreaM2;

    CoolantInput.SystemPressurePa =
        Input.CoolantSystemPressurePa;

    CoolantInput.AmbientPressurePa =
        Input.AmbientPressurePa;

    CoolantInput.DeltaTimeSeconds =
        Input.DeltaTimeSeconds;

    return TAFluidSubsystems::StepCoolant(
        Config.Coolant,
        CoolantInput,
        InOutState.Coolant,
        OutOutput.Coolant);
}
