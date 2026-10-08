#pragma once

#include "CoreMinimal.h"
#include "TABatteryElectrical.h"
#include "TAAlternator.h"

enum class ETAElectricalConsumer : uint8
{
    EcuIgnition = 0,
    FuelPump = 1,
    CoolingFan = 2,
    AbsController = 3,
    LightingAccessories = 4,
    Count = 5
};

constexpr int32 TAElectricalConsumerCount =
    static_cast<int32>(ETAElectricalConsumer::Count);

struct TA_VEHICLE_API FTAElectricalConsumerConfig
{
    double RequestedCurrentA = 0.0;
    double MinimumOperatingVoltageV = 9.0;
    double NominalOperatingVoltageV = 12.0;
    bool bCritical = false;
};

struct TA_VEHICLE_API FTAElectricalBusConfig
{
    FTABatteryConfig Battery;
    FTAAlternatorConfig Alternator;

    FTAElectricalConsumerConfig Consumers[TAElectricalConsumerCount];

    FTAElectricalBusConfig();
};

struct TA_VEHICLE_API FTAElectricalBusState
{
    FTABatteryState Battery;

    double MainBusHealth01 = 1.0;
    double ConsumerConnectionHealth01[TAElectricalConsumerCount] =
        { 1.0, 1.0, 1.0, 1.0, 1.0 };
};

struct TA_VEHICLE_API FTAElectricalBusInput
{
    double EngineRPM = 0.0;
    double AlternatorHealth01 = 1.0;

    bool bStarterEngaged = false;
    double StarterRequestedCurrentA = 0.0;

    double DeltaTimeSeconds = 1.0 / 240.0;
};

struct TA_VEHICLE_API FTAElectricalBusOutput
{
    FTABatteryOutput Battery;
    FTAAlternatorOutput Alternator;

    double BusVoltageV = 0.0;
    double TotalRequestedLoadCurrentA = 0.0;
    double TotalDeliveredLoadCurrentA = 0.0;

    double ConsumerDeliveredCurrentA[TAElectricalConsumerCount] =
        { 0.0, 0.0, 0.0, 0.0, 0.0 };

    double ConsumerVoltageAuthority01[TAElectricalConsumerCount] =
        { 0.0, 0.0, 0.0, 0.0, 0.0 };

    double StarterDeliveredCurrentA = 0.0;

    bool bCriticalBrownout = false;
};

namespace TAElectricalBus
{
    TA_VEHICLE_API bool ValidateConfig(
        const FTAElectricalBusConfig& Config);

    TA_VEHICLE_API void InitializeState(
        FTAElectricalBusState& OutState);

    TA_VEHICLE_API double CalculateVoltageAuthority01(
        double BusVoltageV,
        const FTAElectricalConsumerConfig& Consumer);

    TA_VEHICLE_API bool Step(
        const FTAElectricalBusConfig& Config,
        const FTAElectricalBusInput& Input,
        FTAElectricalBusState& InOutState,
        FTAElectricalBusOutput& OutOutput);
}
