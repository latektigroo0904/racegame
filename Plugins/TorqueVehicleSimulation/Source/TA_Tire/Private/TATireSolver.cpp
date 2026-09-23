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

    double CalculateEffectiveMu(
        const FTATireRuntimeConfig& Config,
        const FTATireRuntimeState& State,
        const FTASurfaceSample& Surface,
        const double RoadSupportedLoadN)
    {
        if (RoadSupportedLoadN <= UE_DOUBLE_SMALL_NUMBER)
        {
            return 0.0;
        }

        const double ReferenceLoadN =
            FMath::Max(1.0, Config.ReferenceLoadN);

        const double LoadRatio =
            FMath::Max(
                0.05,
                RoadSupportedLoadN / ReferenceLoadN);

        const double LoadSensitiveMu =
            Config.DryPeakMu
            * FMath::Pow(
                LoadRatio,
                -Config.LoadSensitivityExponent);

        const double SurfaceMultiplier =
            TASurface::CalculateBaselineFrictionMultiplier(
                Surface);

        const double TemperatureFactor =
            TATireSolver::CalculateTemperatureGripFactor(
                Config,
                State.SurfaceTemperatureC);

        const double PressureFactor =
            TATireSolver::CalculatePressureGripFactor(
                Config,
                State.PressureKPa);

        const double WearFactor =
            1.0
            - FMath::Clamp(
                Config.WearGripLossAtEnd,
                0.0,
                0.8)
            * FMath::Clamp(State.Wear01, 0.0, 1.0);

        const double DamageGripFactor =
            1.0
            - 0.5
            * FMath::Clamp(State.Damage01, 0.0, 1.0);

        const double ThermalDamageFactor =
            1.0
            - 0.35
            * FMath::Clamp(
                State.ThermalDegradation01,
                0.0,
                1.0);

        return FMath::Max(
            0.0,
            LoadSensitiveMu
            * SurfaceMultiplier
            * TemperatureFactor
            * PressureFactor
            * WearFactor
            * DamageGripFactor
            * ThermalDamageFactor);
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

FTATireVerticalForceOutput TATireSolver::CalculateVerticalForce(
    const FTATireRuntimeConfig& Config,
    const FTATireRuntimeState& State,
    const double RequestedDeflectionM,
    const double DeflectionVelocityMps)
{
    FTATireVerticalForceOutput Output;

    Output.RequestedDeflectionM =
        FMath::Max(0.0, RequestedDeflectionM);

    const double MaxDeflectionM =
        FMath::Max(
            0.001,
            Config.MaxRadialDeflectionM);

    Output.EffectiveDeflectionM =
        FMath::Min(
            Output.RequestedDeflectionM,
            MaxDeflectionM);

    Output.DeflectionVelocityMps =
        DeflectionVelocityMps;

    Output.bBottomed =
        Output.RequestedDeflectionM
        >= MaxDeflectionM;

    const double PressureRatio =
        FMath::Clamp(
            State.PressureKPa
            / FMath::Max(
                1.0,
                Config.ReferencePressureKPa),
            0.25,
            2.0);

    Output.EffectiveRadialStiffnessNPerM =
        FMath::Max(
            0.0,
            Config.RadialStiffnessNPerM)
        * FMath::Pow(
            PressureRatio,
            FMath::Max(
                0.0,
                Config.PressureRadialStiffnessExponent));

    const double LinearForceN =
        Output.EffectiveRadialStiffnessNPerM
        * Output.EffectiveDeflectionM;

    const double ProgressiveForceN =
        FMath::Max(
            0.0,
            Config.RadialProgressiveStiffnessNPerM2)
        * FMath::Square(
            Output.EffectiveDeflectionM);

    const double DampingForceN =
        FMath::Max(
            0.0,
            Config.RadialDampingNsPerM)
        * Output.DeflectionVelocityMps;

    Output.NormalForceN =
        FMath::Max(
            0.0,
            LinearForceN
            + ProgressiveForceN
            + DampingForceN);

    return Output;
}

void TATireSolver::CommitVerticalState(
    const FTATireVerticalForceOutput& Vertical,
    FTATireRuntimeState& InOutState)
{
    InOutState.RadialDeflectionM =
        Vertical.EffectiveDeflectionM;

    InOutState.RadialDeflectionVelocityMps =
        Vertical.DeflectionVelocityMps;

    InOutState.bRadialStateInitialized =
        true;
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

double TATireSolver::CalculateTemperatureGripFactor(
    const FTATireRuntimeConfig& Config,
    const double SurfaceTemperatureC)
{
    const double OptimalC =
        Config.OptimalSurfaceTemperatureC;

    const double ColdReferenceC =
        FMath::Min(
            OptimalC - 1.0,
            Config.ReferencePressureTemperatureC);

    if (SurfaceTemperatureC <= OptimalC)
    {
        const double Range =
            FMath::Max(
                1.0,
                OptimalC - ColdReferenceC);

        const double Alpha =
            FMath::Clamp(
                (SurfaceTemperatureC - ColdReferenceC)
                / Range,
                0.0,
                1.0);

        return FMath::Lerp(
            FMath::Clamp(
                Config.ColdGripMultiplier,
                0.20,
                1.0),
            1.0,
            Alpha);
    }

    const double HotRange =
        FMath::Max(
            1.0,
            Config.HotGripTemperatureC
            - OptimalC);

    const double HotAlpha =
        FMath::Clamp(
            (SurfaceTemperatureC - OptimalC)
            / HotRange,
            0.0,
            1.0);

    return FMath::Lerp(
        1.0,
        FMath::Clamp(
            Config.HotGripMultiplier,
            0.20,
            1.0),
        HotAlpha);
}

double TATireSolver::CalculatePressureGripFactor(
    const FTATireRuntimeConfig& Config,
    const double PressureKPa)
{
    const double ReferencePressure =
        FMath::Max(
            1.0,
            Config.ReferencePressureKPa);

    const double RelativeError =
        FMath::Abs(
            PressureKPa - ReferencePressure)
        / ReferencePressure;

    return FMath::Clamp(
        1.0
        - FMath::Max(
            0.0,
            Config.PressureGripSensitivity)
        * RelativeError,
        0.65,
        1.0);
}

void TATireSolver::UpdateThermalPressureAndWear(
    const FTATireRuntimeConfig& Config,
    const FTATireSolveInput& Input,
    const FTATireSolveOutput& Output,
    const double DeltaTimeSeconds,
    FTATireRuntimeState& InOutState)
{
    if (DeltaTimeSeconds <= 0.0)
    {
        return;
    }

    const double AmbientC =
        Input.Surface.TemperatureC;

    const double WheelSurfaceSpeedMps =
        Input.WheelAngularSpeedRadPerSec
        * FMath::Max(0.01, Config.UnloadedRadiusM);

    const double LongitudinalSlipVelocityMps =
        WheelSurfaceSpeedMps
        - Input.LongitudinalVelocityMps;

    const double SlipPowerW =
        FMath::Abs(
            Output.LongitudinalForceN
            * LongitudinalSlipVelocityMps)
        + FMath::Abs(
            Output.LateralForceN
            * Input.LateralVelocityMps);

    const double RollingPowerW =
        FMath::Abs(
            Output.RollingResistanceForceN
            * Input.LongitudinalVelocityMps);

    const double SurfaceHeatInputW =
        FMath::Max(0.0, Config.SlipHeatFraction)
            * SlipPowerW
        + FMath::Max(0.0, Config.RollingHeatFraction)
            * RollingPowerW;

    const double SurfaceToCarcassW =
        FMath::Max(
            0.0,
            Config.SurfaceToCarcassConductanceWPerC)
        * (InOutState.SurfaceTemperatureC
            - InOutState.CarcassTemperatureC);

    const double CarcassToAmbientW =
        FMath::Max(
            0.0,
            Config.CarcassToAmbientConductanceWPerC)
        * (InOutState.CarcassTemperatureC
            - AmbientC);

    if (Config.SurfaceThermalMassJPerC
        > UE_DOUBLE_SMALL_NUMBER)
    {
        InOutState.SurfaceTemperatureC +=
            (SurfaceHeatInputW
                - SurfaceToCarcassW)
            * DeltaTimeSeconds
            / Config.SurfaceThermalMassJPerC;
    }

    if (Config.CarcassThermalMassJPerC
        > UE_DOUBLE_SMALL_NUMBER)
    {
        InOutState.CarcassTemperatureC +=
            (SurfaceToCarcassW
                - CarcassToAmbientW)
            * DeltaTimeSeconds
            / Config.CarcassThermalMassJPerC;
    }

    InOutState.SurfaceTemperatureC =
        FMath::Max(
            AmbientC,
            InOutState.SurfaceTemperatureC);

    InOutState.CarcassTemperatureC =
        FMath::Max(
            AmbientC,
            InOutState.CarcassTemperatureC);

    const double AirTimeConstant =
        FMath::Max(
            0.1,
            Config.InternalAirTimeConstantSeconds);

    const double AirBlend =
        1.0
        - FMath::Exp(
            -DeltaTimeSeconds / AirTimeConstant);

    InOutState.InternalAirTemperatureC =
        FMath::Lerp(
            InOutState.InternalAirTemperatureC,
            InOutState.CarcassTemperatureC,
            FMath::Clamp(AirBlend, 0.0, 1.0));

    const double ReferenceAbsolutePressureKPa =
        FMath::Max(
            1.0,
            Config.ReferencePressureKPa + 101.325);

    const double ReferenceTemperatureK =
        FMath::Max(
            1.0,
            Config.ReferencePressureTemperatureC + 273.15);

    const double CurrentTemperatureK =
        FMath::Max(
            1.0,
            InOutState.InternalAirTemperatureC + 273.15);

    InOutState.PressureKPa =
        FMath::Max(
            0.0,
            ReferenceAbsolutePressureKPa
            * CurrentTemperatureK
            / ReferenceTemperatureK
            - 101.325);

    const double DissipatedEnergyJ =
        (SlipPowerW + RollingPowerW)
        * DeltaTimeSeconds;

    if (Config.WearEnergyCapacityJ
        > UE_DOUBLE_SMALL_NUMBER)
    {
        InOutState.Wear01 =
            FMath::Clamp(
                InOutState.Wear01
                + DissipatedEnergyJ
                / Config.WearEnergyCapacityJ,
                0.0,
                1.0);
    }

    InOutState.TreadDepthMm =
        FMath::Lerp(
            FMath::Max(
                0.0,
                Config.NewTreadDepthMm),
            FMath::Clamp(
                Config.MinimumTreadDepthMm,
                0.0,
                FMath::Max(
                    0.0,
                    Config.NewTreadDepthMm)),
            InOutState.Wear01);

    if (InOutState.SurfaceTemperatureC
        > Config.ThermalDegradationStartC)
    {
        const double Severity =
            FMath::Clamp(
                (InOutState.SurfaceTemperatureC
                    - Config.ThermalDegradationStartC)
                / FMath::Max(
                    1.0,
                    170.0
                    - Config.ThermalDegradationStartC),
                0.0,
                2.0);

        InOutState.ThermalDegradation01 =
            FMath::Clamp(
                InOutState.ThermalDegradation01
                + Severity
                * FMath::Max(
                    0.0,
                    Config.ThermalDegradationRatePerSecondAt170C)
                * DeltaTimeSeconds,
                0.0,
                1.0);
    }
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

    const double EffectiveMu =
        CalculateEffectiveMu(
            Config,
            State,
            Input.Surface,
            RoadSupportedLoadN);

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

    const double ReferenceLoadN =
        FMath::Max(1.0, Config.ReferenceLoadN);

    const double LoadRatio =
        FMath::Max(
            0.05,
            Output.RoadSupportedLoadN / ReferenceLoadN);

    const double EffectiveMu =
        CalculateEffectiveMu(
            Config,
            State,
            Input.Surface,
            Output.RoadSupportedLoadN);

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
