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
    double NewTreadDepthMm = 7.5;

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
    double TreadDepthMm = 7.5;
    double Wear01 = 0.0;
    double ThermalDegradation01 = 0.0;
    double Damage01 = 0.0;
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

    TA_TIRE_API double CalculateHydroFraction01(
        const FTATireRuntimeConfig& Config,
        const FTATireRuntimeState& State,
        const FTATireSolveInput& Input);

    TA_TIRE_API double EstimateLongitudinalForceCapacityN(
        const FTATireRuntimeConfig& Config,
        const FTATireRuntimeState& State,
        const FTATireSolveInput& Input);

    TA_TIRE_API FTATireSolveOutput Solve(
        const FTATireRuntimeConfig& Config,
        const FTATireRuntimeState& State,
        const FTATireSolveInput& Input);
}
