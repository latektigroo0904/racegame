#include "TABrakeHydraulics.h"

namespace
{
    double MoveToward(
        const double Current,
        const double Target,
        const double RiseRatePerSec,
        const double ReleaseRatePerSec,
        const double DeltaTimeSeconds)
    {
        const double Difference =
            Target - Current;

        if (Difference >= 0.0)
        {
            return Current
                + FMath::Min(
                    Difference,
                    RiseRatePerSec * DeltaTimeSeconds);
        }

        return Current
            - FMath::Min(
                -Difference,
                ReleaseRatePerSec * DeltaTimeSeconds);
    }
}

bool TABrakeHydraulics::ValidateConfig(
    const FTABrakeHydraulicConfig& Config)
{
    return
        FMath::IsFinite(Config.PedalForceMaxN)
        && Config.PedalForceMaxN >= 0.0
        && FMath::IsFinite(Config.BoosterGain)
        && Config.BoosterGain >= 0.0
        && FMath::IsFinite(Config.MasterCylinderAreaM2)
        && Config.MasterCylinderAreaM2 > UE_DOUBLE_SMALL_NUMBER
        && FMath::IsFinite(Config.MaxSystemPressurePa)
        && Config.MaxSystemPressurePa > 0.0
        && FMath::IsFinite(Config.FrontPressureRatio01)
        && Config.FrontPressureRatio01 >= 0.0
        && Config.FrontPressureRatio01 <= 1.0
        && FMath::IsFinite(Config.RearPressureRatio01)
        && Config.RearPressureRatio01 >= 0.0
        && Config.RearPressureRatio01 <= 1.0
        && FMath::IsFinite(Config.PressureRiseRatePaPerSec)
        && Config.PressureRiseRatePaPerSec > 0.0
        && FMath::IsFinite(Config.PressureReleaseRatePaPerSec)
        && Config.PressureReleaseRatePaPerSec > 0.0
        && FMath::IsFinite(Config.FrontCaliperPistonAreaM2)
        && Config.FrontCaliperPistonAreaM2 >= 0.0
        && FMath::IsFinite(Config.RearCaliperPistonAreaM2)
        && Config.RearCaliperPistonAreaM2 >= 0.0
        && FMath::IsFinite(Config.FrontClampGeometryFactor)
        && Config.FrontClampGeometryFactor >= 0.0
        && FMath::IsFinite(Config.RearClampGeometryFactor)
        && Config.RearClampGeometryFactor >= 0.0
        && FMath::IsFinite(Config.FrontPadFrictionCoefficient)
        && Config.FrontPadFrictionCoefficient >= 0.0
        && FMath::IsFinite(Config.RearPadFrictionCoefficient)
        && Config.RearPadFrictionCoefficient >= 0.0
        && FMath::IsFinite(Config.FrontEffectiveDiscRadiusM)
        && Config.FrontEffectiveDiscRadiusM >= 0.0
        && FMath::IsFinite(Config.RearEffectiveDiscRadiusM)
        && Config.RearEffectiveDiscRadiusM >= 0.0
        && FMath::IsFinite(Config.FluidDryBoilingPointC)
        && FMath::IsFinite(Config.FluidWetBoilingPointC)
        && Config.FluidDryBoilingPointC
            > Config.FluidWetBoilingPointC
        && FMath::IsFinite(Config.FluidWaterContamination01)
        && Config.FluidWaterContamination01 >= 0.0
        && Config.FluidWaterContamination01 <= 1.0
        && FMath::IsFinite(Config.VaporTransitionRangeC)
        && Config.VaporTransitionRangeC > UE_DOUBLE_SMALL_NUMBER
        && FMath::IsFinite(Config.MaxPressureLossAtFullVapor01)
        && Config.MaxPressureLossAtFullVapor01 >= 0.0
        && Config.MaxPressureLossAtFullVapor01 <= 1.0
        && FMath::IsFinite(Config.FluidThermalMassJPerC)
        && Config.FluidThermalMassJPerC > UE_DOUBLE_SMALL_NUMBER
        && FMath::IsFinite(Config.FluidCoolingWPerC)
        && Config.FluidCoolingWPerC >= 0.0
        && FMath::IsFinite(Config.FluidAmbientTemperatureC)
        && Config.FluidAmbientTemperatureC > -273.15;
}

void TABrakeHydraulics::InitializeState(
    const FTABrakeHydraulicConfig& Config,
    FTABrakeHydraulicState& OutState)
{
    OutState =
        FTABrakeHydraulicState{};

    OutState.FluidTemperatureC =
        Config.FluidAmbientTemperatureC;

    OutState.VaporFraction01 =
        CalculateVaporFraction01(
            Config,
            OutState.FluidTemperatureC);
}

double TABrakeHydraulics::CalculateBoilingPointC(
    const FTABrakeHydraulicConfig& Config)
{
    return FMath::Lerp(
        Config.FluidDryBoilingPointC,
        Config.FluidWetBoilingPointC,
        FMath::Clamp(
            Config.FluidWaterContamination01,
            0.0,
            1.0));
}

double TABrakeHydraulics::CalculateVaporFraction01(
    const FTABrakeHydraulicConfig& Config,
    const double FluidTemperatureC)
{
    if (!ValidateConfig(Config)
        || !FMath::IsFinite(FluidTemperatureC))
    {
        return 0.0;
    }

    const double BoilingPointC =
        CalculateBoilingPointC(
            Config);

    return FMath::Clamp(
        (FluidTemperatureC - BoilingPointC)
            / Config.VaporTransitionRangeC,
        0.0,
        1.0);
}

double TABrakeHydraulics::CalculatePressureTransfer01(
    const FTABrakeHydraulicConfig& Config,
    const double VaporFraction01)
{
    return FMath::Clamp(
        1.0
        - FMath::Clamp(
            VaporFraction01,
            0.0,
            1.0)
        * FMath::Clamp(
            Config.MaxPressureLossAtFullVapor01,
            0.0,
            1.0),
        0.0,
        1.0);
}

double TABrakeHydraulics::CalculateCornerBrakeTorqueNm(
    const double EffectivePressurePa,
    const double CaliperPistonAreaM2,
    const double ClampGeometryFactor,
    const double PadFrictionCoefficient,
    const double EffectiveDiscRadiusM)
{
    if (!FMath::IsFinite(EffectivePressurePa)
        || !FMath::IsFinite(CaliperPistonAreaM2)
        || !FMath::IsFinite(ClampGeometryFactor)
        || !FMath::IsFinite(PadFrictionCoefficient)
        || !FMath::IsFinite(EffectiveDiscRadiusM)
        || EffectivePressurePa <= 0.0
        || CaliperPistonAreaM2 <= 0.0
        || ClampGeometryFactor <= 0.0
        || PadFrictionCoefficient <= 0.0
        || EffectiveDiscRadiusM <= 0.0)
    {
        return 0.0;
    }

    return
        EffectivePressurePa
        * CaliperPistonAreaM2
        * ClampGeometryFactor
        * PadFrictionCoefficient
        * EffectiveDiscRadiusM;
}

bool TABrakeHydraulics::Step(
    const FTABrakeHydraulicConfig& Config,
    const FTABrakeHydraulicInput& Input,
    FTABrakeHydraulicState& InOutState,
    FTABrakeHydraulicOutput& OutOutput)
{
    OutOutput =
        FTABrakeHydraulicOutput{};

    if (!ValidateConfig(Config)
        || !FMath::IsFinite(Input.Pedal01)
        || !FMath::IsFinite(Input.FrontPressureModulation01)
        || !FMath::IsFinite(Input.RearPressureModulation01)
        || !FMath::IsFinite(Input.DeltaTimeSeconds)
        || Input.DeltaTimeSeconds <= 0.0
        || !FMath::IsFinite(InOutState.FrontCircuitPressurePa)
        || !FMath::IsFinite(InOutState.RearCircuitPressurePa)
        || !FMath::IsFinite(InOutState.FrontCircuitHealth01)
        || !FMath::IsFinite(InOutState.RearCircuitHealth01)
        || !FMath::IsFinite(InOutState.FluidTemperatureC))
    {
        return false;
    }

    const double Pedal01 =
        FMath::Clamp(
            Input.Pedal01,
            0.0,
            1.0);

    const double PedalForceN =
        Pedal01
        * Config.PedalForceMaxN;

    const double MasterForceN =
        PedalForceN
        * Config.BoosterGain;

    OutOutput.MasterPressureRequestPa =
        FMath::Clamp(
            MasterForceN
                / Config.MasterCylinderAreaM2,
            0.0,
            Config.MaxSystemPressurePa);

    const double FrontModulation01 =
        FMath::Clamp(
            Input.FrontPressureModulation01,
            0.0,
            1.0);

    const double RearModulation01 =
        FMath::Clamp(
            Input.RearPressureModulation01,
            0.0,
            1.0);

    OutOutput.FrontTargetPressurePa =
        OutOutput.MasterPressureRequestPa
        * Config.FrontPressureRatio01
        * FrontModulation01;

    OutOutput.RearTargetPressurePa =
        OutOutput.MasterPressureRequestPa
        * Config.RearPressureRatio01
        * RearModulation01;

    InOutState.FrontCircuitPressurePa =
        FMath::Clamp(
            MoveToward(
                FMath::Max(
                    0.0,
                    InOutState.FrontCircuitPressurePa),
                OutOutput.FrontTargetPressurePa,
                Config.PressureRiseRatePaPerSec,
                Config.PressureReleaseRatePaPerSec,
                Input.DeltaTimeSeconds),
            0.0,
            Config.MaxSystemPressurePa);

    InOutState.RearCircuitPressurePa =
        FMath::Clamp(
            MoveToward(
                FMath::Max(
                    0.0,
                    InOutState.RearCircuitPressurePa),
                OutOutput.RearTargetPressurePa,
                Config.PressureRiseRatePaPerSec,
                Config.PressureReleaseRatePaPerSec,
                Input.DeltaTimeSeconds),
            0.0,
            Config.MaxSystemPressurePa);

    InOutState.VaporFraction01 =
        CalculateVaporFraction01(
            Config,
            InOutState.FluidTemperatureC);

    OutOutput.BoilingPointC =
        CalculateBoilingPointC(
            Config);

    OutOutput.VaporFraction01 =
        InOutState.VaporFraction01;

    OutOutput.FluidPressureTransfer01 =
        CalculatePressureTransfer01(
            Config,
            InOutState.VaporFraction01);

    OutOutput.FrontEffectivePressurePa =
        InOutState.FrontCircuitPressurePa
        * FMath::Clamp(
            InOutState.FrontCircuitHealth01,
            0.0,
            1.0)
        * OutOutput.FluidPressureTransfer01;

    OutOutput.RearEffectivePressurePa =
        InOutState.RearCircuitPressurePa
        * FMath::Clamp(
            InOutState.RearCircuitHealth01,
            0.0,
            1.0)
        * OutOutput.FluidPressureTransfer01;

    OutOutput.FrontCornerRawBrakeTorqueNm =
        CalculateCornerBrakeTorqueNm(
            OutOutput.FrontEffectivePressurePa,
            Config.FrontCaliperPistonAreaM2,
            Config.FrontClampGeometryFactor,
            Config.FrontPadFrictionCoefficient,
            Config.FrontEffectiveDiscRadiusM);

    OutOutput.RearCornerRawBrakeTorqueNm =
        CalculateCornerBrakeTorqueNm(
            OutOutput.RearEffectivePressurePa,
            Config.RearCaliperPistonAreaM2,
            Config.RearClampGeometryFactor,
            Config.RearPadFrictionCoefficient,
            Config.RearEffectiveDiscRadiusM);

    return true;
}

bool TABrakeHydraulics::UpdateFluidTemperature(
    const FTABrakeHydraulicConfig& Config,
    const double ConductedBrakeHeatPowerW,
    const double DeltaTimeSeconds,
    FTABrakeHydraulicState& InOutState)
{
    if (!ValidateConfig(Config)
        || !FMath::IsFinite(ConductedBrakeHeatPowerW)
        || ConductedBrakeHeatPowerW < 0.0
        || !FMath::IsFinite(DeltaTimeSeconds)
        || DeltaTimeSeconds <= 0.0
        || !FMath::IsFinite(InOutState.FluidTemperatureC))
    {
        return false;
    }

    const double TemperatureAboveAmbientC =
        FMath::Max(
            0.0,
            InOutState.FluidTemperatureC
                - Config.FluidAmbientTemperatureC);

    const double CoolingPowerW =
        Config.FluidCoolingWPerC
        * TemperatureAboveAmbientC;

    const double NetEnergyJ =
        (ConductedBrakeHeatPowerW
            - CoolingPowerW)
        * DeltaTimeSeconds;

    InOutState.FluidTemperatureC =
        FMath::Max(
            Config.FluidAmbientTemperatureC,
            InOutState.FluidTemperatureC
                + NetEnergyJ
                    / Config.FluidThermalMassJPerC);

    InOutState.VaporFraction01 =
        CalculateVaporFraction01(
            Config,
            InOutState.FluidTemperatureC);

    return true;
}
