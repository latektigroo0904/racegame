#pragma once

#include "CoreMinimal.h"
#include "TADamageTypes.h"
#include "TAVehicleSimulation.h"

enum class ETAVehicleDamageConsumerType : uint8
{
    None,
    Radiator
};

struct TA_VEHICLE_API FTAVehicleDamageRoute
{
    int32 TargetComponentIndex = INDEX_NONE;

    ETAVehicleDamageConsumerType Consumer =
        ETAVehicleDamageConsumerType::None;

    bool bAcceptImpactEnergy = true;
    bool bAcceptStructuralDisplacement = true;
    bool bAcceptStructuralFracture = false;

    double ImpactEnergyScale = 1.0;

    // Used by geometry/displacement-driven consumers such as radiator crush.
    double FullCrushDisplacementM = 0.15;
};

struct TA_VEHICLE_API FTAVehicleDamageRoutingConfig
{
    TArray<FTAVehicleDamageRoute> Routes;
};

struct TA_VEHICLE_API FTAVehicleDamageRoutingOutput
{
    int32 SignalsVisited = 0;
    int32 SignalsRouted = 0;
    int32 SignalsIgnored = 0;

    int32 RadiatorSignalsApplied = 0;
};

namespace TAVehicleDamageRouter
{
    TA_VEHICLE_API bool ValidateConfig(
        const FTAVehicleDamageRoutingConfig& Config);

    TA_VEHICLE_API bool RouteSignals(
        const FTAVehicleDamageRoutingConfig& RoutingConfig,
        const FTAVehicleRuntimeConfig& VehicleConfig,
        TConstArrayView<FTADamageSignal> Signals,
        FTAVehicleRuntimeState& InOutVehicleState,
        FTAVehicleDamageRoutingOutput& OutOutput);
}
