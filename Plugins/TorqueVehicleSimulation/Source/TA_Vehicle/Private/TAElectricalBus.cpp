#include "TAElectricalBus.h"

FTAElectricalBusConfig::FTAElectricalBusConfig()
{
    Consumers[static_cast<int32>(ETAElectricalConsumer::EcuIgnition)] =
        FTAElectricalConsumerConfig{
            8.0,
            8.5,
            11.0,
            true
        };

    Consumers[static_cast<int32>(ETAElectricalConsumer::FuelPump)] =
        FTAElectricalConsumerConfig{
            12.0,
            9.0,
            12.0,
            true
        };

    Consumers[static_cast<int32>(ETAElectricalConsumer::CoolingFan)] =
        FTAElectricalConsumerConfig{
            25.0,
            8.0,
            12.0,
            false
        };

    Consumers[static_cast<int32>(ETAElectricalConsumer::AbsController)] =
        FTAElectricalConsumerConfig{
            5.0,
            8.5,
            11.0,
            true
        };

    Consumers[static_cast<int32>(ETAElectricalConsumer::LightingAccessories)] =
        FTAElectricalConsumerConfig{
            18.0,
            7.0,
            12.0,
            false
        };
}

bool TAElectricalBus::ValidateConfig(
    const FTAElectricalBusConfig& Config)
{
    if (!TABatteryElectrical::ValidateConfig(
            Config.Battery)
        || !TAAlternator::ValidateConfig(
            Config.Alternator))
    {
        return false;
    }

    for (int32 Index = 0;
         Index < TAElectricalConsumerCount;
         ++Index)
    {
        const FTAElectricalConsumerConfig& Consumer =
            Config.Consumers[Index];

        if (!FMath::IsFinite(Consumer.RequestedCurrentA)
            || Consumer.RequestedCurrentA < 0.0
            || !FMath::IsFinite(
                Consumer.MinimumOperatingVoltageV)
            || Consumer.MinimumOperatingVoltageV < 0.0
            || !FMath::IsFinite(
                Consumer.NominalOperatingVoltageV)
            || Consumer.NominalOperatingVoltageV
                <= Consumer.MinimumOperatingVoltageV)
        {
            return false;
        }
    }

    return true;
}

void TAElectricalBus::InitializeState(
    FTAElectricalBusState& OutState)
{
    OutState =
        FTAElectricalBusState{};
}

double TAElectricalBus::CalculateVoltageAuthority01(
    const double BusVoltageV,
    const FTAElectricalConsumerConfig& Consumer)
{
    if (!FMath::IsFinite(BusVoltageV))
    {
        return 0.0;
    }

    return FMath::Clamp(
        (BusVoltageV
            - Consumer.MinimumOperatingVoltageV)
        / FMath::Max(
            Consumer.NominalOperatingVoltageV
                - Consumer.MinimumOperatingVoltageV,
            UE_DOUBLE_SMALL_NUMBER),
        0.0,
        1.0);
}

bool TAElectricalBus::Step(
    const FTAElectricalBusConfig& Config,
    const FTAElectricalBusInput& Input,
    FTAElectricalBusState& InOutState,
    FTAElectricalBusOutput& OutOutput)
{
    OutOutput =
        FTAElectricalBusOutput{};

    if (!ValidateConfig(Config)
        || !FMath::IsFinite(Input.EngineRPM)
        || Input.EngineRPM < 0.0
        || !FMath::IsFinite(Input.AlternatorHealth01)
        || !FMath::IsFinite(Input.StarterRequestedCurrentA)
        || Input.StarterRequestedCurrentA < 0.0
        || !FMath::IsFinite(Input.DeltaTimeSeconds)
        || Input.DeltaTimeSeconds <= 0.0
        || !FMath::IsFinite(InOutState.MainBusHealth01))
    {
        return false;
    }

    const double MainBusHealth01 =
        FMath::Clamp(
            InOutState.MainBusHealth01,
            0.0,
            1.0);

    double ConsumerRequestedCurrentA =
        0.0;

    for (int32 Index = 0;
         Index < TAElectricalConsumerCount;
         ++Index)
    {
        const double ConnectionHealth01 =
            FMath::Clamp(
                InOutState.ConsumerConnectionHealth01[Index],
                0.0,
                1.0);

        ConsumerRequestedCurrentA +=
            Config.Consumers[Index].RequestedCurrentA
            * ConnectionHealth01
            * MainBusHealth01;
    }

    const double StarterRequestedCurrentA =
        Input.bStarterEngaged
        ? Input.StarterRequestedCurrentA
            * MainBusHealth01
        : 0.0;

    OutOutput.TotalRequestedLoadCurrentA =
        ConsumerRequestedCurrentA
        + StarterRequestedCurrentA;

    const double BatteryOpenCircuitVoltageV =
        TABatteryElectrical::CalculateOpenCircuitVoltageV(
            Config.Battery,
            InOutState.Battery.StateOfCharge01);

    FTAAlternatorInput AlternatorInput;
    AlternatorInput.EngineRPM =
        Input.EngineRPM;

    AlternatorInput.BusVoltageV =
        BatteryOpenCircuitVoltageV;

    AlternatorInput.Health01 =
        FMath::Clamp(
            Input.AlternatorHealth01,
            0.0,
            1.0);

    if (!TAAlternator::Calculate(
            Config.Alternator,
            AlternatorInput,
            OutOutput.Alternator))
    {
        return false;
    }

    FTABatteryInput BatteryInput;
    BatteryInput.RequestedLoadCurrentA =
        OutOutput.TotalRequestedLoadCurrentA;

    BatteryInput.AlternatorCurrentA =
        OutOutput.Alternator.OutputCurrentA;

    BatteryInput.DeltaTimeSeconds =
        Input.DeltaTimeSeconds;

    if (!TABatteryElectrical::Step(
            Config.Battery,
            BatteryInput,
            InOutState.Battery,
            OutOutput.Battery))
    {
        return false;
    }

    OutOutput.BusVoltageV =
        OutOutput.Battery.TerminalVoltageV
        * MainBusHealth01;

    OutOutput.TotalDeliveredLoadCurrentA =
        OutOutput.Battery.DeliveredLoadCurrentA
        * MainBusHealth01;

    const double DeliveryFraction01 =
        OutOutput.TotalRequestedLoadCurrentA
            > UE_DOUBLE_SMALL_NUMBER
        ? FMath::Clamp(
            OutOutput.TotalDeliveredLoadCurrentA
                / OutOutput.TotalRequestedLoadCurrentA,
            0.0,
            1.0)
        : 1.0;

    OutOutput.StarterDeliveredCurrentA =
        StarterRequestedCurrentA
        * DeliveryFraction01;

    for (int32 Index = 0;
         Index < TAElectricalConsumerCount;
         ++Index)
    {
        const FTAElectricalConsumerConfig& Consumer =
            Config.Consumers[Index];

        const double ConnectionHealth01 =
            FMath::Clamp(
                InOutState.ConsumerConnectionHealth01[Index],
                0.0,
                1.0);

        OutOutput.ConsumerDeliveredCurrentA[Index] =
            Consumer.RequestedCurrentA
            * ConnectionHealth01
            * MainBusHealth01
            * DeliveryFraction01;

        OutOutput.ConsumerVoltageAuthority01[Index] =
            CalculateVoltageAuthority01(
                OutOutput.BusVoltageV,
                Consumer)
            * ConnectionHealth01
            * MainBusHealth01;

        if (Consumer.bCritical
            && OutOutput.ConsumerVoltageAuthority01[Index]
                <= 0.0)
        {
            OutOutput.bCriticalBrownout =
                true;
        }
    }

    return true;
}
