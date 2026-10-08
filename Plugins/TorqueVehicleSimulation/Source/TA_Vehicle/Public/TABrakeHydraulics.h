#pragma once

#include "CoreMinimal.h"

struct TA_VEHICLE_API FTABrakeHydraulicConfig
{
    double PedalForceMaxN = 450.0;
    double BoosterGain = 4.5;
    double MasterCylinderAreaM2 = 5.0e-4;
    double MaxSystemPressurePa = 12.0e6;

    double FrontPressureRatio01 = 1.0;
    double RearPressureRatio01 = 0.70;

    double PressureRiseRatePaPerSec = 80.0e6;
    double PressureReleaseRatePaPerSec = 120.0e6;

    double FrontCaliperPistonAreaM2 = 3.0e-3;
    double RearCaliperPistonAreaM2 = 2.0e-3;

    double FrontClampGeometryFactor = 2.0;
    double RearClampGeometryFactor = 2.0;

    double FrontPadFrictionCoefficient = 0.40;
    double RearPadFrictionCoefficient = 0.38;

    double FrontEffectiveDiscRadiusM = 0.145;
    double RearEffectiveDiscRadiusM = 0.125;

    double FluidDryBoilingPointC = 260.0;
    double FluidWetBoilingPointC = 165.0;
    double FluidWaterContamination01 = 0.10;
    double VaporTransitionRangeC = 25.0;
    double MaxPressureLossAtFullVapor01 = 0.70;

    double FluidThermalMassJPerC = 9000.0;
    double FluidCoolingWPerC = 18.0;
    double FluidAmbientTemperatureC = 20.0;
};

struct TA_VEHICLE_API FTABrakeHydraulicState
{
    double FrontCircuitPressurePa = 0.0;
    double RearCircuitPressurePa = 0.0;

    double FrontCircuitHealth01 = 1.0;
    double RearCircuitHealth01 = 1.0;

    double FluidTemperatureC = 20.0;
    double VaporFraction01 = 0.0;
};

struct TA_VEHICLE_API FTABrakeHydraulicInput
{
    double Pedal01 = 0.0;

    // Future ABS/EBD/ESC controllers modulate pressure demand through these
    // actuator authorities rather than editing wheel speed or tire grip.
    double FrontPressureModulation01 = 1.0;
    double RearPressureModulation01 = 1.0;

    double DeltaTimeSeconds = 1.0 / 240.0;
};

struct TA_VEHICLE_API FTABrakeHydraulicOutput
{
    double MasterPressureRequestPa = 0.0;

    double FrontTargetPressurePa = 0.0;
    double RearTargetPressurePa = 0.0;

    double FrontEffectivePressurePa = 0.0;
    double RearEffectivePressurePa = 0.0;

    double FrontCornerRawBrakeTorqueNm = 0.0;
    double RearCornerRawBrakeTorqueNm = 0.0;

    double BoilingPointC = 0.0;
    double VaporFraction01 = 0.0;
    double FluidPressureTransfer01 = 1.0;
};

namespace TABrakeHydraulics
{
    TA_VEHICLE_API bool ValidateConfig(
        const FTABrakeHydraulicConfig& Config);

    TA_VEHICLE_API void InitializeState(
        const FTABrakeHydraulicConfig& Config,
        FTABrakeHydraulicState& OutState);

    TA_VEHICLE_API double CalculateBoilingPointC(
        const FTABrakeHydraulicConfig& Config);

    TA_VEHICLE_API double CalculateVaporFraction01(
        const FTABrakeHydraulicConfig& Config,
        double FluidTemperatureC);

    TA_VEHICLE_API double CalculatePressureTransfer01(
        const FTABrakeHydraulicConfig& Config,
        double VaporFraction01);

    TA_VEHICLE_API double CalculateCornerBrakeTorqueNm(
        double EffectivePressurePa,
        double CaliperPistonAreaM2,
        double ClampGeometryFactor,
        double PadFrictionCoefficient,
        double EffectiveDiscRadiusM);

    TA_VEHICLE_API bool Step(
        const FTABrakeHydraulicConfig& Config,
        const FTABrakeHydraulicInput& Input,
        FTABrakeHydraulicState& InOutState,
        FTABrakeHydraulicOutput& OutOutput);

    TA_VEHICLE_API bool UpdateFluidTemperature(
        const FTABrakeHydraulicConfig& Config,
        double ConductedBrakeHeatPowerW,
        double DeltaTimeSeconds,
        FTABrakeHydraulicState& InOutState);
}
