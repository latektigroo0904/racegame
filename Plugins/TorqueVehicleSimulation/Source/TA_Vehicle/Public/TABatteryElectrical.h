#pragma once

#include "CoreMinimal.h"

struct TA_VEHICLE_API FTABatteryConfig
{
    double CapacityAh = 60.0;

    double EmptyOpenCircuitVoltageV = 11.6;
    double FullOpenCircuitVoltageV = 12.7;

    double InternalResistanceOhm = 0.020;

    double MaxDischargeCurrentA = 700.0;
    double MaxChargeCurrentA = 120.0;
};

struct TA_VEHICLE_API FTABatteryState
{
    double StateOfCharge01 = 1.0;
};

struct TA_VEHICLE_API FTABatteryInput
{
    // Positive load current draws from the battery.
    double RequestedLoadCurrentA = 0.0;

    // Positive source current charges/supplies the bus.
    double AlternatorCurrentA = 0.0;

    double DeltaTimeSeconds = 1.0 / 240.0;
};

struct TA_VEHICLE_API FTABatteryOutput
{
    double OpenCircuitVoltageV = 0.0;
    double TerminalVoltageV = 0.0;

    // Positive means battery discharge, negative means charge.
    double BatteryCurrentA = 0.0;

    double DeliveredLoadCurrentA = 0.0;
    double AcceptedChargeCurrentA = 0.0;

    double StateOfCharge01 = 0.0;
};

namespace TABatteryElectrical
{
    TA_VEHICLE_API bool ValidateConfig(
        const FTABatteryConfig& Config);

    TA_VEHICLE_API double CalculateOpenCircuitVoltageV(
        const FTABatteryConfig& Config,
        double StateOfCharge01);

    TA_VEHICLE_API bool Step(
        const FTABatteryConfig& Config,
        const FTABatteryInput& Input,
        FTABatteryState& InOutState,
        FTABatteryOutput& OutOutput);
}
