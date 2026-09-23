#include "TATireSolver.h"

namespace
{
    double SmoothStep01(const double T)
    {
        const double X = FMath::Clamp(T, 0.0, 1.0);
        return X * X * (3.0 - 2.0 * X);
    }

    double Saturation(const double S, const double Exponent)
    {
        const double Q = FMath::Max(1.0, Exponent);
        const double AbsS = FMath::Abs(S);
        const double Denominator = FMath::Pow(1.0 + FMath::Pow(AbsS, Q), 1.0 / Q);

        if (Denominator <= UE_DOUBLE_SMALL_NUMBER)
        {
            return 0.0;
        }

        return S / Denominator;
    }
}

double TATireSolver::CalculateSlipRatio(
    const double WheelSurfaceSpeedMps,
    const double GroundLongitudinalSpeedMps,
    const double ReferenceVelocityMps)
{
    const double Denominator =
        FMath::Max(FMath::Abs(GroundLongitudinalSpeedMps), FMath::Max(0.01, ReferenceVelocityMps));

    return (WheelSurfaceSpeedMps - GroundLongitudinalSpeedMps) / Denominator;
}

double TATireSolver::CalculateSlipAngleRad(
    const double LongitudinalVelocityMps,
    const double LateralVelocityMps,
    const double ReferenceVelocityMps)
{
    const double Denominator =
        FMath::Max(FMath::Abs(LongitudinalVelocityMps), FMath::Max(0.01, ReferenceVelocityMps));

    return FMath::Atan2(-LateralVelocityMps, Denominator);
}

double TATireSolver::CalculateHydroFraction01(
    const FTATireRuntimeConfig& Config,
    const FTATireRuntimeState& State,
    const FTATireSolveInput& Input)
{
    if (Input.VerticalLoadN <= 0.0 || Input.Surface.WaterDepthMm <= 0.0)
    {
        return 0.0;
    }

    const double WaterDepthFactor = FMath::Clamp(
        Input.Surface.WaterDepthMm / FMath::Max(0.1, Config.HydroReferenceWaterDepthMm),
        0.0,
        2.0);

    const double TreadRatio = FMath::Clamp(
        State.TreadDepthMm / FMath::Max(0.1, Config.NewTreadDepthMm),
        0.05,
        1.25);

    const double PressureRatio = FMath::Clamp(
        State.PressureKPa / FMath::Max(1.0, Config.ReferencePressureKPa),
        0.5,
        1.5);

    const double LoadRatio = FMath::Clamp(
        Input.VerticalLoadN / FMath::Max(1.0, Config.ReferenceLoadN),
        0.25,
        2.5);

    // Provisional calibration relationship:
    // more tread, pressure and vertical load resist hydro lift;
    // deeper water lowers the effective onset speed.
    const double ResistanceFactor =
        FMath::Sqrt(PressureRatio)
        * FMath::Pow(TreadRatio, 0.35)
        * FMath::Pow(LoadRatio, 0.12);

    const double WaterPenalty =
        1.0 / FMath::Sqrt(FMath::Max(0.25, WaterDepthFactor));

    const double OnsetSpeedMps =
        Config.HydroReferenceOnsetSpeedMps
        * ResistanceFactor
        * WaterPenalty;

    const double SpeedMps = FMath::Abs(Input.LongitudinalVelocityMps);
    const double BlendStart = OnsetSpeedMps * 0.75;
    const double BlendEnd = FMath::Max(BlendStart + 0.1, OnsetSpeedMps * 1.35);

    const double SpeedFactor =
        SmoothStep01((SpeedMps - BlendStart) / (BlendEnd - BlendStart));

    const double DepthStrength = FMath::Clamp(WaterDepthFactor, 0.0, 1.0);

    return FMath::Clamp(SpeedFactor * DepthStrength, 0.0, 1.0);
}

double TATireSolver::EstimateLongitudinalForceCapacityN(
    const FTATireRuntimeConfig& Config,
    const FTATireRuntimeState& State,
    const FTATireSolveInput& Input)
{
    const double Fz = FMath::Max(0.0, Input.VerticalLoadN);
    if (Fz <= UE_DOUBLE_SMALL_NUMBER)
    {
        return 0.0;
    }

    const double HydroFraction =
        CalculateHydroFraction01(Config, State, Input);

    const double RoadSupportedLoadN =
        Fz * (1.0 - HydroFraction);

    if (RoadSupportedLoadN <= UE_DOUBLE_SMALL_NUMBER)
    {
        return 0.0;
    }

    const double ReferenceLoadN =
        FMath::Max(1.0, Config.ReferenceLoadN);

    const double LoadRatio =
        FMath::Max(0.05, RoadSupportedLoadN / ReferenceLoadN);

    const double LoadSensitiveMu =
        Config.DryPeakMu
        * FMath::Pow(LoadRatio, -Config.LoadSensitivityExponent);

    const double SurfaceMultiplier =
        TASurface::CalculateBaselineFrictionMultiplier(Input.Surface);

    const double DamageGripFactor =
        1.0 - 0.5 * FMath::Clamp(State.Damage01, 0.0, 1.0);

    const double ThermalDamageFactor =
        1.0 - 0.35 * FMath::Clamp(State.ThermalDegradation01, 0.0, 1.0);

    const double EffectiveMu =
        FMath::Max(
            0.0,
            LoadSensitiveMu
            * SurfaceMultiplier
            * DamageGripFactor
            * ThermalDamageFactor);

    return EffectiveMu * RoadSupportedLoadN;
}

FTATireSolveOutput TATireSolver::Solve(
    const FTATireRuntimeConfig& Config,
    const FTATireRuntimeState& State,
    const FTATireSolveInput& Input)
{
    FTATireSolveOutput Output;

    const double Fz = FMath::Max(0.0, Input.VerticalLoadN);
    if (Fz <= UE_DOUBLE_SMALL_NUMBER)
    {
        return Output;
    }

    const double RadiusM = FMath::Max(0.01, Config.UnloadedRadiusM);
    const double WheelSurfaceSpeedMps =
        Input.WheelAngularSpeedRadPerSec * RadiusM;

    Output.SlipRatio = CalculateSlipRatio(
        WheelSurfaceSpeedMps,
        Input.LongitudinalVelocityMps,
        Config.SlipReferenceVelocityMps);

    Output.SlipAngleRad = CalculateSlipAngleRad(
        Input.LongitudinalVelocityMps,
        Input.LateralVelocityMps,
        Config.SlipReferenceVelocityMps);

    Output.HydroFraction01 = CalculateHydroFraction01(Config, State, Input);
    Output.RoadSupportedLoadN = Fz * (1.0 - Output.HydroFraction01);

    if (Output.RoadSupportedLoadN <= UE_DOUBLE_SMALL_NUMBER)
    {
        return Output;
    }

    const double ReferenceLoadN = FMath::Max(1.0, Config.ReferenceLoadN);
    const double LoadRatio = FMath::Max(0.05, Output.RoadSupportedLoadN / ReferenceLoadN);

    const double LoadSensitiveMu =
        Config.DryPeakMu
        * FMath::Pow(LoadRatio, -Config.LoadSensitivityExponent);

    const double SurfaceMultiplier =
        TASurface::CalculateBaselineFrictionMultiplier(Input.Surface);

    const double DamageGripFactor =
        1.0 - 0.5 * FMath::Clamp(State.Damage01, 0.0, 1.0);

    const double ThermalDamageFactor =
        1.0 - 0.35 * FMath::Clamp(State.ThermalDegradation01, 0.0, 1.0);

    const double EffectiveMu =
        FMath::Max(
            0.0,
            LoadSensitiveMu
            * SurfaceMultiplier
            * DamageGripFactor
            * ThermalDamageFactor);

    const double ForceMaxN =
        FMath::Max(1.0, EffectiveMu * Output.RoadSupportedLoadN);

    const double Sx =
        Config.LongitudinalStiffnessN * Output.SlipRatio / ForceMaxN;

    const double Sy =
        Config.CorneringStiffnessNPerRad
        * FMath::Tan(Output.SlipAngleRad)
        / ForceMaxN;

    double Fx =
        ForceMaxN * Saturation(Sx, Config.SaturationExponent);

    double Fy =
        ForceMaxN * Saturation(Sy, Config.SaturationExponent);

    const double CamberForceN =
        Config.CamberStiffnessNPerRad
        * Input.CamberRad
        * LoadRatio;

    Fy += CamberForceN;

    const double Demand =
        FMath::Sqrt(
            FMath::Square(Fx / ForceMaxN)
            + FMath::Square(Fy / ForceMaxN));

    if (Demand > 1.0)
    {
        Fx /= Demand;
        Fy /= Demand;
    }

    Output.LongitudinalForceN = Fx;
    Output.LateralForceN = Fy;

    const double LateralDemand =
        FMath::Clamp(FMath::Abs(Fy) / ForceMaxN, 0.0, 1.0);

    const double TrailM =
        FMath::Max(0.0, Config.PneumaticTrailM) * (1.0 - LateralDemand);

    Output.AligningMomentNm = -Fy * TrailM;

    const double Speed = Input.LongitudinalVelocityMps;
    const double SmoothDirection =
        Speed / FMath::Sqrt(Speed * Speed + 0.25);

    Output.RollingResistanceForceN =
        -Config.RollingResistanceCoefficient
        * Output.RoadSupportedLoadN
        * SmoothDirection;

    return Output;
}
