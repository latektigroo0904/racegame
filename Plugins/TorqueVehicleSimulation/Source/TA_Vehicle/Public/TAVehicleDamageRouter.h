#pragma once

#include "CoreMinimal.h"
#include "TADamageTypes.h"
#include "TAVehicleSimulation.h"

enum class ETAVehicleDamageConsumerType : uint8
{
    None,
    Radiator,
    SteeringRack,
    WheelHub,
    SuspensionCorner,
    AntiRollLink,
    ElectricalBus,
    FuelDelivery
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

    // Generic severity scales for impact/displacement-driven functional damage.
    double FullDamageEnergyJ = 12000.0;
    double FullCrushDisplacementM = 0.15;

    // Required by WheelHub routes; ignored by other consumers.
    int32 WheelIndex = INDEX_NONE;

    // Steering-rack functional degradation.
    double MinimumSteeringAuthority01 = 0.20;
    double MaximumSteeringFreePlayM = 0.010;

    // Wheel-hub functional degradation.
    double MinimumBrakeEfficiency01 = 0.20;
    double MinimumDriveEfficiency01 = 0.0;
    double MaximumBearingDragTorqueNm = 80.0;

    // Force-producing suspension assembly degradation.
    double MinimumSpringEfficiency01 = 0.10;
    double MinimumDampingEfficiency01 = 0.0;
    double MinimumStopEfficiency01 = 1.0;

    // Anti-roll drop-link/bar-link degradation.
    double MinimumAntiRollLinkEfficiency01 = 0.0;

    // Electrical-bus functional degradation.
    double MinimumStarterEfficiency01 = 0.0;
    double MinimumEngineControlEfficiency01 = 0.0;

    // Fuel-delivery functional degradation.
    double MinimumFuelDeliveryEfficiency01 = 0.0;
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
    int32 SteeringRackSignalsApplied = 0;
    int32 WheelHubSignalsApplied = 0;
    int32 SuspensionCornerSignalsApplied = 0;
    int32 AntiRollLinkSignalsApplied = 0;
    int32 ElectricalBusSignalsApplied = 0;
    int32 FuelDeliverySignalsApplied = 0;
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
