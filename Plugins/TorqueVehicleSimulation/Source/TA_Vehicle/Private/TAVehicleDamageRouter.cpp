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

    double CalculateFunctionalDamageSeverity01(
        const FTAVehicleDamageRoute& Route,
        const FTADamageSignal& Signal)
    {
        if (Signal.Type == ETADamageSignalType::StructuralFracture)
        {
            return 1.0;
        }

        if (Signal.Type == ETADamageSignalType::ImpactEnergy)
        {
            const double ScaledEnergyJ =
                FMath::Max(
                    0.0,
                    Signal.ScalarValue)
                * FMath::Max(
                    0.0,
                    Route.ImpactEnergyScale);

            return FMath::Clamp(
                ScaledEnergyJ
                / FMath::Max(
                    1.0e-6,
                    Route.FullDamageEnergyJ),
                0.0,
                1.0);
        }

        if (Signal.Type == ETADamageSignalType::StructuralDisplacement)
        {
            const double DisplacementM =
                Signal.VectorValue.IsNearlyZero()
                ? FMath::Max(
                    0.0,
                    Signal.ScalarValue)
                : Signal.VectorValue.Length();

            return FMath::Clamp(
                DisplacementM
                / FMath::Max(
                    1.0e-6,
                    Route.FullCrushDisplacementM),
                0.0,
                1.0);
        }

        return 0.0;
    }

    bool ApplySteeringRackSignal(
        const FTAVehicleDamageRoute& Route,
        const FTADamageSignal& Signal,
        FTAVehicleRuntimeState& InOutVehicleState)
    {
        const double Severity01 =
            CalculateFunctionalDamageSeverity01(
                Route,
                Signal);

        if (Severity01 <= 0.0)
        {
            return false;
        }

        FTASteeringRackFunctionalDamageState& Damage =
            InOutVehicleState.SteeringRackDamage;

        Damage.Damage01 =
            FMath::Max(
                Damage.Damage01,
                Severity01);

        const double TargetAuthority01 =
            FMath::Lerp(
                1.0,
                FMath::Clamp(
                    Route.MinimumSteeringAuthority01,
                    0.0,
                    1.0),
                Severity01);

        Damage.CommandAuthority01 =
            FMath::Min(
                Damage.CommandAuthority01,
                TargetAuthority01);

        Damage.FreePlayM =
            FMath::Max(
                Damage.FreePlayM,
                FMath::Max(
                    0.0,
                    Route.MaximumSteeringFreePlayM)
                * Severity01);

        return true;
    }

    bool ApplyWheelHubSignal(
        const FTAVehicleDamageRoute& Route,
        const FTADamageSignal& Signal,
        FTAVehicleRuntimeState& InOutVehicleState)
    {
        if (!InOutVehicleState.WheelHubDamage.IsValidIndex(
                Route.WheelIndex))
        {
            return false;
        }

        const double Severity01 =
            CalculateFunctionalDamageSeverity01(
                Route,
                Signal);

        if (Severity01 <= 0.0)
        {
            return false;
        }

        FTAWheelHubFunctionalDamageState& Damage =
            InOutVehicleState.WheelHubDamage[
                Route.WheelIndex];

        Damage.Damage01 =
            FMath::Max(
                Damage.Damage01,
                Severity01);

        const double TargetBrakeEfficiency01 =
            FMath::Lerp(
                1.0,
                FMath::Clamp(
                    Route.MinimumBrakeEfficiency01,
                    0.0,
                    1.0),
                Severity01);

        const double TargetDriveEfficiency01 =
            FMath::Lerp(
                1.0,
                FMath::Clamp(
                    Route.MinimumDriveEfficiency01,
                    0.0,
                    1.0),
                Severity01);

        Damage.BrakeEfficiency01 =
            FMath::Min(
                Damage.BrakeEfficiency01,
                TargetBrakeEfficiency01);

        Damage.DriveEfficiency01 =
            FMath::Min(
                Damage.DriveEfficiency01,
                TargetDriveEfficiency01);

        Damage.BearingDragTorqueNm =
            FMath::Max(
                Damage.BearingDragTorqueNm,
                FMath::Max(
                    0.0,
                    Route.MaximumBearingDragTorqueNm)
                * Severity01);

        return true;
    }

    bool ApplySuspensionCornerSignal(
        const FTAVehicleDamageRoute& Route,
        const FTADamageSignal& Signal,
        FTAVehicleRuntimeState& InOutVehicleState)
    {
        if (!InOutVehicleState.SuspensionDamage.IsValidIndex(
                Route.WheelIndex))
        {
            return false;
        }

        const double Severity01 =
            CalculateFunctionalDamageSeverity01(
                Route,
                Signal);

        if (Severity01 <= 0.0)
        {
            return false;
        }

        FTASuspensionFunctionalDamageState& Damage =
            InOutVehicleState.SuspensionDamage[
                Route.WheelIndex];

        Damage.SpringDamperDamage01 =
            FMath::Max(
                Damage.SpringDamperDamage01,
                Severity01);

        const double TargetSpringEfficiency01 =
            FMath::Lerp(
                1.0,
                FMath::Clamp(
                    Route.MinimumSpringEfficiency01,
                    0.0,
                    1.0),
                Severity01);

        const double TargetDampingEfficiency01 =
            FMath::Lerp(
                1.0,
                FMath::Clamp(
                    Route.MinimumDampingEfficiency01,
                    0.0,
                    1.0),
                Severity01);

        const double TargetStopEfficiency01 =
            FMath::Lerp(
                1.0,
                FMath::Clamp(
                    Route.MinimumStopEfficiency01,
                    0.0,
                    1.0),
                Severity01);

        Damage.SpringEfficiency01 =
            FMath::Min(
                Damage.SpringEfficiency01,
                TargetSpringEfficiency01);

        Damage.DampingEfficiency01 =
            FMath::Min(
                Damage.DampingEfficiency01,
                TargetDampingEfficiency01);

        Damage.StopEfficiency01 =
            FMath::Min(
                Damage.StopEfficiency01,
                TargetStopEfficiency01);

        return true;
    }

    bool ApplyAntiRollLinkSignal(
        const FTAVehicleDamageRoute& Route,
        const FTADamageSignal& Signal,
        FTAVehicleRuntimeState& InOutVehicleState)
    {
        if (!InOutVehicleState.SuspensionDamage.IsValidIndex(
                Route.WheelIndex))
        {
            return false;
        }

        const double Severity01 =
            CalculateFunctionalDamageSeverity01(
                Route,
                Signal);

        if (Severity01 <= 0.0)
        {
            return false;
        }

        FTASuspensionFunctionalDamageState& Damage =
            InOutVehicleState.SuspensionDamage[
                Route.WheelIndex];

        Damage.AntiRollLinkDamage01 =
            FMath::Max(
                Damage.AntiRollLinkDamage01,
                Severity01);

        const double TargetEfficiency01 =
            FMath::Lerp(
                1.0,
                FMath::Clamp(
                    Route.MinimumAntiRollLinkEfficiency01,
                    0.0,
                    1.0),
                Severity01);

        Damage.AntiRollLinkEfficiency01 =
            FMath::Min(
                Damage.AntiRollLinkEfficiency01,
                TargetEfficiency01);

        return true;
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
            !FMath::IsFinite(Route.FullDamageEnergyJ) ||
            Route.FullDamageEnergyJ <= 0.0 ||
            !FMath::IsFinite(Route.FullCrushDisplacementM) ||
            Route.FullCrushDisplacementM <= 0.0 ||
            !FMath::IsFinite(Route.MinimumSteeringAuthority01) ||
            Route.MinimumSteeringAuthority01 < 0.0 ||
            Route.MinimumSteeringAuthority01 > 1.0 ||
            !FMath::IsFinite(Route.MaximumSteeringFreePlayM) ||
            Route.MaximumSteeringFreePlayM < 0.0 ||
            !FMath::IsFinite(Route.MinimumBrakeEfficiency01) ||
            Route.MinimumBrakeEfficiency01 < 0.0 ||
            Route.MinimumBrakeEfficiency01 > 1.0 ||
            !FMath::IsFinite(Route.MinimumDriveEfficiency01) ||
            Route.MinimumDriveEfficiency01 < 0.0 ||
            Route.MinimumDriveEfficiency01 > 1.0 ||
            !FMath::IsFinite(Route.MaximumBearingDragTorqueNm) ||
            Route.MaximumBearingDragTorqueNm < 0.0 ||
            !FMath::IsFinite(Route.MinimumSpringEfficiency01) ||
            Route.MinimumSpringEfficiency01 < 0.0 ||
            Route.MinimumSpringEfficiency01 > 1.0 ||
            !FMath::IsFinite(Route.MinimumDampingEfficiency01) ||
            Route.MinimumDampingEfficiency01 < 0.0 ||
            Route.MinimumDampingEfficiency01 > 1.0 ||
            !FMath::IsFinite(Route.MinimumStopEfficiency01) ||
            Route.MinimumStopEfficiency01 < 0.0 ||
            Route.MinimumStopEfficiency01 > 1.0 ||
            !FMath::IsFinite(Route.MinimumAntiRollLinkEfficiency01) ||
            Route.MinimumAntiRollLinkEfficiency01 < 0.0 ||
            Route.MinimumAntiRollLinkEfficiency01 > 1.0 ||
            ((Route.Consumer == ETAVehicleDamageConsumerType::WheelHub ||
              Route.Consumer == ETAVehicleDamageConsumerType::SuspensionCorner ||
              Route.Consumer == ETAVehicleDamageConsumerType::AntiRollLink) &&
             Route.WheelIndex < 0))
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

        case ETAVehicleDamageConsumerType::SteeringRack:
            bApplied =
                ApplySteeringRackSignal(
                    *Route,
                    Signal,
                    InOutVehicleState);

            if (bApplied)
            {
                ++OutOutput.SteeringRackSignalsApplied;
            }
            break;

        case ETAVehicleDamageConsumerType::WheelHub:
            bApplied =
                ApplyWheelHubSignal(
                    *Route,
                    Signal,
                    InOutVehicleState);

            if (bApplied)
            {
                ++OutOutput.WheelHubSignalsApplied;
            }
            break;

        case ETAVehicleDamageConsumerType::SuspensionCorner:
            bApplied =
                ApplySuspensionCornerSignal(
                    *Route,
                    Signal,
                    InOutVehicleState);

            if (bApplied)
            {
                ++OutOutput.SuspensionCornerSignalsApplied;
            }
            break;

        case ETAVehicleDamageConsumerType::AntiRollLink:
            bApplied =
                ApplyAntiRollLinkSignal(
                    *Route,
                    Signal,
                    InOutVehicleState);

            if (bApplied)
            {
                ++OutOutput.AntiRollLinkSignalsApplied;
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
