#pragma once

#include "CoreMinimal.h"
#include "TASurfaceTypes.h"

struct TA_TIRE_API FTATireRuntimeConfig
{
    double UnloadedRadiusM = 0.327;
    double ReferenceLoadN = 3500.0;

    double DryPeakMu = 1.05;
    double LoadSensitivityExponent = 0.08;

    double LongitudinalStiffnessN = 90000.0;
    double CorneringStiffnessNPerRad = 85000.0;
    double CamberStiffnessNPerRad = 6000.0;

    double SaturationExponent = 2.0;
    double PneumaticTrailM = 0.055;
    double RollingResistanceCoefficient = 0.012;

    double ReferencePressureKPa = 230.0;
    double ReferencePressureTemperatureC = 20.0;

    double RadialStiffnessNPerM = 220000.0;
    double RadialProgressiveStiffnessNPerM2 = 1200000.0;
    double RadialDampingNsPerM = 2200.0;
    double MaxRadialDeflectionM = 0.060;
    double PressureRadialStiffnessExponent = 0.45;

    double NewTreadDepthMm = 7.5;
    double MinimumTreadDepthMm = 0.8;

    double OptimalSurfaceTemperatureC = 85.0;
    double ColdGripMultiplier = 0.78;
    double HotGripMultiplier = 0.82;
    double HotGripTemperatureC = 135.0;

    double PressureGripSensitivity = 0.20;
    double WearGripLossAtEnd = 0.15;

    double SurfaceThermalMassJPerC = 4200.0;
    double CarcassThermalMassJPerC = 12000.0;

    double SurfaceToCarcassConductanceWPerC = 95.0;
    double CarcassToAmbientConductanceWPerC = 38.0;

    double InternalAirTimeConstantSeconds = 35.0;

    double SlipHeatFraction = 0.18;
    double RollingHeatFraction = 0.50;

    double WearEnergyCapacityJ = 8.0e7;
    double ThermalDegradationStartC = 135.0;
    double ThermalDegradationRatePerSecondAt170C = 0.0015;

    double SlipReferenceVelocityMps = 0.5;
    double DynamicBlendStartMps = 2.0;
    double DynamicBlendEndMps = 5.0;

    // Calibration seeds for continuous standing-water support loss.
    double HydroReferenceOnsetSpeedMps = 32.0;
    double HydroReferenceWaterDepthMm = 4.0;
};

struct TA_TIRE_API FTATireRuntimeState
{
    double SurfaceTemperatureC = 20.0;
    double CarcassTemperatureC = 20.0;
    double InternalAirTemperatureC = 20.0;

    double PressureKPa = 230.0;

    double RadialDeflectionM = 0.0;
    double RadialDeflectionVelocityMps = 0.0;
    bool bRadialStateInitialized = false;

    double TreadDepthMm = 7.5;
    double Wear01 = 0.0;
    double ThermalDegradation01 = 0.0;
    double Damage01 = 0.0;
};

struct TA_TIRE_API FTATireVerticalForceOutput
{
    double RequestedDeflectionM = 0.0;
    double EffectiveDeflectionM = 0.0;
    double DeflectionVelocityMps = 0.0;

    double EffectiveRadialStiffnessNPerM = 0.0;
    double NormalForceN = 0.0;

    bool bBottomed = false;
};

struct TA_TIRE_API FTATireSolveInput
{
    double VerticalLoadN = 0.0;
    double LongitudinalVelocityMps = 0.0;
    double LateralVelocityMps = 0.0;
    double WheelAngularSpeedRadPerSec = 0.0;
    double CamberRad = 0.0;

    FTASurfaceSample Surface;
};

struct TA_TIRE_API FTATireSolveOutput
{
    double SlipRatio = 0.0;
    double SlipAngleRad = 0.0;

    double RoadSupportedLoadN = 0.0;
    double HydroFraction01 = 0.0;

    double LongitudinalForceN = 0.0;
    double LateralForceN = 0.0;
    double AligningMomentNm = 0.0;
    double RollingResistanceForceN = 0.0;
};

namespace TATireSolver
{
    TA_TIRE_API double CalculateSlipRatio(
        double WheelSurfaceSpeedMps,
        double GroundLongitudinalSpeedMps,
        double ReferenceVelocityMps);

    TA_TIRE_API double CalculateSlipAngleRad(
        double LongitudinalVelocityMps,
        double LateralVelocityMps,
        double ReferenceVelocityMps);

    TA_TIRE_API FTATireVerticalForceOutput CalculateVerticalForce(
        const FTATireRuntimeConfig& Config,
        const FTATireRuntimeState& State,
        double RequestedDeflectionM,
        double DeflectionVelocityMps);

    TA_TIRE_API void CommitVerticalState(
        const FTATireVerticalForceOutput& Vertical,
        FTATireRuntimeState& InOutState);

    TA_TIRE_API double CalculateHydroFraction01(
        const FTATireRuntimeConfig& Config,
        const FTATireRuntimeState& State,
        const FTATireSolveInput& Input);

    TA_TIRE_API double CalculateTemperatureGripFactor(
        const FTATireRuntimeConfig& Config,
        double SurfaceTemperatureC);

    TA_TIRE_API double CalculatePressureGripFactor(
        const FTATireRuntimeConfig& Config,
        double PressureKPa);

    TA_TIRE_API void UpdateThermalPressureAndWear(
        const FTATireRuntimeConfig& Config,
        const FTATireSolveInput& Input,
        const FTATireSolveOutput& Output,
        double DeltaTimeSeconds,
        FTATireRuntimeState& InOutState);

    TA_TIRE_API double EstimateLongitudinalForceCapacityN(
        const FTATireRuntimeConfig& Config,
        const FTATireRuntimeState& State,
        const FTATireSolveInput& Input);

    TA_TIRE_API FTATireSolveOutput Solve(
        const FTATireRuntimeConfig& Config,
        const FTATireRuntimeState& State,
        const FTATireSolveInput& Input);
}
