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

    bool IsFrontCorner(const int32 CornerIndex)
    {
        return
            CornerIndex
                == static_cast<int32>(
                    ETABrakeCornerIndex::FrontLeft)
            || CornerIndex
                == static_cast<int32>(
                    ETABrakeCornerIndex::FrontRight);
    }

    bool UsesCircuitA(
        const ETABrakeCircuitTopology Topology,
        const int32 CornerIndex)
    {
        const int32 FrontLeft =
            static_cast<int32>(
                ETABrakeCornerIndex::FrontLeft);

        const int32 FrontRight =
            static_cast<int32>(
                ETABrakeCornerIndex::FrontRight);

        const int32 RearLeft =
            static_cast<int32>(
                ETABrakeCornerIndex::RearLeft);

        const int32 RearRight =
            static_cast<int32>(
                ETABrakeCornerIndex::RearRight);

        if (Topology == ETABrakeCircuitTopology::FrontRear)
        {
            return CornerIndex == FrontLeft
                || CornerIndex == FrontRight;
        }

        return CornerIndex == FrontLeft
            || CornerIndex == RearRight;
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
        || !FMath::IsFinite(Input.DeltaTimeSeconds)
        || Input.DeltaTimeSeconds <= 0.0
        || !FMath::IsFinite(InOutState.CircuitAHealth01)
        || !FMath::IsFinite(InOutState.CircuitBHealth01)
        || !FMath::IsFinite(InOutState.FluidTemperatureC))
    {
        return false;
    }

    for (int32 CornerIndex = 0;
         CornerIndex < TABrakeCornerCount;
         ++CornerIndex)
    {
        if (!FMath::IsFinite(
                Input.CornerPressureModulation01[CornerIndex])
            || !FMath::IsFinite(
                InOutState.CornerLinePressurePa[CornerIndex]))
        {
            return false;
        }
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

    for (int32 CornerIndex = 0;
         CornerIndex < TABrakeCornerCount;
         ++CornerIndex)
    {
        const bool bFront =
            IsFrontCorner(CornerIndex);

        const double AxlePressureRatio01 =
            bFront
            ? Config.FrontPressureRatio01
            : Config.RearPressureRatio01;

        const double Modulation01 =
            FMath::Clamp(
                Input.CornerPressureModulation01[CornerIndex],
                0.0,
                1.0);

        OutOutput.CornerTargetPressurePa[CornerIndex] =
            OutOutput.MasterPressureRequestPa
            * AxlePressureRatio01
            * Modulation01;

        InOutState.CornerLinePressurePa[CornerIndex] =
            FMath::Clamp(
                MoveToward(
                    FMath::Max(
                        0.0,
                        InOutState.CornerLinePressurePa[CornerIndex]),
                    OutOutput.CornerTargetPressurePa[CornerIndex],
                    Config.PressureRiseRatePaPerSec,
                    Config.PressureReleaseRatePaPerSec,
                    Input.DeltaTimeSeconds),
                0.0,
                Config.MaxSystemPressurePa);

        const double CircuitHealth01 =
            UsesCircuitA(
                Config.CircuitTopology,
                CornerIndex)
            ? FMath::Clamp(
                InOutState.CircuitAHealth01,
                0.0,
                1.0)
            : FMath::Clamp(
                InOutState.CircuitBHealth01,
                0.0,
                1.0);

        OutOutput.CornerEffectivePressurePa[CornerIndex] =
            InOutState.CornerLinePressurePa[CornerIndex]
            * CircuitHealth01
            * OutOutput.FluidPressureTransfer01;

        const double PistonAreaM2 =
            bFront
            ? Config.FrontCaliperPistonAreaM2
            : Config.RearCaliperPistonAreaM2;

        const double ClampGeometryFactor =
            bFront
            ? Config.FrontClampGeometryFactor
            : Config.RearClampGeometryFactor;

        const double PadFrictionCoefficient =
            bFront
            ? Config.FrontPadFrictionCoefficient
            : Config.RearPadFrictionCoefficient;

        const double EffectiveDiscRadiusM =
            bFront
            ? Config.FrontEffectiveDiscRadiusM
            : Config.RearEffectiveDiscRadiusM;

        OutOutput.CornerRawBrakeTorqueNm[CornerIndex] =
            CalculateCornerBrakeTorqueNm(
                OutOutput.CornerEffectivePressurePa[CornerIndex],
                PistonAreaM2,
                ClampGeometryFactor,
                PadFrictionCoefficient,
                EffectiveDiscRadiusM);
    }

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
