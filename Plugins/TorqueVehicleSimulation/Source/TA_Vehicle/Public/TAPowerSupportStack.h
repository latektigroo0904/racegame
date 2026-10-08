#pragma once

#include "CoreMinimal.h"
#include "TAElectricalBus.h"
#include "TAFluidSubsystems.h"

struct TA_VEHICLE_API FTAPowerSupportStackConfig
{
    FTAElectricalBusConfig Electrical;
    FTAFuelSystemConfig Fuel;
    FTACoolantSystemConfig Coolant;

    double CoolingFanMaximumAirflowAuthority01 = 0.55;
};

struct TA_VEHICLE_API FTAPowerSupportStackState
{
    FTAElectricalBusState Electrical;
    FTAFuelSystemState Fuel;
    FTACoolantSystemState Coolant;
};

struct TA_VEHICLE_API FTAPowerSupportStackInput
{
    double EngineRPM = 0.0;

    bool bStarterEngaged = false;
    double StarterRequestedCurrentA = 0.0;

    double AlternatorHealth01 = 1.0;

    double EngineFuelDemandKgPerSec = 0.0;
    double FuelLeakAreaM2 = 0.0;

    double CoolantPumpHealth01 = 1.0;
    double RadiatorHealth01 = 1.0;
    double RamAirflowAuthority01 = 0.0;
    double CoolantLeakAreaM2 = 0.0;
    double CoolantSystemPressurePa = 150000.0;

    double AmbientPressurePa = 101325.0;

    double DeltaTimeSeconds = 1.0 / 240.0;
};

struct TA_VEHICLE_API FTAPowerSupportStackOutput
{
    FTAElectricalBusOutput Electrical;
    FTAFuelSystemOutput Fuel;
    FTACoolantSystemOutput Coolant;

    double EngineControlAuthority01 = 0.0;
    double StarterCurrentAuthority01 = 0.0;
    double FuelPumpAuthority01 = 0.0;
    double CoolingFanAuthority01 = 0.0;
    double EffectiveCoolingAirflowAuthority01 = 0.0;
};

namespace TAPowerSupportStack
{
    TA_VEHICLE_API bool ValidateConfig(
        const FTAPowerSupportStackConfig& Config);

    TA_VEHICLE_API void InitializeState(
        const FTAPowerSupportStackConfig& Config,
        FTAPowerSupportStackState& OutState);

    TA_VEHICLE_API bool Step(
        const FTAPowerSupportStackConfig& Config,
        const FTAPowerSupportStackInput& Input,
        FTAPowerSupportStackState& InOutState,
        FTAPowerSupportStackOutput& OutOutput);
}
