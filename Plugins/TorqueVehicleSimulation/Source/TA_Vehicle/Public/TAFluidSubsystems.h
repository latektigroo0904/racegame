#pragma once

#include "CoreMinimal.h"
#include "TAFluidPrimitives.h"

struct TA_VEHICLE_API FTAFuelSystemConfig
{
    double NominalFuelMassKg = 45.0;

    double MaxRailPressurePa = 500000.0;
    double MinimumCombustionPressurePa = 250000.0;
    double PressureRiseRatePaPerSec = 2.0e6;
    double PressureDecayRatePaPerSec = 3.0e6;

    FTAFluidLeakConfig Leak;
};

struct TA_VEHICLE_API FTAFuelSystemState
{
    FTAFluidReservoirState Reservoir;
    double RailPressurePa = 0.0;
};

struct TA_VEHICLE_API FTAFuelSystemInput
{
    // Electrical/fuel-pump authority [0,1].
    double PumpAuthority01 = 1.0;

    // Engine-consumed fuel mass flow.
    double EngineFuelDemandKgPerSec = 0.0;

    double LeakAreaM2 = 0.0;
    double TankPressurePa = 101325.0;
    double AmbientPressurePa = 101325.0;

    double DeltaTimeSeconds = 1.0 / 240.0;
};

struct TA_VEHICLE_API FTAFuelSystemOutput
{
    double FuelMassKg = 0.0;
    double FuelMassFraction01 = 0.0;

    double RailPressurePa = 0.0;
    double DeliveryAuthority01 = 0.0;

    double ConsumedMassKg = 0.0;
    FTAFluidLeakOutput Leak;
};

struct TA_VEHICLE_API FTAOilSystemConfig
{
    double NominalOilMassKg = 4.5;

    double IdlePressurePa = 150000.0;
    double MaxPressurePa = 500000.0;
    double FullPressureRPM = 3000.0;
    double MinimumSafePressurePa = 120000.0;

    double PressureRiseRatePaPerSec = 2.0e6;
    double PressureDecayRatePaPerSec = 3.0e6;

    FTAFluidLeakConfig Leak;
};

struct TA_VEHICLE_API FTAOilSystemState
{
    FTAFluidReservoirState Reservoir;
    double GalleryPressurePa = 0.0;
};

struct TA_VEHICLE_API FTAOilSystemInput
{
    double EngineRPM = 0.0;
    double PumpHealth01 = 1.0;

    double LeakAreaM2 = 0.0;
    double GalleryPressureForLeakPa = 0.0;
    double AmbientPressurePa = 101325.0;

    double DeltaTimeSeconds = 1.0 / 240.0;
};

struct TA_VEHICLE_API FTAOilSystemOutput
{
    double OilMassKg = 0.0;
    double OilMassFraction01 = 0.0;

    double GalleryPressurePa = 0.0;
    double LubricationAuthority01 = 0.0;

    FTAFluidLeakOutput Leak;
};

struct TA_VEHICLE_API FTACoolantSystemConfig
{
    double NominalCoolantMassKg = 7.0;

    double PumpFullFlowRPM = 2500.0;
    double MinimumCirculationMassFraction01 = 0.15;

    FTAFluidLeakConfig Leak;
};

struct TA_VEHICLE_API FTACoolantSystemState
{
    FTAFluidReservoirState Reservoir;
};

struct TA_VEHICLE_API FTACoolantSystemInput
{
    double EngineRPM = 0.0;
    double PumpHealth01 = 1.0;
    double RadiatorHealth01 = 1.0;
    double AirflowAuthority01 = 1.0;

    double LeakAreaM2 = 0.0;
    double SystemPressurePa = 150000.0;
    double AmbientPressurePa = 101325.0;

    double DeltaTimeSeconds = 1.0 / 60.0;
};

struct TA_VEHICLE_API FTACoolantSystemOutput
{
    double CoolantMassKg = 0.0;
    double CoolantMassFraction01 = 0.0;

    double CirculationAuthority01 = 0.0;
    double RadiatorHeatRejectionAuthority01 = 0.0;
    double CoolingAuthority01 = 0.0;

    FTAFluidLeakOutput Leak;
};

namespace TAFluidSubsystems
{
    TA_VEHICLE_API bool ValidateFuelConfig(
        const FTAFuelSystemConfig& Config);

    TA_VEHICLE_API void InitializeFuelState(
        const FTAFuelSystemConfig& Config,
        FTAFuelSystemState& OutState);

    TA_VEHICLE_API bool StepFuel(
        const FTAFuelSystemConfig& Config,
        const FTAFuelSystemInput& Input,
        FTAFuelSystemState& InOutState,
        FTAFuelSystemOutput& OutOutput);

    TA_VEHICLE_API bool ValidateOilConfig(
        const FTAOilSystemConfig& Config);

    TA_VEHICLE_API void InitializeOilState(
        const FTAOilSystemConfig& Config,
        FTAOilSystemState& OutState);

    TA_VEHICLE_API bool StepOil(
        const FTAOilSystemConfig& Config,
        const FTAOilSystemInput& Input,
        FTAOilSystemState& InOutState,
        FTAOilSystemOutput& OutOutput);

    TA_VEHICLE_API bool ValidateCoolantConfig(
        const FTACoolantSystemConfig& Config);

    TA_VEHICLE_API void InitializeCoolantState(
        const FTACoolantSystemConfig& Config,
        FTACoolantSystemState& OutState);

    TA_VEHICLE_API bool StepCoolant(
        const FTACoolantSystemConfig& Config,
        const FTACoolantSystemInput& Input,
        FTACoolantSystemState& InOutState,
        FTACoolantSystemOutput& OutOutput);
}
