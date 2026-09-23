#include "TAVehicleDamageRouter.h"

namespace
{
    const FTAVehicleDamageRoute* FindRoute(
        const FTAVehicleDamageRoutingConfig& Config,
        const int32 TargetComponentIndex)
    {
        for (const FTAVehicleDamageRoute& Route :
             Config.Routes)
        {
            if (Route.TargetComponentIndex ==
                TargetComponentIndex)
            {
                return &Route;
            }
        }

        return nullptr;
    }

    bool RouteAcceptsSignal(
        const FTAVehicleDamageRoute& Route,
        const ETADamageSignalType Type)
    {
        switch (Type)
        {
        case ETADamageSignalType::ImpactEnergy:
            return Route.bAcceptImpactEnergy;

        case ETADamageSignalType::StructuralDisplacement:
            return Route.bAcceptStructuralDisplacement;

        case ETADamageSignalType::StructuralFracture:
            return Route.bAcceptStructuralFracture;

        default:
            return false;
        }
    }

    bool ApplyRadiatorSignal(
        const FTAVehicleDamageRoute& Route,
        const FTAVehicleRuntimeConfig& VehicleConfig,
        const FTADamageSignal& Signal,
        FTAVehicleRuntimeState& InOutVehicleState)
    {
        if (Signal.Type == ETADamageSignalType::ImpactEnergy)
        {
            const double ImpactEnergyJ =
                FMath::Max(
                    0.0,
                    Signal.ScalarValue)
                * FMath::Max(
                    0.0,
                    Route.ImpactEnergyScale);

            TADamage::ApplyRadiatorImpact(
                VehicleConfig.Radiator,
                ImpactEnergyJ,
                0.0,
                InOutVehicleState.Radiator);

            return true;
        }

        if (Signal.Type == ETADamageSignalType::StructuralDisplacement)
        {
            const double DisplacementM =
                Signal.VectorValue.IsNearlyZero()
                ? FMath::Max(
                    0.0,
                    Signal.ScalarValue)
                : Signal.VectorValue.Length();

            const double FullCrushDisplacementM =
                FMath::Max(
                    1.0e-6,
                    Route.FullCrushDisplacementM);

            const double CrushFraction01 =
                FMath::Clamp(
                    DisplacementM
                    / FullCrushDisplacementM,
                    0.0,
                    1.0);

            TADamage::ApplyRadiatorImpact(
                VehicleConfig.Radiator,
                0.0,
                CrushFraction01,
                InOutVehicleState.Radiator);

            return true;
        }

        if (Signal.Type == ETADamageSignalType::StructuralFracture)
        {
            // Prototype interpretation: a fracture specifically routed to the
            // radiator support is a complete airflow-support collapse.
            TADamage::ApplyRadiatorImpact(
                VehicleConfig.Radiator,
                0.0,
                1.0,
                InOutVehicleState.Radiator);

            return true;
        }

        return false;
    }
}

bool TAVehicleDamageRouter::ValidateConfig(
    const FTAVehicleDamageRoutingConfig& Config)
{
    for (int32 RouteIndex = 0;
         RouteIndex < Config.Routes.Num();
         ++RouteIndex)
    {
        const FTAVehicleDamageRoute& Route =
            Config.Routes[RouteIndex];

        if (Route.TargetComponentIndex == INDEX_NONE ||
            Route.Consumer == ETAVehicleDamageConsumerType::None ||
            !FMath::IsFinite(Route.ImpactEnergyScale) ||
            Route.ImpactEnergyScale < 0.0 ||
            !FMath::IsFinite(Route.FullCrushDisplacementM) ||
            Route.FullCrushDisplacementM <= 0.0)
        {
            return false;
        }

        for (int32 OtherIndex = RouteIndex + 1;
             OtherIndex < Config.Routes.Num();
             ++OtherIndex)
        {
            if (Config.Routes[OtherIndex].TargetComponentIndex ==
                Route.TargetComponentIndex)
            {
                return false;
            }
        }
    }

    return true;
}

bool TAVehicleDamageRouter::RouteSignals(
    const FTAVehicleDamageRoutingConfig& RoutingConfig,
    const FTAVehicleRuntimeConfig& VehicleConfig,
    const TConstArrayView<FTADamageSignal> Signals,
    FTAVehicleRuntimeState& InOutVehicleState,
    FTAVehicleDamageRoutingOutput& OutOutput)
{
    OutOutput =
        FTAVehicleDamageRoutingOutput{};

    if (!ValidateConfig(RoutingConfig))
    {
        return false;
    }

    for (const FTADamageSignal& Signal :
         Signals)
    {
        ++OutOutput.SignalsVisited;

        const FTAVehicleDamageRoute* Route =
            FindRoute(
                RoutingConfig,
                Signal.TargetComponentIndex);

        if (!Route ||
            !RouteAcceptsSignal(
                *Route,
                Signal.Type))
        {
            ++OutOutput.SignalsIgnored;
            continue;
        }

        bool bApplied = false;

        switch (Route->Consumer)
        {
        case ETAVehicleDamageConsumerType::Radiator:
            bApplied =
                ApplyRadiatorSignal(
                    *Route,
                    VehicleConfig,
                    Signal,
                    InOutVehicleState);

            if (bApplied)
            {
                ++OutOutput.RadiatorSignalsApplied;
            }
            break;

        case ETAVehicleDamageConsumerType::None:
        default:
            break;
        }

        if (bApplied)
        {
            ++OutOutput.SignalsRouted;
        }
        else
        {
            ++OutOutput.SignalsIgnored;
        }
    }

    return true;
}
