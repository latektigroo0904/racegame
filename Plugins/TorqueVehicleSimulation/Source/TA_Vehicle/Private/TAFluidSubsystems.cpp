#include "TAFluidSubsystems.h"

namespace
{
    double MoveToward(
        const double Current,
        const double Target,
        const double RiseRatePerSec,
        const double DecayRatePerSec,
        const double DeltaTimeSeconds)
    {
        const double Difference =
            Target - Current;

        const double Rate =
            Difference >= 0.0
            ? RiseRatePerSec
            : DecayRatePerSec;

        const double Delta =
            FMath::Min(
                FMath::Abs(Difference),
                Rate * DeltaTimeSeconds);

        return Current
            + FMath::Sign(Difference)
                * Delta;
    }

    double MassFraction01(
        const double MassKg,
        const double NominalMassKg)
    {
        return FMath::Clamp(
            MassKg
            / FMath::Max(
                NominalMassKg,
                UE_DOUBLE_SMALL_NUMBER),
            0.0,
            1.0);
    }

    bool FiniteNonNegative(
        const double Value)
    {
        return FMath::IsFinite(Value)
            && Value >= 0.0;
    }
}

bool TAFluidSubsystems::ValidateFuelConfig(
    const FTAFuelSystemConfig& Config)
{
    return
        FiniteNonNegative(Config.NominalFuelMassKg)
        && Config.NominalFuelMassKg > UE_DOUBLE_SMALL_NUMBER
        && FiniteNonNegative(Config.MaxRailPressurePa)
        && Config.MaxRailPressurePa > 0.0
        && FiniteNonNegative(Config.MinimumCombustionPressurePa)
        && Config.MinimumCombustionPressurePa > 0.0
        && Config.MinimumCombustionPressurePa
            <= Config.MaxRailPressurePa
        && FiniteNonNegative(Config.PressureRiseRatePaPerSec)
        && Config.PressureRiseRatePaPerSec > 0.0
        && FiniteNonNegative(Config.PressureDecayRatePaPerSec)
        && Config.PressureDecayRatePaPerSec > 0.0
        && TAFluidPrimitives::ValidateLeakConfig(Config.Leak);
}

void TAFluidSubsystems::InitializeFuelState(
    const FTAFuelSystemConfig& Config,
    FTAFuelSystemState& OutState)
{
    OutState =
        FTAFuelSystemState{};

    OutState.Reservoir.MassKg =
        Config.NominalFuelMassKg;
}

bool TAFluidSubsystems::StepFuel(
    const FTAFuelSystemConfig& Config,
    const FTAFuelSystemInput& Input,
    FTAFuelSystemState& InOutState,
    FTAFuelSystemOutput& OutOutput)
{
    OutOutput =
        FTAFuelSystemOutput{};

    if (!ValidateFuelConfig(Config)
        || !FMath::IsFinite(Input.PumpAuthority01)
        || !FiniteNonNegative(Input.EngineFuelDemandKgPerSec)
        || !FiniteNonNegative(Input.LeakAreaM2)
        || !FiniteNonNegative(Input.TankPressurePa)
        || !FiniteNonNegative(Input.AmbientPressurePa)
        || !FMath::IsFinite(Input.DeltaTimeSeconds)
        || Input.DeltaTimeSeconds <= 0.0
        || !FiniteNonNegative(InOutState.Reservoir.MassKg)
        || !FiniteNonNegative(InOutState.RailPressurePa))
    {
        return false;
    }

    if (!TAFluidPrimitives::IntegrateReservoirLeak(
            Config.Leak,
            Input.LeakAreaM2,
            Input.TankPressurePa,
            Input.AmbientPressurePa,
            Input.DeltaTimeSeconds,
            InOutState.Reservoir,
            OutOutput.Leak))
    {
        return false;
    }

    const double RequestedConsumedMassKg =
        Input.EngineFuelDemandKgPerSec
        * Input.DeltaTimeSeconds;

    OutOutput.ConsumedMassKg =
        FMath::Min(
            InOutState.Reservoir.MassKg,
            RequestedConsumedMassKg);

    InOutState.Reservoir.MassKg -=
        OutOutput.ConsumedMassKg;

    OutOutput.FuelMassFraction01 =
        MassFraction01(
            InOutState.Reservoir.MassKg,
            Config.NominalFuelMassKg);

    const double PumpAuthority01 =
        FMath::Clamp(
            Input.PumpAuthority01,
            0.0,
            1.0)
        * OutOutput.FuelMassFraction01;

    const double TargetPressurePa =
        Config.MaxRailPressurePa
        * PumpAuthority01;

    InOutState.RailPressurePa =
        FMath::Max(
            0.0,
            MoveToward(
                InOutState.RailPressurePa,
                TargetPressurePa,
                Config.PressureRiseRatePaPerSec,
                Config.PressureDecayRatePaPerSec,
                Input.DeltaTimeSeconds));

    OutOutput.FuelMassKg =
        InOutState.Reservoir.MassKg;

    OutOutput.RailPressurePa =
        InOutState.RailPressurePa;

    OutOutput.DeliveryAuthority01 =
        FMath::Clamp(
            InOutState.RailPressurePa
            / Config.MinimumCombustionPressurePa,
            0.0,
            1.0)
        * OutOutput.FuelMassFraction01;

    return true;
}

bool TAFluidSubsystems::ValidateOilConfig(
    const FTAOilSystemConfig& Config)
{
    return
        FiniteNonNegative(Config.NominalOilMassKg)
        && Config.NominalOilMassKg > UE_DOUBLE_SMALL_NUMBER
        && FiniteNonNegative(Config.IdlePressurePa)
        && FiniteNonNegative(Config.MaxPressurePa)
        && Config.MaxPressurePa >= Config.IdlePressurePa
        && FiniteNonNegative(Config.FullPressureRPM)
        && Config.FullPressureRPM > UE_DOUBLE_SMALL_NUMBER
        && FiniteNonNegative(Config.MinimumSafePressurePa)
        && Config.MinimumSafePressurePa > UE_DOUBLE_SMALL_NUMBER
        && FiniteNonNegative(Config.PressureRiseRatePaPerSec)
        && Config.PressureRiseRatePaPerSec > 0.0
        && FiniteNonNegative(Config.PressureDecayRatePaPerSec)
        && Config.PressureDecayRatePaPerSec > 0.0
        && TAFluidPrimitives::ValidateLeakConfig(Config.Leak);
}

void TAFluidSubsystems::InitializeOilState(
    const FTAOilSystemConfig& Config,
    FTAOilSystemState& OutState)
{
    OutState =
        FTAOilSystemState{};

    OutState.Reservoir.MassKg =
        Config.NominalOilMassKg;
}

bool TAFluidSubsystems::StepOil(
    const FTAOilSystemConfig& Config,
    const FTAOilSystemInput& Input,
    FTAOilSystemState& InOutState,
    FTAOilSystemOutput& OutOutput)
{
    OutOutput =
        FTAOilSystemOutput{};

    if (!ValidateOilConfig(Config)
        || !FiniteNonNegative(Input.EngineRPM)
        || !FMath::IsFinite(Input.PumpHealth01)
        || !FiniteNonNegative(Input.LeakAreaM2)
        || !FiniteNonNegative(Input.GalleryPressureForLeakPa)
        || !FiniteNonNegative(Input.AmbientPressurePa)
        || !FMath::IsFinite(Input.DeltaTimeSeconds)
        || Input.DeltaTimeSeconds <= 0.0
        || !FiniteNonNegative(InOutState.Reservoir.MassKg)
        || !FiniteNonNegative(InOutState.GalleryPressurePa))
    {
        return false;
    }

    if (!TAFluidPrimitives::IntegrateReservoirLeak(
            Config.Leak,
            Input.LeakAreaM2,
            Input.GalleryPressureForLeakPa,
            Input.AmbientPressurePa,
            Input.DeltaTimeSeconds,
            InOutState.Reservoir,
            OutOutput.Leak))
    {
        return false;
    }

    OutOutput.OilMassFraction01 =
        MassFraction01(
            InOutState.Reservoir.MassKg,
            Config.NominalOilMassKg);

    const double SpeedAuthority01 =
        FMath::Clamp(
            Input.EngineRPM
            / Config.FullPressureRPM,
            0.0,
            1.0);

    const double HealthyPressureTargetPa =
        FMath::Lerp(
            Config.IdlePressurePa,
            Config.MaxPressurePa,
            SpeedAuthority01);

    const double TargetPressurePa =
        HealthyPressureTargetPa
        * FMath::Clamp(Input.PumpHealth01, 0.0, 1.0)
        * OutOutput.OilMassFraction01;

    InOutState.GalleryPressurePa =
        FMath::Max(
            0.0,
            MoveToward(
                InOutState.GalleryPressurePa,
                TargetPressurePa,
                Config.PressureRiseRatePaPerSec,
                Config.PressureDecayRatePaPerSec,
                Input.DeltaTimeSeconds));

    OutOutput.OilMassKg =
        InOutState.Reservoir.MassKg;

    OutOutput.GalleryPressurePa =
        InOutState.GalleryPressurePa;

    OutOutput.LubricationAuthority01 =
        FMath::Clamp(
            InOutState.GalleryPressurePa
            / Config.MinimumSafePressurePa,
            0.0,
            1.0)
        * OutOutput.OilMassFraction01;

    return true;
}

bool TAFluidSubsystems::ValidateCoolantConfig(
    const FTACoolantSystemConfig& Config)
{
    return
        FiniteNonNegative(Config.NominalCoolantMassKg)
        && Config.NominalCoolantMassKg > UE_DOUBLE_SMALL_NUMBER
        && FiniteNonNegative(Config.PumpFullFlowRPM)
        && Config.PumpFullFlowRPM > UE_DOUBLE_SMALL_NUMBER
        && FMath::IsFinite(
            Config.MinimumCirculationMassFraction01)
        && Config.MinimumCirculationMassFraction01 >= 0.0
        && Config.MinimumCirculationMassFraction01 <= 1.0
        && TAFluidPrimitives::ValidateLeakConfig(Config.Leak);
}

void TAFluidSubsystems::InitializeCoolantState(
    const FTACoolantSystemConfig& Config,
    FTACoolantSystemState& OutState)
{
    OutState =
        FTACoolantSystemState{};

    OutState.Reservoir.MassKg =
        Config.NominalCoolantMassKg;
}

bool TAFluidSubsystems::StepCoolant(
    const FTACoolantSystemConfig& Config,
    const FTACoolantSystemInput& Input,
    FTACoolantSystemState& InOutState,
    FTACoolantSystemOutput& OutOutput)
{
    OutOutput =
        FTACoolantSystemOutput{};

    if (!ValidateCoolantConfig(Config)
        || !FiniteNonNegative(Input.EngineRPM)
        || !FMath::IsFinite(Input.PumpHealth01)
        || !FMath::IsFinite(Input.RadiatorHealth01)
        || !FMath::IsFinite(Input.AirflowAuthority01)
        || !FiniteNonNegative(Input.LeakAreaM2)
        || !FiniteNonNegative(Input.SystemPressurePa)
        || !FiniteNonNegative(Input.AmbientPressurePa)
        || !FMath::IsFinite(Input.DeltaTimeSeconds)
        || Input.DeltaTimeSeconds <= 0.0
        || !FiniteNonNegative(InOutState.Reservoir.MassKg))
    {
        return false;
    }

    if (!TAFluidPrimitives::IntegrateReservoirLeak(
            Config.Leak,
            Input.LeakAreaM2,
            Input.SystemPressurePa,
            Input.AmbientPressurePa,
            Input.DeltaTimeSeconds,
            InOutState.Reservoir,
            OutOutput.Leak))
    {
        return false;
    }

    OutOutput.CoolantMassKg =
        InOutState.Reservoir.MassKg;

    OutOutput.CoolantMassFraction01 =
        MassFraction01(
            InOutState.Reservoir.MassKg,
            Config.NominalCoolantMassKg);

    const double MassCirculationAuthority01 =
        FMath::Clamp(
            OutOutput.CoolantMassFraction01
            / FMath::Max(
                Config.MinimumCirculationMassFraction01,
                UE_DOUBLE_SMALL_NUMBER),
            0.0,
            1.0);

    const double PumpSpeedAuthority01 =
        FMath::Clamp(
            Input.EngineRPM
            / Config.PumpFullFlowRPM,
            0.0,
            1.0);

    OutOutput.CirculationAuthority01 =
        PumpSpeedAuthority01
        * FMath::Clamp(Input.PumpHealth01, 0.0, 1.0)
        * MassCirculationAuthority01;

    OutOutput.RadiatorHeatRejectionAuthority01 =
        FMath::Clamp(Input.RadiatorHealth01, 0.0, 1.0)
        * FMath::Clamp(Input.AirflowAuthority01, 0.0, 1.0);

    OutOutput.CoolingAuthority01 =
        OutOutput.CirculationAuthority01
        * OutOutput.RadiatorHeatRejectionAuthority01;

    return true;
}
