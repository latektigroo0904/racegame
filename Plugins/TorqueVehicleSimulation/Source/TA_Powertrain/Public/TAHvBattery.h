#pragma once

#include "CoreMinimal.h"

struct TA_POWERTRAIN_API FTAHvBatteryConfig
{
    double CapacityKWh = 75.0;

    double EmptyOpenCircuitVoltageV = 320.0;
    double FullOpenCircuitVoltageV = 400.0;

    double InternalResistanceOhm = 0.080;

    double MaxDischargeCurrentA = 600.0;
    double MaxChargeCurrentA = 350.0;
};

struct TA_POWERTRAIN_API FTAHvBatteryState
{
    double StateOfCharge01 = 0.80;
};

struct TA_POWERTRAIN_API FTAHvBatteryInput
{
    // Positive requests discharge to the DC bus. Negative requests charge.
    double RequestedTerminalPowerW = 0.0;
    double DeltaTimeSeconds = 1.0 / 240.0;
};

struct TA_POWERTRAIN_API FTAHvBatteryOutput
{
    double OpenCircuitVoltageV = 0.0;
    double TerminalVoltageV = 0.0;
    double CurrentA = 0.0;

    double ActualTerminalPowerW = 0.0;
    double ChemicalPowerW = 0.0;
    double InternalHeatPowerW = 0.0;

    double StateOfCharge01 = 0.0;
    bool bPowerLimited = false;
};

namespace TAHvBattery
{
    TA_POWERTRAIN_API bool ValidateConfig(
        const FTAHvBatteryConfig& Config);

    TA_POWERTRAIN_API double CalculateOpenCircuitVoltageV(
        const FTAHvBatteryConfig& Config,
        double StateOfCharge01);

    TA_POWERTRAIN_API bool Step(
        const FTAHvBatteryConfig& Config,
        const FTAHvBatteryInput& Input,
        FTAHvBatteryState& InOutState,
        FTAHvBatteryOutput& OutOutput);
}
