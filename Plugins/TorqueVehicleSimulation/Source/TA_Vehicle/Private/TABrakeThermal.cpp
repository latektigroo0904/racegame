#include "TABrakeThermal.h"

bool TABrakeThermal::ValidateConfig(
    const FTABrakeThermalConfig& Config)
{
    return
        FMath::IsFinite(Config.ThermalMassJPerC)
        && Config.ThermalMassJPerC > UE_DOUBLE_SMALL_NUMBER
        && FMath::IsFinite(Config.CoolingWPerC)
        && Config.CoolingWPerC >= 0.0
        && FMath::IsFinite(Config.AmbientTemperatureC)
        && Config.AmbientTemperatureC > -273.15
        && FMath::IsFinite(Config.HeatFraction01)
        && Config.HeatFraction01 >= 0.0
        && Config.HeatFraction01 <= 1.0
        && FMath::IsFinite(Config.FadeStartTemperatureC)
        && FMath::IsFinite(Config.FadeEndTemperatureC)
        && Config.FadeEndTemperatureC
            > Config.FadeStartTemperatureC
        && FMath::IsFinite(Config.MinimumFadeTorqueFactor01)
        && Config.MinimumFadeTorqueFactor01 >= 0.0
        && Config.MinimumFadeTorqueFactor01 <= 1.0
        && FMath::IsFinite(Config.WearEnergyCapacityJ)
        && Config.WearEnergyCapacityJ > UE_DOUBLE_SMALL_NUMBER
        && FMath::IsFinite(Config.WearTorqueLossAtEnd01)
        && Config.WearTorqueLossAtEnd01 >= 0.0
        && Config.WearTorqueLossAtEnd01 <= 1.0;
}

void TABrakeThermal::InitializeState(
    const FTABrakeThermalConfig& Config,
    FTABrakeThermalState& OutState)
{
    OutState =
        FTABrakeThermalState{};

    OutState.TemperatureC =
        Config.AmbientTemperatureC;

    OutState.ThermalTorqueFactor01 =
        CalculateThermalTorqueFactor01(
            Config,
            OutState.TemperatureC);

    OutState.WearTorqueFactor01 =
        CalculateWearTorqueFactor01(
            Config,
            OutState.Wear01);
}

double TABrakeThermal::CalculateThermalTorqueFactor01(
    const FTABrakeThermalConfig& Config,
    const double TemperatureC)
{
    if (!ValidateConfig(Config)
        || !FMath::IsFinite(TemperatureC))
    {
        return 0.0;
    }

    if (TemperatureC
        <= Config.FadeStartTemperatureC)
    {
        return 1.0;
    }

    if (TemperatureC
        >= Config.FadeEndTemperatureC)
    {
        return Config.MinimumFadeTorqueFactor01;
    }

    const double FadeAlpha01 =
        (TemperatureC
            - Config.FadeStartTemperatureC)
        / (Config.FadeEndTemperatureC
            - Config.FadeStartTemperatureC);

    return FMath::Lerp(
        1.0,
        Config.MinimumFadeTorqueFactor01,
        FMath::Clamp(
            FadeAlpha01,
            0.0,
            1.0));
}

double TABrakeThermal::CalculateWearTorqueFactor01(
    const FTABrakeThermalConfig& Config,
    const double Wear01)
{
    if (!ValidateConfig(Config)
        || !FMath::IsFinite(Wear01))
    {
        return 0.0;
    }

    return
        1.0
        - FMath::Clamp(
            Config.WearTorqueLossAtEnd01,
            0.0,
            1.0)
        * FMath::Clamp(
            Wear01,
            0.0,
            1.0);
}

double TABrakeThermal::CalculateAvailableTorqueFactor01(
    const FTABrakeThermalConfig& Config,
    const FTABrakeThermalState& State)
{
    return FMath::Clamp(
        CalculateThermalTorqueFactor01(
            Config,
            State.TemperatureC)
        * CalculateWearTorqueFactor01(
            Config,
            State.Wear01),
        0.0,
        1.0);
}

bool TABrakeThermal::Update(
    const FTABrakeThermalConfig& Config,
    const double AppliedBrakeTorqueNm,
    const double AverageAbsAngularSpeedRadPerSec,
    const double DeltaTimeSeconds,
    FTABrakeThermalState& InOutState,
    FTABrakeThermalOutput& OutOutput)
{
    OutOutput =
        FTABrakeThermalOutput{};

    if (!ValidateConfig(Config)
        || !FMath::IsFinite(AppliedBrakeTorqueNm)
        || AppliedBrakeTorqueNm < 0.0
        || !FMath::IsFinite(AverageAbsAngularSpeedRadPerSec)
        || AverageAbsAngularSpeedRadPerSec < 0.0
        || !FMath::IsFinite(DeltaTimeSeconds)
        || DeltaTimeSeconds <= 0.0
        || !FMath::IsFinite(InOutState.TemperatureC)
        || !FMath::IsFinite(InOutState.Wear01))
    {
        return false;
    }

    OutOutput.FrictionPowerW =
        AppliedBrakeTorqueNm
        * AverageAbsAngularSpeedRadPerSec;

    OutOutput.HeatPowerW =
        OutOutput.FrictionPowerW
        * Config.HeatFraction01;

    const double TemperatureAboveAmbientC =
        FMath::Max(
            0.0,
            InOutState.TemperatureC
                - Config.AmbientTemperatureC);

    OutOutput.CoolingPowerW =
        Config.CoolingWPerC
        * TemperatureAboveAmbientC;

    OutOutput.GeneratedHeatEnergyJ =
        OutOutput.HeatPowerW
        * DeltaTimeSeconds;

    const double NetHeatEnergyJ =
        (OutOutput.HeatPowerW
            - OutOutput.CoolingPowerW)
        * DeltaTimeSeconds;

    InOutState.TemperatureC =
        FMath::Max(
            Config.AmbientTemperatureC,
            InOutState.TemperatureC
                + NetHeatEnergyJ
                    / Config.ThermalMassJPerC);

    InOutState.Wear01 =
        FMath::Clamp(
            InOutState.Wear01
                + OutOutput.GeneratedHeatEnergyJ
                    / Config.WearEnergyCapacityJ,
            0.0,
            1.0);

    InOutState.ThermalTorqueFactor01 =
        CalculateThermalTorqueFactor01(
            Config,
            InOutState.TemperatureC);

    InOutState.WearTorqueFactor01 =
        CalculateWearTorqueFactor01(
            Config,
            InOutState.Wear01);

    OutOutput.ThermalTorqueFactor01 =
        InOutState.ThermalTorqueFactor01;

    OutOutput.WearTorqueFactor01 =
        InOutState.WearTorqueFactor01;

    OutOutput.AvailableTorqueFactor01 =
        FMath::Clamp(
            InOutState.ThermalTorqueFactor01
            * InOutState.WearTorqueFactor01,
            0.0,
            1.0);

    return true;
}
