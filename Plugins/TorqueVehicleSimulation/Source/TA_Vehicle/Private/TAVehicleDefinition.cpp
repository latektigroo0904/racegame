#include "TAVehicleDefinition.h"

namespace
{
    void AddValidation(
        FTAValidationResult& Result,
        const ETAValidationSeverity Severity,
        const FName Code,
        const FString& Message)
    {
        FTAValidationMessage Entry;
        Entry.Severity = Severity;
        Entry.Code = Code;
        Entry.Message = Message;
        Result.Messages.Add(MoveTemp(Entry));
    }

    uint32 HashDouble(
        uint32 Seed,
        const double Value)
    {
        return HashCombineFast(
            Seed,
            GetTypeHash(Value));
    }

    uint32 HashBool(
        uint32 Seed,
        const bool Value)
    {
        return HashCombineFast(
            Seed,
            GetTypeHash(Value));
    }

    uint32 HashVector(
        uint32 Seed,
        const FVector3d& Value)
    {
        Seed = HashDouble(Seed, Value.X);
        Seed = HashDouble(Seed, Value.Y);
        Seed = HashDouble(Seed, Value.Z);
        return Seed;
    }

    FVector3d ToComLocal(
        const FVector& VehicleLocalPoint,
        const FVector3d& CenterOfMassVehicleLocalM)
    {
        return
            FVector3d(VehicleLocalPoint)
            - CenterOfMassVehicleLocalM;
    }

    FTASuspensionRuntimeConfig BuildSuspensionConfig(
        const double SpringRateNPerM,
        const double StaticSpringCompressionM,
        const double BumpDampingNsPerM,
        const double ReboundDampingNsPerM,
        const double BumpStopRateNPerM,
        const double DroopStopRateNPerM,
        const double MinTravelM,
        const double MaxTravelM)
    {
        FTASuspensionRuntimeConfig Config;

        Config.SpringRateNPerM =
            SpringRateNPerM;

        Config.StaticSpringCompressionM =
            StaticSpringCompressionM;

        Config.BumpDampingNsPerM =
            BumpDampingNsPerM;

        Config.ReboundDampingNsPerM =
            ReboundDampingNsPerM;

        Config.BumpStopRateNPerM =
            BumpStopRateNPerM;

        Config.DroopStopRateNPerM =
            DroopStopRateNPerM;

        Config.DroopStopTravelM =
            MinTravelM;

        Config.BumpStopTravelM =
            MaxTravelM;

        return Config;
    }

    void CompileTire(
        const FTAPrototypeTireDefinition& Authored,
        FTATireRuntimeConfig& Out)
    {
        Out.UnloadedRadiusM =
            Authored.UnloadedRadiusM;

        Out.ReferenceLoadN =
            Authored.ReferenceLoadN;

        Out.DryPeakMu =
            Authored.DryPeakMu;

        Out.LoadSensitivityExponent =
            Authored.LoadSensitivityExponent;

        Out.LongitudinalStiffnessN =
            Authored.LongitudinalStiffnessN;

        Out.CorneringStiffnessNPerRad =
            Authored.CorneringStiffnessNPerRad;

        Out.CamberStiffnessNPerRad =
            Authored.CamberStiffnessNPerRad;

        Out.SaturationExponent =
            Authored.SaturationExponent;

        Out.PneumaticTrailM =
            Authored.PneumaticTrailM;

        Out.RollingResistanceCoefficient =
            Authored.RollingResistanceCoefficient;

        Out.ReferencePressureKPa =
            Authored.ReferencePressureKPa;

        Out.ReferencePressureTemperatureC =
            Authored.ReferencePressureTemperatureC;

        Out.RadialStiffnessNPerM =
            Authored.RadialStiffnessNPerM;

        Out.RadialProgressiveStiffnessNPerM2 =
            Authored.RadialProgressiveStiffnessNPerM2;

        Out.RadialDampingNsPerM =
            Authored.RadialDampingNsPerM;

        Out.MaxRadialDeflectionM =
            Authored.MaxRadialDeflectionM;

        Out.PressureRadialStiffnessExponent =
            Authored.PressureRadialStiffnessExponent;

        Out.NewTreadDepthMm =
            Authored.NewTreadDepthMm;

        Out.MinimumTreadDepthMm =
            Authored.MinimumTreadDepthMm;

        Out.OptimalSurfaceTemperatureC =
            Authored.OptimalSurfaceTemperatureC;

        Out.ColdGripMultiplier =
            Authored.ColdGripMultiplier;

        Out.HotGripMultiplier =
            Authored.HotGripMultiplier;

        Out.HotGripTemperatureC =
            Authored.HotGripTemperatureC;

        Out.PressureGripSensitivity =
            Authored.PressureGripSensitivity;

        Out.WearGripLossAtEnd =
            Authored.WearGripLossAtEnd;

        Out.SurfaceThermalMassJPerC =
            Authored.SurfaceThermalMassJPerC;

        Out.CarcassThermalMassJPerC =
            Authored.CarcassThermalMassJPerC;

        Out.SurfaceToCarcassConductanceWPerC =
            Authored.SurfaceToCarcassConductanceWPerC;

        Out.CarcassToAmbientConductanceWPerC =
            Authored.CarcassToAmbientConductanceWPerC;

        Out.InternalAirTimeConstantSeconds =
            Authored.InternalAirTimeConstantSeconds;

        Out.SlipHeatFraction =
            Authored.SlipHeatFraction;

        Out.RollingHeatFraction =
            Authored.RollingHeatFraction;

        Out.WearEnergyCapacityJ =
            Authored.WearEnergyCapacityJ;

        Out.ThermalDegradationStartC =
            Authored.ThermalDegradationStartC;

        Out.ThermalDegradationRatePerSecondAt170C =
            Authored.ThermalDegradationRatePerSecondAt170C;

        Out.SlipReferenceVelocityMps =
            Authored.SlipReferenceVelocityMps;

        Out.DynamicBlendStartMps =
            Authored.DynamicBlendStartMps;

        Out.DynamicBlendEndMps =
            Authored.DynamicBlendEndMps;

        Out.HydroReferenceOnsetSpeedMps =
            Authored.HydroReferenceOnsetSpeedMps;

        Out.HydroReferenceWaterDepthMm =
            Authored.HydroReferenceWaterDepthMm;
    }

    uint32 HashTireConfig(
        uint32 Hash,
        const FTATireRuntimeConfig& Tire)
    {
        Hash = HashDouble(Hash, Tire.UnloadedRadiusM);
        Hash = HashDouble(Hash, Tire.ReferenceLoadN);
        Hash = HashDouble(Hash, Tire.DryPeakMu);
        Hash = HashDouble(Hash, Tire.LoadSensitivityExponent);
        Hash = HashDouble(Hash, Tire.LongitudinalStiffnessN);
        Hash = HashDouble(Hash, Tire.CorneringStiffnessNPerRad);
        Hash = HashDouble(Hash, Tire.CamberStiffnessNPerRad);
        Hash = HashDouble(Hash, Tire.SaturationExponent);
        Hash = HashDouble(Hash, Tire.PneumaticTrailM);
        Hash = HashDouble(Hash, Tire.RollingResistanceCoefficient);
        Hash = HashDouble(Hash, Tire.ReferencePressureKPa);
        Hash = HashDouble(Hash, Tire.ReferencePressureTemperatureC);
        Hash = HashDouble(Hash, Tire.RadialStiffnessNPerM);
        Hash = HashDouble(Hash, Tire.RadialProgressiveStiffnessNPerM2);
        Hash = HashDouble(Hash, Tire.RadialDampingNsPerM);
        Hash = HashDouble(Hash, Tire.MaxRadialDeflectionM);
        Hash = HashDouble(Hash, Tire.PressureRadialStiffnessExponent);
        Hash = HashDouble(Hash, Tire.NewTreadDepthMm);
        Hash = HashDouble(Hash, Tire.MinimumTreadDepthMm);
        Hash = HashDouble(Hash, Tire.OptimalSurfaceTemperatureC);
        Hash = HashDouble(Hash, Tire.ColdGripMultiplier);
        Hash = HashDouble(Hash, Tire.HotGripMultiplier);
        Hash = HashDouble(Hash, Tire.HotGripTemperatureC);
        Hash = HashDouble(Hash, Tire.PressureGripSensitivity);
        Hash = HashDouble(Hash, Tire.WearGripLossAtEnd);
        Hash = HashDouble(Hash, Tire.SurfaceThermalMassJPerC);
        Hash = HashDouble(Hash, Tire.CarcassThermalMassJPerC);
        Hash = HashDouble(Hash, Tire.SurfaceToCarcassConductanceWPerC);
        Hash = HashDouble(Hash, Tire.CarcassToAmbientConductanceWPerC);
        Hash = HashDouble(Hash, Tire.InternalAirTimeConstantSeconds);
        Hash = HashDouble(Hash, Tire.SlipHeatFraction);
        Hash = HashDouble(Hash, Tire.RollingHeatFraction);
        Hash = HashDouble(Hash, Tire.WearEnergyCapacityJ);
        Hash = HashDouble(Hash, Tire.ThermalDegradationStartC);
        Hash = HashDouble(Hash, Tire.ThermalDegradationRatePerSecondAt170C);
        Hash = HashDouble(Hash, Tire.SlipReferenceVelocityMps);
        Hash = HashDouble(Hash, Tire.DynamicBlendStartMps);
        Hash = HashDouble(Hash, Tire.DynamicBlendEndMps);
        Hash = HashDouble(Hash, Tire.HydroReferenceOnsetSpeedMps);
        Hash = HashDouble(Hash, Tire.HydroReferenceWaterDepthMm);
        return Hash;
    }

    bool BuildFrontKinematicSamples(
        const FTADoubleWishboneSolverConfig& Geometry,
        TArray<FTASuspensionKinematicSample>& OutSamples)
    {
        constexpr int32 SampleCount = 17;

        if (!TADoubleWishboneSolver::ValidateConfig(
                Geometry))
        {
            return false;
        }

        TArray<FTADoubleWishboneSolveOutput> Solved;
        Solved.SetNum(SampleCount);

        OutSamples.SetNum(SampleCount);

        FTADoubleWishboneState State;

        const double TravelRangeM =
            Geometry.MaxTravelM
            - Geometry.MinTravelM;

        for (int32 Index = 0;
             Index < SampleCount;
             ++Index)
        {
            const double Alpha =
                static_cast<double>(Index)
                / static_cast<double>(SampleCount - 1);

            FTADoubleWishboneSolveInput Input;
            Input.TravelM =
                FMath::Lerp(
                    Geometry.MinTravelM,
                    Geometry.MaxTravelM,
                    Alpha);

            if (!TADoubleWishboneSolver::Solve(
                    Geometry,
                    Input,
                    State,
                    Solved[Index]))
            {
                OutSamples.Reset();
                return false;
            }

            FTASuspensionKinematicSample& Sample =
                OutSamples[Index];

            Sample.TravelM =
                Input.TravelM;

            Sample.WheelCenterOffsetM =
                Solved[Index].WheelCenterLocalM
                - Geometry.Hardpoints
                    .WheelCenterReference;

            Sample.CamberRad =
                Solved[Index].CamberRad;

            Sample.ToeRad =
                Solved[Index].ToeRad;
        }

        for (int32 Index = 0;
             Index < SampleCount;
             ++Index)
        {
            const int32 LowerIndex =
                FMath::Max(
                    0,
                    Index - 1);

            const int32 UpperIndex =
                FMath::Min(
                    SampleCount - 1,
                    Index + 1);

            const double TravelDeltaM =
                OutSamples[UpperIndex].TravelM
                - OutSamples[LowerIndex].TravelM;

            const double DamperDeltaM =
                Solved[UpperIndex].DamperLengthM
                - Solved[LowerIndex].DamperLengthM;

            OutSamples[Index].MotionRatio =
                FMath::Clamp(
                    FMath::Abs(
                        DamperDeltaM
                        / FMath::Max(
                            1.0e-9,
                            FMath::Abs(
                                TravelDeltaM))),
                    0.05,
                    3.0);
        }

        return
            TravelRangeM > 0.0;
    }

    bool BuildRearKinematicSamples(
        const FTAMultiLinkSolverConfig& Geometry,
        TArray<FTASuspensionKinematicSample>& OutSamples)
    {
        constexpr int32 SampleCount = 17;

        if (!TAMultiLinkSolver::ValidateConfig(
                Geometry))
        {
            return false;
        }

        TArray<FTAMultiLinkSolveOutput> Solved;
        Solved.SetNum(SampleCount);

        OutSamples.SetNum(SampleCount);

        FTAMultiLinkRuntimeState State;

        const double TravelRangeM =
            Geometry.MaxTravelM
            - Geometry.MinTravelM;

        for (int32 Index = 0;
             Index < SampleCount;
             ++Index)
        {
            const double Alpha =
                static_cast<double>(Index)
                / static_cast<double>(SampleCount - 1);

            FTAMultiLinkSolveInput Input;
            Input.TravelM =
                FMath::Lerp(
                    Geometry.MinTravelM,
                    Geometry.MaxTravelM,
                    Alpha);

            if (!TAMultiLinkSolver::Solve(
                    Geometry,
                    Input,
                    State,
                    Solved[Index]))
            {
                OutSamples.Reset();
                return false;
            }

            FTASuspensionKinematicSample& Sample =
                OutSamples[Index];

            Sample.TravelM =
                Input.TravelM;

            Sample.WheelCenterOffsetM =
                Solved[Index].WheelCenterLocalM
                - Geometry.WheelCenterReference;

            Sample.CamberRad =
                Solved[Index].CamberRad;

            Sample.ToeRad =
                Solved[Index].ToeRad;
        }

        for (int32 Index = 0;
             Index < SampleCount;
             ++Index)
        {
            const int32 LowerIndex =
                FMath::Max(
                    0,
                    Index - 1);

            const int32 UpperIndex =
                FMath::Min(
                    SampleCount - 1,
                    Index + 1);

            const double TravelDeltaM =
                OutSamples[UpperIndex].TravelM
                - OutSamples[LowerIndex].TravelM;

            const double DamperDeltaM =
                Solved[UpperIndex].DamperLengthM
                - Solved[LowerIndex].DamperLengthM;

            OutSamples[Index].MotionRatio =
                FMath::Clamp(
                    FMath::Abs(
                        DamperDeltaM
                        / FMath::Max(
                            1.0e-9,
                            FMath::Abs(
                                TravelDeltaM))),
                    0.05,
                    3.0);
        }

        return
            TravelRangeM > 0.0;
    }

    uint32 HashSuspensionConfig(
        uint32 Hash,
        const FTASuspensionRuntimeConfig& Suspension)
    {
        Hash = HashDouble(Hash, Suspension.SpringRateNPerM);
        Hash = HashDouble(Hash, Suspension.StaticSpringCompressionM);
        Hash = HashDouble(Hash, Suspension.BumpDampingNsPerM);
        Hash = HashDouble(Hash, Suspension.ReboundDampingNsPerM);
        Hash = HashDouble(Hash, Suspension.BumpStopTravelM);
        Hash = HashDouble(Hash, Suspension.DroopStopTravelM);
        Hash = HashDouble(Hash, Suspension.BumpStopRateNPerM);
        Hash = HashDouble(Hash, Suspension.DroopStopRateNPerM);

        Hash = HashCombineFast(
            Hash,
            GetTypeHash(
                Suspension.KinematicSamples.Num()));

        for (const FTASuspensionKinematicSample& Sample :
             Suspension.KinematicSamples)
        {
            Hash = HashDouble(
                Hash,
                Sample.TravelM);

            Hash = HashVector(
                Hash,
                Sample.WheelCenterOffsetM);

            Hash = HashDouble(
                Hash,
                Sample.CamberRad);

            Hash = HashDouble(
                Hash,
                Sample.ToeRad);

            Hash = HashDouble(
                Hash,
                Sample.MotionRatio);
        }

        return Hash;
    }

    uint32 HashFrontGeometry(
        uint32 Hash,
        const FTADoubleWishboneSolverConfig& Config)
    {
        const FTADoubleWishboneHardpoints& H =
            Config.Hardpoints;

        Hash = HashVector(Hash, H.UpperInnerA);
        Hash = HashVector(Hash, H.UpperInnerB);
        Hash = HashVector(Hash, H.LowerInnerA);
        Hash = HashVector(Hash, H.LowerInnerB);
        Hash = HashVector(Hash, H.TieRodInner);
        Hash = HashVector(Hash, H.DamperChassis);
        Hash = HashVector(Hash, H.DamperLowerArmReference);
        Hash = HashVector(Hash, H.UpperBallJointReference);
        Hash = HashVector(Hash, H.LowerBallJointReference);
        Hash = HashVector(Hash, H.TieRodOuterReference);
        Hash = HashVector(Hash, H.WheelCenterReference);
        Hash = HashDouble(Hash, Config.MinTravelM);
        Hash = HashDouble(Hash, Config.MaxTravelM);
        Hash = HashCombineFast(Hash, GetTypeHash(Config.MaxIterations));
        Hash = HashDouble(Hash, Config.PositionToleranceM);
        return Hash;
    }

    uint32 HashRearGeometry(
        uint32 Hash,
        const FTAMultiLinkSolverConfig& Config)
    {
        for (int32 Index = 0;
             Index < TARearMultiLinkCount;
             ++Index)
        {
            Hash = HashVector(
                Hash,
                Config.Links[Index].ChassisPickupReference);

            Hash = HashVector(
                Hash,
                Config.Links[Index].UprightPickupReference);
        }

        Hash = HashVector(
            Hash,
            Config.WheelCenterReference);

        Hash = HashVector(
            Hash,
            Config.DamperChassisReference);

        Hash = HashVector(
            Hash,
            Config.DamperUprightReference);

        Hash = HashDouble(Hash, Config.MinTravelM);
        Hash = HashDouble(Hash, Config.MaxTravelM);
        Hash = HashCombineFast(Hash, GetTypeHash(Config.MaxIterations));
        Hash = HashDouble(Hash, Config.PositionToleranceM);

        return Hash;
    }
    bool ValidateTireAuthoring(
        const FTAPrototypeTireDefinition& Tire,
        FTAValidationResult& OutValidation)
    {
        const bool bFinite =
            FMath::IsFinite(Tire.UnloadedRadiusM)
            && FMath::IsFinite(Tire.ReferenceLoadN)
            && FMath::IsFinite(Tire.DryPeakMu)
            && FMath::IsFinite(Tire.LoadSensitivityExponent)
            && FMath::IsFinite(Tire.LongitudinalStiffnessN)
            && FMath::IsFinite(Tire.CorneringStiffnessNPerRad)
            && FMath::IsFinite(Tire.CamberStiffnessNPerRad)
            && FMath::IsFinite(Tire.SaturationExponent)
            && FMath::IsFinite(Tire.PneumaticTrailM)
            && FMath::IsFinite(Tire.RollingResistanceCoefficient)
            && FMath::IsFinite(Tire.ReferencePressureKPa)
            && FMath::IsFinite(Tire.ReferencePressureTemperatureC)
            && FMath::IsFinite(Tire.RadialStiffnessNPerM)
            && FMath::IsFinite(Tire.RadialProgressiveStiffnessNPerM2)
            && FMath::IsFinite(Tire.RadialDampingNsPerM)
            && FMath::IsFinite(Tire.MaxRadialDeflectionM)
            && FMath::IsFinite(Tire.PressureRadialStiffnessExponent)
            && FMath::IsFinite(Tire.NewTreadDepthMm)
            && FMath::IsFinite(Tire.MinimumTreadDepthMm)
            && FMath::IsFinite(Tire.OptimalSurfaceTemperatureC)
            && FMath::IsFinite(Tire.ColdGripMultiplier)
            && FMath::IsFinite(Tire.HotGripMultiplier)
            && FMath::IsFinite(Tire.HotGripTemperatureC)
            && FMath::IsFinite(Tire.PressureGripSensitivity)
            && FMath::IsFinite(Tire.WearGripLossAtEnd)
            && FMath::IsFinite(Tire.SurfaceThermalMassJPerC)
            && FMath::IsFinite(Tire.CarcassThermalMassJPerC)
            && FMath::IsFinite(Tire.SurfaceToCarcassConductanceWPerC)
            && FMath::IsFinite(Tire.CarcassToAmbientConductanceWPerC)
            && FMath::IsFinite(Tire.InternalAirTimeConstantSeconds)
            && FMath::IsFinite(Tire.SlipHeatFraction)
            && FMath::IsFinite(Tire.RollingHeatFraction)
            && FMath::IsFinite(Tire.WearEnergyCapacityJ)
            && FMath::IsFinite(Tire.ThermalDegradationStartC)
            && FMath::IsFinite(Tire.ThermalDegradationRatePerSecondAt170C)
            && FMath::IsFinite(Tire.SlipReferenceVelocityMps)
            && FMath::IsFinite(Tire.DynamicBlendStartMps)
            && FMath::IsFinite(Tire.DynamicBlendEndMps)
            && FMath::IsFinite(Tire.HydroReferenceOnsetSpeedMps)
            && FMath::IsFinite(Tire.HydroReferenceWaterDepthMm);

        const bool bRangesValid =
            Tire.UnloadedRadiusM > 0.0
            && Tire.ReferenceLoadN > 0.0
            && Tire.DryPeakMu > 0.0
            && Tire.LoadSensitivityExponent >= 0.0
            && Tire.LongitudinalStiffnessN > 0.0
            && Tire.CorneringStiffnessNPerRad > 0.0
            && Tire.CamberStiffnessNPerRad >= 0.0
            && Tire.SaturationExponent > 0.0
            && Tire.PneumaticTrailM >= 0.0
            && Tire.RollingResistanceCoefficient >= 0.0
            && Tire.ReferencePressureKPa > 0.0
            && Tire.ReferencePressureTemperatureC > -273.15
            && Tire.RadialStiffnessNPerM > 0.0
            && Tire.RadialProgressiveStiffnessNPerM2 >= 0.0
            && Tire.RadialDampingNsPerM >= 0.0
            && Tire.MaxRadialDeflectionM > 0.0
            && Tire.PressureRadialStiffnessExponent >= 0.0
            && Tire.NewTreadDepthMm >= 0.0
            && Tire.MinimumTreadDepthMm >= 0.0
            && Tire.MinimumTreadDepthMm <= Tire.NewTreadDepthMm
            && Tire.ColdGripMultiplier > 0.0
            && Tire.HotGripMultiplier > 0.0
            && Tire.HotGripTemperatureC
                > Tire.OptimalSurfaceTemperatureC
            && Tire.PressureGripSensitivity >= 0.0
            && Tire.WearGripLossAtEnd >= 0.0
            && Tire.WearGripLossAtEnd <= 1.0
            && Tire.SurfaceThermalMassJPerC > 0.0
            && Tire.CarcassThermalMassJPerC > 0.0
            && Tire.SurfaceToCarcassConductanceWPerC >= 0.0
            && Tire.CarcassToAmbientConductanceWPerC >= 0.0
            && Tire.InternalAirTimeConstantSeconds > 0.0
            && Tire.SlipHeatFraction >= 0.0
            && Tire.SlipHeatFraction <= 1.0
            && Tire.RollingHeatFraction >= 0.0
            && Tire.RollingHeatFraction <= 1.0
            && Tire.WearEnergyCapacityJ > 0.0
            && Tire.ThermalDegradationRatePerSecondAt170C >= 0.0
            && Tire.SlipReferenceVelocityMps > 0.0
            && Tire.DynamicBlendStartMps >= 0.0
            && Tire.DynamicBlendEndMps
                > Tire.DynamicBlendStartMps
            && Tire.HydroReferenceOnsetSpeedMps > 0.0
            && Tire.HydroReferenceWaterDepthMm > 0.0;

        if (!bFinite || !bRangesValid)
        {
            AddValidation(
                OutValidation,
                ETAValidationSeverity::Error,
                TEXT("Vehicle.InvalidTireCalibration"),
                TEXT(
                    "Tire calibration contains non-finite values or invalid force, "
                    "thermal, tread, pressure, slip or hydro ranges."));

            return false;
        }

        return true;
    }

    bool ValidatePowertrainAuthoring(
        const FTAPrototypeDrivetrainDefinition& Powertrain,
        const FTAEngineThermalAuthoringDefinition& Thermal,
        FTAValidationResult& OutValidation)
    {
        bool bValid = true;

        const bool bEngineRanges =
            FMath::IsFinite(Powertrain.IdleRPM)
            && FMath::IsFinite(Powertrain.RedlineRPM)
            && FMath::IsFinite(Powertrain.LimiterRPM)
            && FMath::IsFinite(Powertrain.CrankInertiaKgm2)
            && FMath::IsFinite(Powertrain.FrictionConstantNm)
            && FMath::IsFinite(Powertrain.FrictionLinearNms)
            && FMath::IsFinite(Powertrain.FrictionQuadraticNms2)
            && FMath::IsFinite(Powertrain.StallRPM)
            && FMath::IsFinite(Powertrain.CombustionStartRPM)
            && FMath::IsFinite(Powertrain.StarterTorqueNm)
            && FMath::IsFinite(Powertrain.StarterMaxRPM)
            && FMath::IsFinite(Powertrain.IdleControlGainNmPerRPM)
            && FMath::IsFinite(Powertrain.MaxIdleControlTorqueNm)
            && Powertrain.IdleRPM > 0.0
            && Powertrain.RedlineRPM > Powertrain.IdleRPM
            && Powertrain.LimiterRPM >= Powertrain.RedlineRPM
            && Powertrain.CrankInertiaKgm2 > 0.0
            && Powertrain.FrictionConstantNm >= 0.0
            && Powertrain.FrictionLinearNms >= 0.0
            && Powertrain.FrictionQuadraticNms2 >= 0.0
            && Powertrain.StallRPM >= 0.0
            && Powertrain.CombustionStartRPM > Powertrain.StallRPM
            && Powertrain.CombustionStartRPM <= Powertrain.IdleRPM
            && Powertrain.StarterTorqueNm >= 0.0
            && Powertrain.StarterMaxRPM >= Powertrain.CombustionStartRPM
            && Powertrain.IdleControlGainNmPerRPM >= 0.0
            && Powertrain.MaxIdleControlTorqueNm >= 0.0;

        if (!bEngineRanges)
        {
            AddValidation(
                OutValidation,
                ETAValidationSeverity::Error,
                TEXT("Vehicle.InvalidEngineCalibration"),
                TEXT("Engine speed, friction, starter or idle-control calibration is invalid."));
            bValid = false;
        }

        if (Powertrain.TorqueCurve.Num() < 2)
        {
            AddValidation(
                OutValidation,
                ETAValidationSeverity::Error,
                TEXT("Vehicle.InvalidTorqueCurve"),
                TEXT("Engine torque curve requires at least two ordered points."));
            bValid = false;
        }
        else
        {
            double PreviousRPM = -1.0;

            for (const FTAEngineTorqueDefinitionPoint& Point :
                 Powertrain.TorqueCurve)
            {
                if (!FMath::IsFinite(Point.RPM)
                    || !FMath::IsFinite(Point.TorqueNm)
                    || Point.RPM < 0.0
                    || Point.TorqueNm < 0.0
                    || Point.RPM <= PreviousRPM)
                {
                    AddValidation(
                        OutValidation,
                        ETAValidationSeverity::Error,
                        TEXT("Vehicle.InvalidTorqueCurve"),
                        TEXT(
                            "Engine torque-curve RPM values must be strictly increasing "
                            "and all RPM/torque values must be finite and non-negative."));
                    bValid = false;
                    break;
                }

                PreviousRPM =
                    Point.RPM;
            }
        }

        const bool bClutchAndGearbox =
            FMath::IsFinite(Powertrain.ClutchMaxTorqueNm)
            && FMath::IsFinite(Powertrain.ClutchCouplingStiffnessNms)
            && FMath::IsFinite(Powertrain.ClutchThermalMassJPerC)
            && FMath::IsFinite(Powertrain.ClutchCoolingWPerC)
            && FMath::IsFinite(Powertrain.ClutchAmbientTemperatureC)
            && FMath::IsFinite(Powertrain.ClutchFadeStartTemperatureC)
            && FMath::IsFinite(Powertrain.ClutchFadeEndTemperatureC)
            && FMath::IsFinite(Powertrain.ClutchWearEnergyCapacityJ)
            && FMath::IsFinite(Powertrain.ReverseGearRatio)
            && FMath::IsFinite(Powertrain.FinalDriveRatio)
            && FMath::IsFinite(Powertrain.MechanicalEfficiency)
            && FMath::IsFinite(Powertrain.DrivelineTorsionalStiffnessNmPerRad)
            && FMath::IsFinite(Powertrain.DrivelineTorsionalDampingNmsPerRad)
            && Powertrain.ClutchMaxTorqueNm > 0.0
            && Powertrain.ClutchCouplingStiffnessNms >= 0.0
            && Powertrain.ClutchThermalMassJPerC > 0.0
            && Powertrain.ClutchCoolingWPerC >= 0.0
            && Powertrain.ClutchAmbientTemperatureC > -273.15
            && Powertrain.ClutchFadeEndTemperatureC
                > Powertrain.ClutchFadeStartTemperatureC
            && Powertrain.ClutchWearEnergyCapacityJ > 0.0
            && Powertrain.ReverseGearRatio > 0.0
            && Powertrain.FinalDriveRatio > 0.0
            && Powertrain.MechanicalEfficiency > 0.0
            && Powertrain.MechanicalEfficiency <= 1.0
            && Powertrain.DrivelineTorsionalStiffnessNmPerRad >= 0.0
            && Powertrain.DrivelineTorsionalDampingNmsPerRad >= 0.0;

        if (!bClutchAndGearbox
            || Powertrain.ForwardGearRatios.Num() <= 0)
        {
            AddValidation(
                OutValidation,
                ETAValidationSeverity::Error,
                TEXT("Vehicle.InvalidDrivetrainCalibration"),
                TEXT("Clutch, gearbox or driveline calibration is invalid."));
            bValid = false;
        }

        for (const double Ratio :
             Powertrain.ForwardGearRatios)
        {
            if (!FMath::IsFinite(Ratio)
                || Ratio <= 0.0)
            {
                AddValidation(
                    OutValidation,
                    ETAValidationSeverity::Error,
                    TEXT("Vehicle.InvalidForwardGear"),
                    TEXT("All forward gear ratios must be finite and positive."));
                bValid = false;
                break;
            }
        }

        const bool bThermal =
            FMath::IsFinite(Thermal.AmbientTemperatureC)
            && FMath::IsFinite(Thermal.InitialCoolantTemperatureC)
            && FMath::IsFinite(Thermal.EffectiveThermalMassJPerC)
            && FMath::IsFinite(Thermal.BaseHeatGenerationW)
            && FMath::IsFinite(Thermal.FullLoadAdditionalHeatW)
            && FMath::IsFinite(Thermal.CoolingCapacityWPerC)
            && FMath::IsFinite(Thermal.DerateStartTemperatureC)
            && FMath::IsFinite(Thermal.DerateFullTemperatureC)
            && FMath::IsFinite(Thermal.MinimumThermalTorqueFactor)
            && FMath::IsFinite(Thermal.DamageStartTemperatureC)
            && FMath::IsFinite(Thermal.DamageRatePerSecondAt150C)
            && Thermal.AmbientTemperatureC > -273.15
            && Thermal.InitialCoolantTemperatureC > -273.15
            && Thermal.EffectiveThermalMassJPerC > 0.0
            && Thermal.BaseHeatGenerationW >= 0.0
            && Thermal.FullLoadAdditionalHeatW >= 0.0
            && Thermal.CoolingCapacityWPerC >= 0.0
            && Thermal.DerateFullTemperatureC
                > Thermal.DerateStartTemperatureC
            && Thermal.MinimumThermalTorqueFactor >= 0.0
            && Thermal.MinimumThermalTorqueFactor <= 1.0
            && Thermal.DamageRatePerSecondAt150C >= 0.0;

        if (!bThermal)
        {
            AddValidation(
                OutValidation,
                ETAValidationSeverity::Error,
                TEXT("Vehicle.InvalidEngineThermalCalibration"),
                TEXT("Engine thermal/derate calibration is invalid."));
            bValid = false;
        }

        return bValid;
    }

    bool ValidateCoolingAuthoring(
        const FTACoolingSystemAuthoringDefinition& Cooling,
        FTAValidationResult& OutValidation)
    {
        const bool bValid =
            FMath::IsFinite(Cooling.InitialCoolantMassKg)
            && FMath::IsFinite(Cooling.PunctureThresholdEnergyJ)
            && FMath::IsFinite(Cooling.FullLeakEnergyJ)
            && FMath::IsFinite(Cooling.MaxLeakAreaMm2)
            && FMath::IsFinite(Cooling.LeakMassFlowKgPerSecPerMm2)
            && FMath::IsFinite(Cooling.MinimumAirflowEfficiency01)
            && Cooling.InitialCoolantMassKg > 0.0
            && Cooling.PunctureThresholdEnergyJ >= 0.0
            && Cooling.FullLeakEnergyJ
                > Cooling.PunctureThresholdEnergyJ
            && Cooling.MaxLeakAreaMm2 >= 0.0
            && Cooling.LeakMassFlowKgPerSecPerMm2 >= 0.0
            && Cooling.MinimumAirflowEfficiency01 >= 0.0
            && Cooling.MinimumAirflowEfficiency01 <= 1.0;

        if (!bValid)
        {
            AddValidation(
                OutValidation,
                ETAValidationSeverity::Error,
                TEXT("Vehicle.InvalidCoolingCalibration"),
                TEXT(
                    "Cooling/radiator calibration contains invalid coolant, "
                    "damage threshold, leak or airflow values."));

            return false;
        }

        return true;
    }

    void CompileDisplacementBinding(
        const FTAStructureDisplacementBindingAuthoringDefinition& Authored,
        FTAStructureDisplacementBinding& Out)
    {
        Out.NodeIndices =
            Authored.NodeIndices;

        Out.Weights =
            Authored.Weights;
    }

    bool HasBindingData(
        const FTAStructureDisplacementBindingAuthoringDefinition& Binding)
    {
        return
            Binding.NodeIndices.Num() > 0
            || Binding.Weights.Num() > 0;
    }

    bool HasAnyStructureBindingData(
        const FTAVehicleStructureAuthoringDefinition& Structure)
    {
        const FTADoubleWishboneStructuralBindingsAuthoringDefinition* FrontBindings[2] =
        {
            &Structure.FrontLeftSuspensionBindings,
            &Structure.FrontRightSuspensionBindings
        };

        for (const FTADoubleWishboneStructuralBindingsAuthoringDefinition* Front :
             FrontBindings)
        {
            if (HasBindingData(Front->UpperInnerA) ||
                HasBindingData(Front->UpperInnerB) ||
                HasBindingData(Front->LowerInnerA) ||
                HasBindingData(Front->LowerInnerB) ||
                HasBindingData(Front->TieRodInner) ||
                HasBindingData(Front->DamperChassis))
            {
                return true;
            }
        }

        const FTAMultiLinkStructuralBindingsAuthoringDefinition* RearBindings[2] =
        {
            &Structure.RearLeftSuspensionBindings,
            &Structure.RearRightSuspensionBindings
        };

        for (const FTAMultiLinkStructuralBindingsAuthoringDefinition* Rear :
             RearBindings)
        {
            if (HasBindingData(Rear->Link0) ||
                HasBindingData(Rear->Link1) ||
                HasBindingData(Rear->Link2) ||
                HasBindingData(Rear->Link3) ||
                HasBindingData(Rear->Link4) ||
                HasBindingData(Rear->DamperChassis))
            {
                return true;
            }
        }

        return false;
    }

    void CompileFrontStructuralBindings(
        const FTADoubleWishboneStructuralBindingsAuthoringDefinition& Authored,
        FTADoubleWishboneStructuralBindings& Out)
    {
        CompileDisplacementBinding(
            Authored.UpperInnerA,
            Out.UpperInnerA);

        CompileDisplacementBinding(
            Authored.UpperInnerB,
            Out.UpperInnerB);

        CompileDisplacementBinding(
            Authored.LowerInnerA,
            Out.LowerInnerA);

        CompileDisplacementBinding(
            Authored.LowerInnerB,
            Out.LowerInnerB);

        CompileDisplacementBinding(
            Authored.TieRodInner,
            Out.TieRodInner);

        CompileDisplacementBinding(
            Authored.DamperChassis,
            Out.DamperChassis);
    }

    void CompileRearStructuralBindings(
        const FTAMultiLinkStructuralBindingsAuthoringDefinition& Authored,
        FTAMultiLinkStructuralBindings& Out)
    {
        const FTAStructureDisplacementBindingAuthoringDefinition* AuthoredLinks[TARearMultiLinkCount] =
        {
            &Authored.Link0,
            &Authored.Link1,
            &Authored.Link2,
            &Authored.Link3,
            &Authored.Link4
        };

        for (int32 Index = 0;
             Index < TARearMultiLinkCount;
             ++Index)
        {
            CompileDisplacementBinding(
                *AuthoredLinks[Index],
                Out.ChassisPickups[Index]);
        }

        CompileDisplacementBinding(
            Authored.DamperChassis,
            Out.DamperChassis);
    }

    bool ValidateFrontStructuralBindings(
        const FTADoubleWishboneStructuralBindings& Bindings,
        const int32 NodeCount)
    {
        return
            TASuspensionDamageBinding::ValidateBinding(
                Bindings.UpperInnerA,
                NodeCount)
            && TASuspensionDamageBinding::ValidateBinding(
                Bindings.UpperInnerB,
                NodeCount)
            && TASuspensionDamageBinding::ValidateBinding(
                Bindings.LowerInnerA,
                NodeCount)
            && TASuspensionDamageBinding::ValidateBinding(
                Bindings.LowerInnerB,
                NodeCount)
            && TASuspensionDamageBinding::ValidateBinding(
                Bindings.TieRodInner,
                NodeCount)
            && TASuspensionDamageBinding::ValidateBinding(
                Bindings.DamperChassis,
                NodeCount);
    }

    bool ValidateRearStructuralBindings(
        const FTAMultiLinkStructuralBindings& Bindings,
        const int32 NodeCount)
    {
        for (int32 Index = 0;
             Index < TARearMultiLinkCount;
             ++Index)
        {
            if (!TASuspensionDamageBinding::ValidateBinding(
                    Bindings.ChassisPickups[Index],
                    NodeCount))
            {
                return false;
            }
        }

        return TASuspensionDamageBinding::ValidateBinding(
            Bindings.DamperChassis,
            NodeCount);
    }

    ETAVehicleDamageConsumerType CompileDamageConsumer(
        const ETAVehicleDamageConsumerAuthoringType Consumer)
    {
        switch (Consumer)
        {
        case ETAVehicleDamageConsumerAuthoringType::SteeringRack:
            return ETAVehicleDamageConsumerType::SteeringRack;

        case ETAVehicleDamageConsumerAuthoringType::WheelHub:
            return ETAVehicleDamageConsumerType::WheelHub;

        case ETAVehicleDamageConsumerAuthoringType::Radiator:
        default:
            return ETAVehicleDamageConsumerType::Radiator;
        }
    }

    bool CompileStructureRuntime(
        const FTAVehicleStructureAuthoringDefinition& Authored,
        const FVector3d& CenterOfMassVehicleLocalM,
        FTAVehicleStructureCompiledConfig& Out,
        FTAValidationResult& OutValidation)
    {
        Out =
            FTAVehicleStructureCompiledConfig{};

        const bool bHasDependentData =
            Authored.Constraints.Num() > 0
            || Authored.MountDamageBindings.Num() > 0
            || Authored.DamageRoutes.Num() > 0
            || HasAnyStructureBindingData(Authored);

        if (Authored.Nodes.Num() == 0)
        {
            if (bHasDependentData)
            {
                AddValidation(
                    OutValidation,
                    ETAValidationSeverity::Error,
                    TEXT("Vehicle.StructureMissingNodes"),
                    TEXT("Structural constraints, routes or bindings require authored structural nodes."));

                return false;
            }

            return true;
        }

        Out.Solver.ConstraintIterations =
            Authored.ConstraintIterations;

        Out.Solver.MaxPositionCorrectionM =
            Authored.MaxPositionCorrectionM;

        Out.Solver.MaxPlasticRestChangeFractionPerStep =
            Authored.MaxPlasticRestChangeFractionPerStep;

        Out.ImpactDistribution.DistributionRadiusM =
            Authored.ImpactDistributionRadiusM;

        Out.ImpactDistribution.DistanceFalloffExponent =
            Authored.ImpactDistanceFalloffExponent;

        Out.ImpactDistribution.DeformationImpulseFraction01 =
            Authored.DeformationImpulseFraction01;

        Out.ImpactDistribution.MaxDeformationEnergyFraction01 =
            Authored.MaxDeformationEnergyFraction01;

        Out.ImpactDistribution.MaxNodeDeltaVelocityMps =
            Authored.MaxNodeDeltaVelocityMps;

        Out.InitialNodes.SetNum(
            Authored.Nodes.Num());

        for (int32 NodeIndex = 0;
             NodeIndex < Authored.Nodes.Num();
             ++NodeIndex)
        {
            const FTAStructureNodeAuthoringDefinition& Source =
                Authored.Nodes[NodeIndex];

            if (Source.MassKg <= 0.0)
            {
                AddValidation(
                    OutValidation,
                    ETAValidationSeverity::Error,
                    TEXT("Vehicle.StructureInvalidNodeMass"),
                    FString::Printf(
                        TEXT("Structural node %d has non-positive mass."),
                        NodeIndex));

                continue;
            }

            FTAStructureNode& Node =
                Out.InitialNodes[NodeIndex];

            Node.PositionM =
                ToComLocal(
                    Source.PositionVehicleLocalM,
                    CenterOfMassVehicleLocalM);

            Node.ReferencePositionM =
                Node.PositionM;

            Node.PreviousPositionM =
                Node.PositionM;

            Node.VelocityMps =
                FVector3d::ZeroVector;

            Node.bPinned =
                Source.bPinned;

            Node.InverseMassPerKg =
                Source.bPinned
                ? 0.0
                : 1.0 / Source.MassKg;
        }

        Out.Constraints.SetNum(
            Authored.Constraints.Num());

        Out.DamageBridge.ConstraintTargetComponentIndices.SetNum(
            Authored.Constraints.Num());

        for (int32 ConstraintIndex = 0;
             ConstraintIndex < Authored.Constraints.Num();
             ++ConstraintIndex)
        {
            const FTAStructureConstraintAuthoringDefinition& Source =
                Authored.Constraints[ConstraintIndex];

            if (!Out.InitialNodes.IsValidIndex(Source.NodeA) ||
                !Out.InitialNodes.IsValidIndex(Source.NodeB) ||
                Source.NodeA == Source.NodeB)
            {
                AddValidation(
                    OutValidation,
                    ETAValidationSeverity::Error,
                    TEXT("Vehicle.StructureInvalidConstraintNodes"),
                    FString::Printf(
                        TEXT("Structural constraint %d references invalid or identical nodes."),
                        ConstraintIndex));

                continue;
            }

            FTADistanceConstraint& Constraint =
                Out.Constraints[ConstraintIndex];

            Constraint.NodeA =
                Source.NodeA;

            Constraint.NodeB =
                Source.NodeB;

            Constraint.RestLengthM =
                (Out.InitialNodes[Source.NodeB].PositionM
                    - Out.InitialNodes[Source.NodeA].PositionM).Length();

            if (Constraint.RestLengthM <= UE_DOUBLE_SMALL_NUMBER)
            {
                AddValidation(
                    OutValidation,
                    ETAValidationSeverity::Error,
                    TEXT("Vehicle.StructureZeroLengthConstraint"),
                    FString::Printf(
                        TEXT("Structural constraint %d has zero rest length."),
                        ConstraintIndex));
            }

            Constraint.Compliance =
                FMath::Max(
                    0.0,
                    Source.Compliance);

            Constraint.YieldStrain =
                FMath::Max(
                    0.0,
                    Source.YieldStrain);

            Constraint.FractureStrain =
                FMath::Max(
                    Constraint.YieldStrain,
                    Source.FractureStrain);

            Constraint.PlasticFlowRate01 =
                FMath::Clamp(
                    Source.PlasticFlowRate01,
                    0.0,
                    1.0);

            Out.DamageBridge.ConstraintTargetComponentIndices[ConstraintIndex] =
                Source.TargetComponentIndex;
        }

        Out.DamageBridge.ImpactTargetComponentIndex =
            Authored.ImpactTargetComponentIndex;

        for (const FTAStructureMountDamageAuthoringDefinition& Source :
             Authored.MountDamageBindings)
        {
            FTAStructureMountDamageBinding Binding;

            Binding.TargetComponentIndex =
                Source.TargetComponentIndex;

            Binding.NodeIndices =
                Source.NodeIndices;

            Binding.Weights =
                Source.Weights;

            Binding.DisplacementThresholdsM =
                Source.DisplacementThresholdsM;

            Out.DamageBridge.MountBindings.Add(
                MoveTemp(Binding));
        }

        for (const FTAVehicleDamageRouteAuthoringDefinition& Source :
             Authored.DamageRoutes)
        {
            FTAVehicleDamageRoute Route;

            Route.TargetComponentIndex =
                Source.TargetComponentIndex;

            Route.Consumer =
                CompileDamageConsumer(
                    Source.Consumer);

            Route.bAcceptImpactEnergy =
                Source.bAcceptImpactEnergy;

            Route.bAcceptStructuralDisplacement =
                Source.bAcceptStructuralDisplacement;

            Route.bAcceptStructuralFracture =
                Source.bAcceptStructuralFracture;

            Route.ImpactEnergyScale =
                Source.ImpactEnergyScale;

            Route.FullDamageEnergyJ =
                Source.FullDamageEnergyJ;

            Route.FullCrushDisplacementM =
                Source.FullCrushDisplacementM;

            Route.WheelIndex =
                Source.WheelIndex;

            Route.MinimumSteeringAuthority01 =
                Source.MinimumSteeringAuthority01;

            Route.MaximumSteeringFreePlayM =
                Source.MaximumSteeringFreePlayM;

            Route.MinimumBrakeEfficiency01 =
                Source.MinimumBrakeEfficiency01;

            Route.MinimumDriveEfficiency01 =
                Source.MinimumDriveEfficiency01;

            Route.MaximumBearingDragTorqueNm =
                Source.MaximumBearingDragTorqueNm;

            Out.DamageRouting.Routes.Add(
                Route);
        }

        CompileFrontStructuralBindings(
            Authored.FrontLeftSuspensionBindings,
            Out.FrontLeftSuspensionBindings);

        CompileFrontStructuralBindings(
            Authored.FrontRightSuspensionBindings,
            Out.FrontRightSuspensionBindings);

        CompileRearStructuralBindings(
            Authored.RearLeftSuspensionBindings,
            Out.RearLeftSuspensionBindings);

        CompileRearStructuralBindings(
            Authored.RearRightSuspensionBindings,
            Out.RearRightSuspensionBindings);

        const int32 NodeCount =
            Out.InitialNodes.Num();

        const bool bBindingsValid =
            ValidateFrontStructuralBindings(
                Out.FrontLeftSuspensionBindings,
                NodeCount)
            && ValidateFrontStructuralBindings(
                Out.FrontRightSuspensionBindings,
                NodeCount)
            && ValidateRearStructuralBindings(
                Out.RearLeftSuspensionBindings,
                NodeCount)
            && ValidateRearStructuralBindings(
                Out.RearRightSuspensionBindings,
                NodeCount);

        if (!bBindingsValid)
        {
            AddValidation(
                OutValidation,
                ETAValidationSeverity::Error,
                TEXT("Vehicle.StructureInvalidSuspensionBinding"),
                TEXT("One or more structural suspension bindings are invalid."));
        }

        if (!TAStructureDamageBridge::ValidateConfig(
                Out.DamageBridge,
                Out.InitialNodes.Num(),
                Out.Constraints.Num()))
        {
            AddValidation(
                OutValidation,
                ETAValidationSeverity::Error,
                TEXT("Vehicle.StructureInvalidDamageBridge"),
                TEXT("Structural damage-event bindings are invalid."));
        }

        if (!TAVehicleDamageRouter::ValidateConfig(
                Out.DamageRouting))
        {
            AddValidation(
                OutValidation,
                ETAValidationSeverity::Error,
                TEXT("Vehicle.StructureInvalidDamageRouting"),
                TEXT("Vehicle damage routing contains invalid or duplicate component targets."));
        }

        return !OutValidation.HasErrors();
    }

    bool CalculateBindingReferencePosition(
        const TArray<FTAStructureNode>& Nodes,
        const FTAStructureDisplacementBinding& Binding,
        FVector3d& OutPositionM)
    {
        OutPositionM =
            FVector3d::ZeroVector;

        if (Binding.NodeIndices.Num() == 0)
        {
            return false;
        }

        if (Binding.NodeIndices.Num()
            != Binding.Weights.Num())
        {
            return false;
        }

        double WeightSum = 0.0;

        for (int32 Index = 0;
             Index < Binding.NodeIndices.Num();
             ++Index)
        {
            const int32 NodeIndex =
                Binding.NodeIndices[Index];

            if (!Nodes.IsValidIndex(NodeIndex))
            {
                return false;
            }

            const double Weight =
                Binding.Weights[Index];

            OutPositionM +=
                Nodes[NodeIndex].ReferencePositionM
                * Weight;

            WeightSum +=
                Weight;
        }

        if (WeightSum <= UE_DOUBLE_SMALL_NUMBER)
        {
            return false;
        }

        OutPositionM /=
            WeightSum;

        return true;
    }

    void ValidateBindingAnchorLocation(
        const TArray<FTAStructureNode>& Nodes,
        const FTAStructureDisplacementBinding& Binding,
        const FVector3d& PhysicalPickupM,
        const FString& Label,
        FTAValidationResult& OutValidation)
    {
        if (Binding.NodeIndices.Num() == 0)
        {
            return;
        }

        FVector3d BoundReferencePositionM;

        if (!CalculateBindingReferencePosition(
                Nodes,
                Binding,
                BoundReferencePositionM))
        {
            return;
        }

        const double DistanceM =
            (BoundReferencePositionM
                - PhysicalPickupM).Length();

        constexpr double WarningDistanceM = 0.10;
        constexpr double ErrorDistanceM = 0.30;

        if (DistanceM > ErrorDistanceM)
        {
            AddValidation(
                OutValidation,
                ETAValidationSeverity::Error,
                TEXT("Vehicle.StructureBindingSpatialMismatch"),
                FString::Printf(
                    TEXT("%s structural binding is %.3f m from its physical suspension pickup."),
                    *Label,
                    DistanceM));
        }
        else if (DistanceM > WarningDistanceM)
        {
            AddValidation(
                OutValidation,
                ETAValidationSeverity::Warning,
                TEXT("Vehicle.StructureBindingSpatialWarning"),
                FString::Printf(
                    TEXT("%s structural binding centroid is %.3f m from its physical suspension pickup."),
                    *Label,
                    DistanceM));
        }
    }

    void ValidateStructureBindingLocations(
        const FTAVehicleStructureCompiledConfig& Structure,
        const FTADoubleWishboneSolverConfig& FrontRight,
        const FTAMultiLinkSolverConfig& RearRight,
        FTAValidationResult& OutValidation)
    {
        if (!Structure.HasStructure())
        {
            return;
        }

        const FTADoubleWishboneSolverConfig FrontLeft =
            TADoubleWishboneSolver::MirrorAcrossCenterline(
                FrontRight);

        const FTAMultiLinkSolverConfig RearLeft =
            TAMultiLinkSolver::MirrorAcrossCenterline(
                RearRight);

        const FTADoubleWishboneStructuralBindings* FrontBindings[2] =
        {
            &Structure.FrontLeftSuspensionBindings,
            &Structure.FrontRightSuspensionBindings
        };

        const FTADoubleWishboneSolverConfig* FrontGeometry[2] =
        {
            &FrontLeft,
            &FrontRight
        };

        const TCHAR* FrontSideNames[2] =
        {
            TEXT("FrontLeft"),
            TEXT("FrontRight")
        };

        for (int32 Side = 0; Side < 2; ++Side)
        {
            const FTADoubleWishboneStructuralBindings& B =
                *FrontBindings[Side];

            const FTADoubleWishboneHardpoints& H =
                FrontGeometry[Side]->Hardpoints;

            ValidateBindingAnchorLocation(
                Structure.InitialNodes,
                B.UpperInnerA,
                H.UpperInnerA,
                FString::Printf(TEXT("%s.UpperInnerA"), FrontSideNames[Side]),
                OutValidation);

            ValidateBindingAnchorLocation(
                Structure.InitialNodes,
                B.UpperInnerB,
                H.UpperInnerB,
                FString::Printf(TEXT("%s.UpperInnerB"), FrontSideNames[Side]),
                OutValidation);

            ValidateBindingAnchorLocation(
                Structure.InitialNodes,
                B.LowerInnerA,
                H.LowerInnerA,
                FString::Printf(TEXT("%s.LowerInnerA"), FrontSideNames[Side]),
                OutValidation);

            ValidateBindingAnchorLocation(
                Structure.InitialNodes,
                B.LowerInnerB,
                H.LowerInnerB,
                FString::Printf(TEXT("%s.LowerInnerB"), FrontSideNames[Side]),
                OutValidation);

            ValidateBindingAnchorLocation(
                Structure.InitialNodes,
                B.TieRodInner,
                H.TieRodInner,
                FString::Printf(TEXT("%s.TieRodInner"), FrontSideNames[Side]),
                OutValidation);

            ValidateBindingAnchorLocation(
                Structure.InitialNodes,
                B.DamperChassis,
                H.DamperChassis,
                FString::Printf(TEXT("%s.DamperChassis"), FrontSideNames[Side]),
                OutValidation);
        }

        const FTAMultiLinkStructuralBindings* RearBindings[2] =
        {
            &Structure.RearLeftSuspensionBindings,
            &Structure.RearRightSuspensionBindings
        };

        const FTAMultiLinkSolverConfig* RearGeometry[2] =
        {
            &RearLeft,
            &RearRight
        };

        const TCHAR* RearSideNames[2] =
        {
            TEXT("RearLeft"),
            TEXT("RearRight")
        };

        for (int32 Side = 0; Side < 2; ++Side)
        {
            const FTAMultiLinkStructuralBindings& B =
                *RearBindings[Side];

            const FTAMultiLinkSolverConfig& G =
                *RearGeometry[Side];

            for (int32 Link = 0;
                 Link < TARearMultiLinkCount;
                 ++Link)
            {
                ValidateBindingAnchorLocation(
                    Structure.InitialNodes,
                    B.ChassisPickups[Link],
                    G.Links[Link].ChassisPickupReference,
                    FString::Printf(
                        TEXT("%s.Link%d"),
                        RearSideNames[Side],
                        Link),
                    OutValidation);
            }

            ValidateBindingAnchorLocation(
                Structure.InitialNodes,
                B.DamperChassis,
                G.DamperChassisReference,
                FString::Printf(
                    TEXT("%s.DamperChassis"),
                    RearSideNames[Side]),
                OutValidation);
        }
    }

    uint32 HashDisplacementBinding(
        uint32 Hash,
        const FTAStructureDisplacementBinding& Binding)
    {
        Hash = HashCombineFast(
            Hash,
            GetTypeHash(
                Binding.NodeIndices.Num()));

        for (int32 Index = 0;
             Index < Binding.NodeIndices.Num();
             ++Index)
        {
            Hash = HashCombineFast(
                Hash,
                GetTypeHash(
                    Binding.NodeIndices[Index]));

            if (Binding.Weights.IsValidIndex(Index))
            {
                Hash = HashDouble(
                    Hash,
                    Binding.Weights[Index]);
            }
        }

        return Hash;
    }

    uint32 HashFrontStructuralBindings(
        uint32 Hash,
        const FTADoubleWishboneStructuralBindings& Bindings)
    {
        Hash = HashDisplacementBinding(
            Hash,
            Bindings.UpperInnerA);

        Hash = HashDisplacementBinding(
            Hash,
            Bindings.UpperInnerB);

        Hash = HashDisplacementBinding(
            Hash,
            Bindings.LowerInnerA);

        Hash = HashDisplacementBinding(
            Hash,
            Bindings.LowerInnerB);

        Hash = HashDisplacementBinding(
            Hash,
            Bindings.TieRodInner);

        Hash = HashDisplacementBinding(
            Hash,
            Bindings.DamperChassis);

        return Hash;
    }

    uint32 HashRearStructuralBindings(
        uint32 Hash,
        const FTAMultiLinkStructuralBindings& Bindings)
    {
        for (int32 Index = 0;
             Index < TARearMultiLinkCount;
             ++Index)
        {
            Hash = HashDisplacementBinding(
                Hash,
                Bindings.ChassisPickups[Index]);
        }

        Hash = HashDisplacementBinding(
            Hash,
            Bindings.DamperChassis);

        return Hash;
    }

    uint32 HashStructureRuntime(
        uint32 Hash,
        const FTAVehicleStructureCompiledConfig& Structure)
    {
        Hash = HashVector(
            Hash,
            Structure.Solver.GravityMps2);

        Hash = HashCombineFast(
            Hash,
            GetTypeHash(
                Structure.Solver.ConstraintIterations));

        Hash = HashDouble(
            Hash,
            Structure.Solver.MaxPositionCorrectionM);

        Hash = HashDouble(
            Hash,
            Structure.Solver.MaxPlasticRestChangeFractionPerStep);

        Hash = HashCombineFast(
            Hash,
            GetTypeHash(Structure.InitialNodes.Num()));

        Hash = HashCombineFast(
            Hash,
            GetTypeHash(Structure.Constraints.Num()));

        Hash = HashDouble(
            Hash,
            Structure.ImpactDistribution.DistributionRadiusM);

        Hash = HashDouble(
            Hash,
            Structure.ImpactDistribution.DistanceFalloffExponent);

        Hash = HashDouble(
            Hash,
            Structure.ImpactDistribution.DeformationImpulseFraction01);

        Hash = HashDouble(
            Hash,
            Structure.ImpactDistribution.MaxDeformationEnergyFraction01);

        Hash = HashDouble(
            Hash,
            Structure.ImpactDistribution.MaxNodeDeltaVelocityMps);

        for (const FTAStructureNode& Node :
             Structure.InitialNodes)
        {
            Hash = HashVector(
                Hash,
                Node.ReferencePositionM);

            Hash = HashDouble(
                Hash,
                Node.InverseMassPerKg);

            Hash = HashBool(
                Hash,
                Node.bPinned);
        }

        for (const FTADistanceConstraint& Constraint :
             Structure.Constraints)
        {
            Hash = HashCombineFast(
                Hash,
                GetTypeHash(Constraint.NodeA));

            Hash = HashCombineFast(
                Hash,
                GetTypeHash(Constraint.NodeB));

            Hash = HashDouble(
                Hash,
                Constraint.RestLengthM);

            Hash = HashDouble(
                Hash,
                Constraint.Compliance);

            Hash = HashDouble(
                Hash,
                Constraint.YieldStrain);

            Hash = HashDouble(
                Hash,
                Constraint.FractureStrain);

            Hash = HashDouble(
                Hash,
                Constraint.PlasticFlowRate01);
        }

        Hash = HashCombineFast(
            Hash,
            GetTypeHash(
                Structure.DamageBridge.ImpactTargetComponentIndex));

        Hash = HashCombineFast(
            Hash,
            GetTypeHash(
                Structure.DamageBridge.ConstraintTargetComponentIndices.Num()));

        for (const int32 TargetComponentIndex :
             Structure.DamageBridge.ConstraintTargetComponentIndices)
        {
            Hash = HashCombineFast(
                Hash,
                GetTypeHash(
                    TargetComponentIndex));
        }

        Hash = HashCombineFast(
            Hash,
            GetTypeHash(
                Structure.DamageBridge.MountBindings.Num()));

        for (const FTAStructureMountDamageBinding& Binding :
             Structure.DamageBridge.MountBindings)
        {
            Hash = HashCombineFast(
                Hash,
                GetTypeHash(
                    Binding.TargetComponentIndex));

            Hash = HashCombineFast(
                Hash,
                GetTypeHash(
                    Binding.NodeIndices.Num()));

            for (int32 Index = 0;
                 Index < Binding.NodeIndices.Num();
                 ++Index)
            {
                Hash = HashCombineFast(
                    Hash,
                    GetTypeHash(
                        Binding.NodeIndices[Index]));

                if (Binding.Weights.IsValidIndex(Index))
                {
                    Hash = HashDouble(
                        Hash,
                        Binding.Weights[Index]);
                }
            }

            Hash = HashCombineFast(
                Hash,
                GetTypeHash(
                    Binding.DisplacementThresholdsM.Num()));

            for (const double ThresholdM :
                 Binding.DisplacementThresholdsM)
            {
                Hash = HashDouble(
                    Hash,
                    ThresholdM);
            }
        }

        for (const FTAVehicleDamageRoute& Route :
             Structure.DamageRouting.Routes)
        {
            Hash = HashCombineFast(
                Hash,
                GetTypeHash(Route.TargetComponentIndex));

            Hash = HashCombineFast(
                Hash,
                GetTypeHash(
                    static_cast<uint8>(Route.Consumer)));

            Hash = HashBool(
                Hash,
                Route.bAcceptImpactEnergy);

            Hash = HashBool(
                Hash,
                Route.bAcceptStructuralDisplacement);

            Hash = HashBool(
                Hash,
                Route.bAcceptStructuralFracture);

            Hash = HashDouble(
                Hash,
                Route.ImpactEnergyScale);

            Hash = HashDouble(
                Hash,
                Route.FullDamageEnergyJ);

            Hash = HashDouble(
                Hash,
                Route.FullCrushDisplacementM);

            Hash = HashCombineFast(
                Hash,
                GetTypeHash(Route.WheelIndex));

            Hash = HashDouble(
                Hash,
                Route.MinimumSteeringAuthority01);

            Hash = HashDouble(
                Hash,
                Route.MaximumSteeringFreePlayM);

            Hash = HashDouble(
                Hash,
                Route.MinimumBrakeEfficiency01);

            Hash = HashDouble(
                Hash,
                Route.MinimumDriveEfficiency01);

            Hash = HashDouble(
                Hash,
                Route.MaximumBearingDragTorqueNm);
        }

        Hash = HashFrontStructuralBindings(
            Hash,
            Structure.FrontLeftSuspensionBindings);

        Hash = HashFrontStructuralBindings(
            Hash,
            Structure.FrontRightSuspensionBindings);

        Hash = HashRearStructuralBindings(
            Hash,
            Structure.RearLeftSuspensionBindings);

        Hash = HashRearStructuralBindings(
            Hash,
            Structure.RearRightSuspensionBindings);

        return Hash;
    }

}

FTARearSuspensionDefinition::FTARearSuspensionDefinition()
{
    Link0.ChassisPickup =
        FVector(-1.15, 0.35, -0.18);
    Link0.UprightPickup =
        FVector(-1.20, 0.68, -0.24);

    Link1.ChassisPickup =
        FVector(-1.50, 0.36, -0.20);
    Link1.UprightPickup =
        FVector(-1.42, 0.69, -0.25);

    Link2.ChassisPickup =
        FVector(-1.10, 0.34, -0.46);
    Link2.UprightPickup =
        FVector(-1.18, 0.72, -0.49);

    Link3.ChassisPickup =
        FVector(-1.53, 0.33, -0.47);
    Link3.UprightPickup =
        FVector(-1.45, 0.73, -0.50);

    Link4.ChassisPickup =
        FVector(-1.50, 0.38, -0.36);
    Link4.UprightPickup =
        FVector(-1.48, 0.71, -0.38);
}

FTAPrototypeDrivetrainDefinition::FTAPrototypeDrivetrainDefinition()
{
    const double TorqueRPM[] =
    {
        1000.0,
        2000.0,
        3000.0,
        4000.0,
        5000.0,
        6000.0,
        7000.0
    };

    const double TorqueNm[] =
    {
        150.0,
        310.0,
        400.0,
        410.0,
        405.0,
        365.0,
        300.0
    };

    TorqueCurve.SetNum(
        UE_ARRAY_COUNT(TorqueRPM));

    for (int32 Index = 0;
         Index < UE_ARRAY_COUNT(TorqueRPM);
         ++Index)
    {
        TorqueCurve[Index].RPM =
            TorqueRPM[Index];

        TorqueCurve[Index].TorqueNm =
            TorqueNm[Index];
    }

    ForwardGearRatios =
    {
        3.45,
        2.15,
        1.52,
        1.16,
        0.92,
        0.76
    };
}

bool UTAVehicleDefinition::BuildCompiledConfig(
    FTAVehicleCompiledConfig& OutConfig,
    FTAValidationResult& OutValidation) const
{
    OutConfig =
        FTAVehicleCompiledConfig{};

    OutValidation.Messages.Reset();

    if (DefinitionId.IsNone())
    {
        AddValidation(
            OutValidation,
            ETAValidationSeverity::Error,
            TEXT("Vehicle.MissingDefinitionId"),
            TEXT("Vehicle DefinitionId must not be None."));
    }

    if (Mass.ReferenceMassKg <= 0.0)
    {
        AddValidation(
            OutValidation,
            ETAValidationSeverity::Error,
            TEXT("Vehicle.InvalidMass"),
            TEXT("Reference mass must be greater than zero."));
    }

    if (Mass.PrincipalInertiaKgm2.X <= 0.0 ||
        Mass.PrincipalInertiaKgm2.Y <= 0.0 ||
        Mass.PrincipalInertiaKgm2.Z <= 0.0)
    {
        AddValidation(
            OutValidation,
            ETAValidationSeverity::Error,
            TEXT("Vehicle.InvalidInertia"),
            TEXT("All principal inertia values must be greater than zero."));
    }

    if (Dimensions.WheelbaseMeters <= 0.0 ||
        Dimensions.TrackFrontMeters <= 0.0 ||
        Dimensions.TrackRearMeters <= 0.0)
    {
        AddValidation(
            OutValidation,
            ETAValidationSeverity::Error,
            TEXT("Vehicle.InvalidGeometry"),
            TEXT("Wheelbase and track widths must be greater than zero."));
    }

    if (WheelCount != 4)
    {
        AddValidation(
            OutValidation,
            ETAValidationSeverity::Error,
            TEXT("Vehicle.PrototypeRequiresFourWheels"),
            TEXT("The current high-fidelity compiled runtime requires exactly four wheels."));
    }

    ValidateTireAuthoring(
        Tire,
        OutValidation);

    if (FrontSuspension.MaxTravelM <=
            FrontSuspension.MinTravelM ||
        RearSuspension.MaxTravelM <=
            RearSuspension.MinTravelM)
    {
        AddValidation(
            OutValidation,
            ETAValidationSeverity::Error,
            TEXT("Vehicle.InvalidSuspensionTravel"),
            TEXT("Suspension maximum travel must be greater than minimum travel."));
    }

    ValidatePowertrainAuthoring(
        Drivetrain,
        EngineThermal,
        OutValidation);

    ValidateCoolingAuthoring(
        Cooling,
        OutValidation);

    const bool bSuspensionForceCalibrationValid =
        FMath::IsFinite(FrontSuspension.SpringRateNPerM)
        && FrontSuspension.SpringRateNPerM >= 0.0
        && FMath::IsFinite(FrontSuspension.StaticSpringCompressionM)
        && FrontSuspension.StaticSpringCompressionM >= 0.0
        && FMath::IsFinite(FrontSuspension.BumpDampingNsPerM)
        && FrontSuspension.BumpDampingNsPerM >= 0.0
        && FMath::IsFinite(FrontSuspension.ReboundDampingNsPerM)
        && FrontSuspension.ReboundDampingNsPerM >= 0.0
        && FMath::IsFinite(FrontSuspension.BumpStopRateNPerM)
        && FrontSuspension.BumpStopRateNPerM >= 0.0
        && FMath::IsFinite(FrontSuspension.DroopStopRateNPerM)
        && FrontSuspension.DroopStopRateNPerM >= 0.0
        && FMath::IsFinite(RearSuspension.SpringRateNPerM)
        && RearSuspension.SpringRateNPerM >= 0.0
        && FMath::IsFinite(RearSuspension.StaticSpringCompressionM)
        && RearSuspension.StaticSpringCompressionM >= 0.0
        && FMath::IsFinite(RearSuspension.BumpDampingNsPerM)
        && RearSuspension.BumpDampingNsPerM >= 0.0
        && FMath::IsFinite(RearSuspension.ReboundDampingNsPerM)
        && RearSuspension.ReboundDampingNsPerM >= 0.0
        && FMath::IsFinite(RearSuspension.BumpStopRateNPerM)
        && RearSuspension.BumpStopRateNPerM >= 0.0
        && FMath::IsFinite(RearSuspension.DroopStopRateNPerM)
        && RearSuspension.DroopStopRateNPerM >= 0.0;

    if (!bSuspensionForceCalibrationValid)
    {
        AddValidation(
            OutValidation,
            ETAValidationSeverity::Error,
            TEXT("Vehicle.InvalidSuspensionForceCalibration"),
            TEXT("Suspension spring, damper or stop calibration is invalid."));
    }

    if (OutValidation.HasErrors())
    {
        return false;
    }

    OutConfig.Version =
        Version;

    OutConfig.DefinitionId =
        DefinitionId;

    OutConfig.MassKg =
        Mass.ReferenceMassKg;

    OutConfig.CenterOfMassMeters =
        FVector3d(Mass.CenterOfMassMeters);

    OutConfig.PrincipalInertiaKgm2 =
        FVector3d(Mass.PrincipalInertiaKgm2);

    OutConfig.LengthMeters =
        Dimensions.LengthMeters;

    OutConfig.WidthMeters =
        Dimensions.WidthMeters;

    OutConfig.HeightMeters =
        Dimensions.HeightMeters;

    OutConfig.WheelbaseMeters =
        Dimensions.WheelbaseMeters;

    OutConfig.TrackFrontMeters =
        Dimensions.TrackFrontMeters;

    OutConfig.TrackRearMeters =
        Dimensions.TrackRearMeters;

    OutConfig.WheelCount =
        WheelCount;

    FTAVehicleRuntimeConfig& VehicleRuntime =
        OutConfig.VehicleRuntime;

    VehicleRuntime.ReferenceMassKg =
        Mass.ReferenceMassKg;

    VehicleRuntime.Chassis.MassKg =
        Mass.ReferenceMassKg;

    VehicleRuntime.Chassis.PrincipalInertiaBodyKgm2 =
        FVector3d(Mass.PrincipalInertiaKgm2);

    VehicleRuntime.Wheels.SetNum(4);
    VehicleRuntime.Tires.SetNum(4);

    FTATireRuntimeConfig TireRuntime;
    CompileTire(
        Tire,
        TireRuntime);

    for (int32 Index = 0;
         Index < 4;
         ++Index)
    {
        VehicleRuntime.Tires[Index] =
            TireRuntime;

        VehicleRuntime.Wheels[Index].RadiusM =
            TireRuntime.UnloadedRadiusM;

        VehicleRuntime.Wheels[Index].InertiaKgm2 =
            Wheel.InertiaKgm2;

        VehicleRuntime.Wheels[Index].MaxBrakeTorqueNm =
            Wheel.MaxBrakeTorqueNm;
    }

    const bool bRearDriven =
        Drivetrain.bRearWheelDrive;

    VehicleRuntime.Wheels[
        static_cast<int32>(
            ETAPrototypeWheelIndex::FrontLeft)].bDriven =
        !bRearDriven;

    VehicleRuntime.Wheels[
        static_cast<int32>(
            ETAPrototypeWheelIndex::FrontRight)].bDriven =
        !bRearDriven;

    VehicleRuntime.Wheels[
        static_cast<int32>(
            ETAPrototypeWheelIndex::RearLeft)].bDriven =
        bRearDriven;

    VehicleRuntime.Wheels[
        static_cast<int32>(
            ETAPrototypeWheelIndex::RearRight)].bDriven =
        bRearDriven;

    VehicleRuntime.Engine.IdleRPM =
        Drivetrain.IdleRPM;

    VehicleRuntime.Engine.RedlineRPM =
        Drivetrain.RedlineRPM;

    VehicleRuntime.Engine.LimiterRPM =
        Drivetrain.LimiterRPM;

    VehicleRuntime.Engine.CrankInertiaKgm2 =
        Drivetrain.CrankInertiaKgm2;

    VehicleRuntime.Engine.FrictionConstantNm =
        Drivetrain.FrictionConstantNm;

    VehicleRuntime.Engine.FrictionLinearNms =
        Drivetrain.FrictionLinearNms;

    VehicleRuntime.Engine.FrictionQuadraticNms2 =
        Drivetrain.FrictionQuadraticNms2;

    VehicleRuntime.Engine.StallRPM =
        Drivetrain.StallRPM;

    VehicleRuntime.Engine.CombustionStartRPM =
        Drivetrain.CombustionStartRPM;

    VehicleRuntime.Engine.StarterTorqueNm =
        Drivetrain.StarterTorqueNm;

    VehicleRuntime.Engine.StarterMaxRPM =
        Drivetrain.StarterMaxRPM;

    VehicleRuntime.Engine.IdleControlGainNmPerRPM =
        Drivetrain.IdleControlGainNmPerRPM;

    VehicleRuntime.Engine.MaxIdleControlTorqueNm =
        Drivetrain.MaxIdleControlTorqueNm;

    VehicleRuntime.Engine.TorqueCurve.Reset(
        Drivetrain.TorqueCurve.Num());

    for (const FTAEngineTorqueDefinitionPoint& Point :
         Drivetrain.TorqueCurve)
    {
        FTAEngineTorquePoint RuntimePoint;
        RuntimePoint.RPM =
            Point.RPM;
        RuntimePoint.TorqueNm =
            Point.TorqueNm;

        VehicleRuntime.Engine.TorqueCurve.Add(
            RuntimePoint);
    }

    VehicleRuntime.EngineThermal.AmbientTemperatureC =
        EngineThermal.AmbientTemperatureC;

    VehicleRuntime.EngineThermal.InitialCoolantTemperatureC =
        EngineThermal.InitialCoolantTemperatureC;

    VehicleRuntime.EngineThermal.EffectiveThermalMassJPerC =
        EngineThermal.EffectiveThermalMassJPerC;

    VehicleRuntime.EngineThermal.BaseHeatGenerationW =
        EngineThermal.BaseHeatGenerationW;

    VehicleRuntime.EngineThermal.FullLoadAdditionalHeatW =
        EngineThermal.FullLoadAdditionalHeatW;

    VehicleRuntime.EngineThermal.CoolingCapacityWPerC =
        EngineThermal.CoolingCapacityWPerC;

    VehicleRuntime.EngineThermal.DerateStartTemperatureC =
        EngineThermal.DerateStartTemperatureC;

    VehicleRuntime.EngineThermal.DerateFullTemperatureC =
        EngineThermal.DerateFullTemperatureC;

    VehicleRuntime.EngineThermal.MinimumThermalTorqueFactor =
        EngineThermal.MinimumThermalTorqueFactor;

    VehicleRuntime.EngineThermal.DamageStartTemperatureC =
        EngineThermal.DamageStartTemperatureC;

    VehicleRuntime.EngineThermal.DamageRatePerSecondAt150C =
        EngineThermal.DamageRatePerSecondAt150C;

    VehicleRuntime.Radiator.InitialCoolantMassKg =
        Cooling.InitialCoolantMassKg;

    VehicleRuntime.Radiator.PunctureThresholdEnergyJ =
        Cooling.PunctureThresholdEnergyJ;

    VehicleRuntime.Radiator.FullLeakEnergyJ =
        Cooling.FullLeakEnergyJ;

    VehicleRuntime.Radiator.MaxLeakAreaMm2 =
        Cooling.MaxLeakAreaMm2;

    VehicleRuntime.Radiator.LeakMassFlowKgPerSecPerMm2 =
        Cooling.LeakMassFlowKgPerSecPerMm2;

    VehicleRuntime.Radiator.MinimumAirflowEfficiency01 =
        Cooling.MinimumAirflowEfficiency01;

    VehicleRuntime.Clutch.MaxTorqueCapacityNm =
        Drivetrain.ClutchMaxTorqueNm;

    VehicleRuntime.Clutch.CouplingStiffnessNms =
        Drivetrain.ClutchCouplingStiffnessNms;

    VehicleRuntime.Clutch.ThermalMassJPerC =
        Drivetrain.ClutchThermalMassJPerC;

    VehicleRuntime.Clutch.CoolingWPerC =
        Drivetrain.ClutchCoolingWPerC;

    VehicleRuntime.Clutch.AmbientTemperatureC =
        Drivetrain.ClutchAmbientTemperatureC;

    VehicleRuntime.Clutch.FadeStartTemperatureC =
        Drivetrain.ClutchFadeStartTemperatureC;

    VehicleRuntime.Clutch.FadeEndTemperatureC =
        Drivetrain.ClutchFadeEndTemperatureC;

    VehicleRuntime.Clutch.WearEnergyCapacityJ =
        Drivetrain.ClutchWearEnergyCapacityJ;

    VehicleRuntime.Driveline.TorsionalStiffnessNmPerRad =
        Drivetrain.DrivelineTorsionalStiffnessNmPerRad;

    VehicleRuntime.Driveline.TorsionalDampingNmsPerRad =
        Drivetrain.DrivelineTorsionalDampingNmsPerRad;

    VehicleRuntime.Gearbox.ForwardGearRatios =
        Drivetrain.ForwardGearRatios;

    VehicleRuntime.Gearbox.ReverseGearRatio =
        Drivetrain.ReverseGearRatio;

    VehicleRuntime.Gearbox.FinalDriveRatio =
        Drivetrain.FinalDriveRatio;

    VehicleRuntime.Gearbox.MechanicalEfficiency =
        Drivetrain.MechanicalEfficiency;

    const FVector3d CenterOfMassVehicleLocalM =
        OutConfig.CenterOfMassMeters;

    FTAFourWheelRuntimeConfig& FourWheel =
        OutConfig.FourWheelRuntime;

    FTADoubleWishboneSolverConfig& FrontGeometry =
        FourWheel.FrontAxle.RightGeometry;

    FTADoubleWishboneHardpoints& FrontHardpoints =
        FrontGeometry.Hardpoints;

    FrontHardpoints.UpperInnerA =
        ToComLocal(
            FrontSuspension.UpperInnerA,
            CenterOfMassVehicleLocalM);

    FrontHardpoints.UpperInnerB =
        ToComLocal(
            FrontSuspension.UpperInnerB,
            CenterOfMassVehicleLocalM);

    FrontHardpoints.LowerInnerA =
        ToComLocal(
            FrontSuspension.LowerInnerA,
            CenterOfMassVehicleLocalM);

    FrontHardpoints.LowerInnerB =
        ToComLocal(
            FrontSuspension.LowerInnerB,
            CenterOfMassVehicleLocalM);

    FrontHardpoints.TieRodInner =
        ToComLocal(
            FrontSuspension.TieRodInner,
            CenterOfMassVehicleLocalM);

    FrontHardpoints.DamperChassis =
        ToComLocal(
            FrontSuspension.DamperChassis,
            CenterOfMassVehicleLocalM);

    FrontHardpoints.DamperLowerArmReference =
        ToComLocal(
            FrontSuspension.DamperLowerArm,
            CenterOfMassVehicleLocalM);

    FrontHardpoints.UpperBallJointReference =
        ToComLocal(
            FrontSuspension.UpperBallJoint,
            CenterOfMassVehicleLocalM);

    FrontHardpoints.LowerBallJointReference =
        ToComLocal(
            FrontSuspension.LowerBallJoint,
            CenterOfMassVehicleLocalM);

    FrontHardpoints.TieRodOuterReference =
        ToComLocal(
            FrontSuspension.TieRodOuter,
            CenterOfMassVehicleLocalM);

    FrontHardpoints.WheelCenterReference =
        ToComLocal(
            FrontSuspension.WheelCenter,
            CenterOfMassVehicleLocalM);

    FrontHardpoints.WheelForwardReference =
        FVector3d(1.0, 0.0, 0.0);

    FrontHardpoints.WheelUpReference =
        FVector3d(0.0, 0.0, 1.0);

    FrontHardpoints.SteeringRackAxisLocal =
        FVector3d(0.0, 1.0, 0.0);

    FrontHardpoints.SideSign = 1.0;

    FrontGeometry.MinTravelM =
        FrontSuspension.MinTravelM;

    FrontGeometry.MaxTravelM =
        FrontSuspension.MaxTravelM;

    FrontGeometry.MaxIterations =
        FrontSuspension.MaxSolverIterations;

    FrontGeometry.PositionToleranceM =
        FrontSuspension.PositionToleranceM;

    FourWheel.FrontAxle.LeftSuspension =
        BuildSuspensionConfig(
            FrontSuspension.SpringRateNPerM,
            FrontSuspension.StaticSpringCompressionM,
            FrontSuspension.BumpDampingNsPerM,
            FrontSuspension.ReboundDampingNsPerM,
            FrontSuspension.BumpStopRateNPerM,
            FrontSuspension.DroopStopRateNPerM,
            FrontSuspension.MinTravelM,
            FrontSuspension.MaxTravelM);

    FourWheel.FrontAxle.RightSuspension =
        FourWheel.FrontAxle.LeftSuspension;

    const FTADoubleWishboneSolverConfig FrontLeftGeometryForCache =
        TADoubleWishboneSolver::MirrorAcrossCenterline(
            FrontGeometry);

    const bool bFrontKinematicCacheBuilt =
        BuildFrontKinematicSamples(
            FrontLeftGeometryForCache,
            FourWheel.FrontAxle.LeftSuspension
                .KinematicSamples)
        && BuildFrontKinematicSamples(
            FrontGeometry,
            FourWheel.FrontAxle.RightSuspension
                .KinematicSamples);

    if (!bFrontKinematicCacheBuilt)
    {
        AddValidation(
            OutValidation,
            ETAValidationSeverity::Error,
            TEXT("Vehicle.FrontKinematicCacheBuildFailed"),
            TEXT(
                "Failed to derive front suspension kinematic cache from authored hardpoints."));
    }

    FourWheel.FrontAxle.AntiRollBar.CouplingRateNPerM =
        FrontSuspension.AntiRollCouplingRateNPerM;

    FourWheel.FrontAxle.AntiRollBar.MaxTransferForceN =
        FrontSuspension.AntiRollMaxTransferForceN;

    FourWheel.FrontAxle.SteeringRack.MaxRackDisplacementM =
        FrontSuspension.MaxRackDisplacementM;

    FourWheel.FrontAxle.SteeringRack.InputExponent =
        FrontSuspension.SteeringInputExponent;

    FourWheel.FrontAxle.SteeringRack.SteeringSign =
        FrontSuspension.SteeringSign;

    FTAMultiLinkSolverConfig& RearGeometry =
        FourWheel.RearAxle.RightGeometry;

    const FTARearLinkDefinition* RearLinks[TARearMultiLinkCount] =
    {
        &RearSuspension.Link0,
        &RearSuspension.Link1,
        &RearSuspension.Link2,
        &RearSuspension.Link3,
        &RearSuspension.Link4
    };

    for (int32 Index = 0;
         Index < TARearMultiLinkCount;
         ++Index)
    {
        RearGeometry.Links[Index].ChassisPickupReference =
            ToComLocal(
                RearLinks[Index]->ChassisPickup,
                CenterOfMassVehicleLocalM);

        RearGeometry.Links[Index].UprightPickupReference =
            ToComLocal(
                RearLinks[Index]->UprightPickup,
                CenterOfMassVehicleLocalM);
    }

    RearGeometry.WheelCenterReference =
        ToComLocal(
            RearSuspension.WheelCenter,
            CenterOfMassVehicleLocalM);

    RearGeometry.DamperChassisReference =
        ToComLocal(
            RearSuspension.DamperChassis,
            CenterOfMassVehicleLocalM);

    RearGeometry.DamperUprightReference =
        ToComLocal(
            RearSuspension.DamperUpright,
            CenterOfMassVehicleLocalM);

    RearGeometry.WheelForwardReference =
        FVector3d(1.0, 0.0, 0.0);

    RearGeometry.WheelUpReference =
        FVector3d(0.0, 0.0, 1.0);

    RearGeometry.SideSign = 1.0;

    RearGeometry.MinTravelM =
        RearSuspension.MinTravelM;

    RearGeometry.MaxTravelM =
        RearSuspension.MaxTravelM;

    RearGeometry.MaxIterations =
        RearSuspension.MaxSolverIterations;

    RearGeometry.PositionToleranceM =
        RearSuspension.PositionToleranceM;

    FourWheel.RearAxle.LeftSuspension =
        BuildSuspensionConfig(
            RearSuspension.SpringRateNPerM,
            RearSuspension.StaticSpringCompressionM,
            RearSuspension.BumpDampingNsPerM,
            RearSuspension.ReboundDampingNsPerM,
            RearSuspension.BumpStopRateNPerM,
            RearSuspension.DroopStopRateNPerM,
            RearSuspension.MinTravelM,
            RearSuspension.MaxTravelM);

    FourWheel.RearAxle.RightSuspension =
        FourWheel.RearAxle.LeftSuspension;

    const FTAMultiLinkSolverConfig RearLeftGeometryForCache =
        TAMultiLinkSolver::MirrorAcrossCenterline(
            RearGeometry);

    const bool bRearKinematicCacheBuilt =
        BuildRearKinematicSamples(
            RearLeftGeometryForCache,
            FourWheel.RearAxle.LeftSuspension
                .KinematicSamples)
        && BuildRearKinematicSamples(
            RearGeometry,
            FourWheel.RearAxle.RightSuspension
                .KinematicSamples);

    if (!bRearKinematicCacheBuilt)
    {
        AddValidation(
            OutValidation,
            ETAValidationSeverity::Error,
            TEXT("Vehicle.RearKinematicCacheBuildFailed"),
            TEXT(
                "Failed to derive rear suspension kinematic cache from authored hardpoints."));
    }

    FourWheel.RearAxle.AntiRollBar.CouplingRateNPerM =
        RearSuspension.AntiRollCouplingRateNPerM;

    FourWheel.RearAxle.AntiRollBar.MaxTransferForceN =
        RearSuspension.AntiRollMaxTransferForceN;

    CompileStructureRuntime(
        Structure,
        CenterOfMassVehicleLocalM,
        OutConfig.StructureRuntime,
        OutValidation);

    for (const FTAVehicleDamageRoute& Route :
         OutConfig.StructureRuntime.DamageRouting.Routes)
    {
        if (Route.Consumer ==
                ETAVehicleDamageConsumerType::WheelHub &&
            (Route.WheelIndex < 0 ||
             Route.WheelIndex >= WheelCount))
        {
            AddValidation(
                OutValidation,
                ETAValidationSeverity::Error,
                TEXT("Vehicle.InvalidWheelHubDamageRoute"),
                FString::Printf(
                    TEXT(
                        "Wheel-hub damage route for target component %d uses wheel index %d, "
                        "but the compiled vehicle has %d wheels."),
                    Route.TargetComponentIndex,
                    Route.WheelIndex,
                    WheelCount));
        }
    }

    ValidateStructureBindingLocations(
        OutConfig.StructureRuntime,
        FrontGeometry,
        RearGeometry,
        OutValidation);

    if (!TADoubleWishboneSolver::ValidateConfig(
            FourWheel.FrontAxle.RightGeometry))
    {
        AddValidation(
            OutValidation,
            ETAValidationSeverity::Error,
            TEXT("Vehicle.InvalidFrontSuspension"),
            TEXT("Compiled front double-wishbone geometry is invalid or degenerate."));
    }

    if (!TAMultiLinkSolver::ValidateConfig(
            FourWheel.RearAxle.RightGeometry))
    {
        AddValidation(
            OutValidation,
            ETAValidationSeverity::Error,
            TEXT("Vehicle.InvalidRearSuspension"),
            TEXT("Compiled rear multi-link geometry is invalid or degenerate."));
    }

    const double CompiledWheelbaseM =
        FrontGeometry.Hardpoints.WheelCenterReference.X
        - RearGeometry.WheelCenterReference.X;

    if (FMath::Abs(
            CompiledWheelbaseM
            - Dimensions.WheelbaseMeters) > 0.02)
    {
        AddValidation(
            OutValidation,
            ETAValidationSeverity::Warning,
            TEXT("Vehicle.WheelbaseHardpointMismatch"),
            FString::Printf(
                TEXT("Authored wheelbase %.3f m differs from suspension wheel-center geometry %.3f m."),
                Dimensions.WheelbaseMeters,
                CompiledWheelbaseM));
    }

    const double CompiledFrontTrackM =
        2.0
        * FMath::Abs(
            FrontGeometry.Hardpoints.WheelCenterReference.Y);

    const double CompiledRearTrackM =
        2.0
        * FMath::Abs(
            RearGeometry.WheelCenterReference.Y);

    if (FMath::Abs(
            CompiledFrontTrackM
            - Dimensions.TrackFrontMeters) > 0.02)
    {
        AddValidation(
            OutValidation,
            ETAValidationSeverity::Warning,
            TEXT("Vehicle.FrontTrackHardpointMismatch"),
            TEXT("Front track metadata differs from suspension wheel-center geometry."));
    }

    if (FMath::Abs(
            CompiledRearTrackM
            - Dimensions.TrackRearMeters) > 0.02)
    {
        AddValidation(
            OutValidation,
            ETAValidationSeverity::Warning,
            TEXT("Vehicle.RearTrackHardpointMismatch"),
            TEXT("Rear track metadata differs from suspension wheel-center geometry."));
    }

    if (OutValidation.HasErrors())
    {
        return false;
    }

    uint32 Hash =
        GetTypeHash(DefinitionId);

    Hash = HashCombineFast(
        Hash,
        GetTypeHash(Version.SchemaVersion));

    Hash = HashCombineFast(
        Hash,
        GetTypeHash(Version.PhysicsVersion));

    Hash = HashCombineFast(
        Hash,
        GetTypeHash(Version.DamageModelVersion));

    Hash = HashDouble(
        Hash,
        OutConfig.MassKg);

    Hash = HashVector(
        Hash,
        OutConfig.CenterOfMassMeters);

    Hash = HashVector(
        Hash,
        OutConfig.PrincipalInertiaKgm2);

    Hash = HashDouble(
        Hash,
        OutConfig.WheelbaseMeters);

    Hash = HashDouble(
        Hash,
        OutConfig.TrackFrontMeters);

    Hash = HashDouble(
        Hash,
        OutConfig.TrackRearMeters);

    Hash = HashCombineFast(
        Hash,
        GetTypeHash(OutConfig.WheelCount));

    Hash = HashTireConfig(
        Hash,
        VehicleRuntime.Tires[0]);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Wheels[0].InertiaKgm2);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Wheels[0].MaxBrakeTorqueNm);

    for (const FTAWheelRuntimeConfig& WheelConfig :
         VehicleRuntime.Wheels)
    {
        Hash = HashBool(
            Hash,
            WheelConfig.bDriven);
    }

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Engine.CrankInertiaKgm2);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Engine.FrictionConstantNm);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Engine.FrictionLinearNms);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Engine.FrictionQuadraticNms2);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Engine.IdleRPM);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Engine.RedlineRPM);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Engine.LimiterRPM);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Engine.StallRPM);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Engine.CombustionStartRPM);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Engine.StarterTorqueNm);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Engine.StarterMaxRPM);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Engine.IdleControlGainNmPerRPM);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Engine.MaxIdleControlTorqueNm);

    Hash = HashCombineFast(
        Hash,
        GetTypeHash(
            VehicleRuntime.Engine.TorqueCurve.Num()));

    for (const FTAEngineTorquePoint& Point :
         VehicleRuntime.Engine.TorqueCurve)
    {
        Hash = HashDouble(
            Hash,
            Point.RPM);

        Hash = HashDouble(
            Hash,
            Point.TorqueNm);
    }

    Hash = HashDouble(
        Hash,
        VehicleRuntime.EngineThermal.AmbientTemperatureC);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.EngineThermal.InitialCoolantTemperatureC);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.EngineThermal.EffectiveThermalMassJPerC);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.EngineThermal.BaseHeatGenerationW);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.EngineThermal.FullLoadAdditionalHeatW);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.EngineThermal.CoolingCapacityWPerC);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.EngineThermal.DerateStartTemperatureC);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.EngineThermal.DerateFullTemperatureC);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.EngineThermal.MinimumThermalTorqueFactor);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.EngineThermal.DamageStartTemperatureC);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.EngineThermal.DamageRatePerSecondAt150C);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Radiator.InitialCoolantMassKg);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Radiator.PunctureThresholdEnergyJ);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Radiator.FullLeakEnergyJ);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Radiator.MaxLeakAreaMm2);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Radiator.LeakMassFlowKgPerSecPerMm2);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Radiator.MinimumAirflowEfficiency01);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Clutch.MaxTorqueCapacityNm);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Clutch.CouplingStiffnessNms);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Clutch.ThermalMassJPerC);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Clutch.CoolingWPerC);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Clutch.AmbientTemperatureC);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Clutch.FadeStartTemperatureC);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Clutch.FadeEndTemperatureC);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Clutch.WearEnergyCapacityJ);

    Hash = HashCombineFast(
        Hash,
        GetTypeHash(
            VehicleRuntime.Gearbox.ForwardGearRatios.Num()));

    for (const double Ratio :
         VehicleRuntime.Gearbox.ForwardGearRatios)
    {
        Hash = HashDouble(
            Hash,
            Ratio);
    }

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Gearbox.ReverseGearRatio);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Gearbox.FinalDriveRatio);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Gearbox.MechanicalEfficiency);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Driveline.TorsionalStiffnessNmPerRad);

    Hash = HashDouble(
        Hash,
        VehicleRuntime.Driveline.TorsionalDampingNmsPerRad);

    Hash = HashFrontGeometry(
        Hash,
        FrontGeometry);

    Hash = HashSuspensionConfig(
        Hash,
        FourWheel.FrontAxle.LeftSuspension);

    Hash = HashDouble(
        Hash,
        FourWheel.FrontAxle.AntiRollBar.CouplingRateNPerM);

    Hash = HashDouble(
        Hash,
        FourWheel.FrontAxle.AntiRollBar.MaxTransferForceN);

    Hash = HashDouble(
        Hash,
        FourWheel.FrontAxle.SteeringRack.MaxRackDisplacementM);

    Hash = HashDouble(
        Hash,
        FourWheel.FrontAxle.SteeringRack.InputExponent);

    Hash = HashDouble(
        Hash,
        FourWheel.FrontAxle.SteeringRack.SteeringSign);

    Hash = HashRearGeometry(
        Hash,
        RearGeometry);

    Hash = HashSuspensionConfig(
        Hash,
        FourWheel.RearAxle.LeftSuspension);

    Hash = HashDouble(
        Hash,
        FourWheel.RearAxle.AntiRollBar.CouplingRateNPerM);

    Hash = HashDouble(
        Hash,
        FourWheel.RearAxle.AntiRollBar.MaxTransferForceN);

    Hash = HashStructureRuntime(
        Hash,
        OutConfig.StructureRuntime);

    OutConfig.PhysicsConfigHash =
        Hash;

    return true;
}
