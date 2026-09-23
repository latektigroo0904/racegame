#include "TAVehicleDefinition.h"

FTARearSuspensionDefinition::FTARearSuspensionDefinition()
{
    Link0.ChassisPickup = FVector(-1.15, 0.35, -0.18);
    Link0.UprightPickup = FVector(-1.20, 0.68, -0.24);

    Link1.ChassisPickup = FVector(-1.50, 0.36, -0.20);
    Link1.UprightPickup = FVector(-1.42, 0.69, -0.25);

    Link2.ChassisPickup = FVector(-1.10, 0.34, -0.46);
    Link2.UprightPickup = FVector(-1.18, 0.72, -0.49);

    Link3.ChassisPickup = FVector(-1.53, 0.33, -0.47);
    Link3.UprightPickup = FVector(-1.45, 0.73, -0.50);

    Link4.ChassisPickup = FVector(-1.50, 0.38, -0.36);
    Link4.UprightPickup = FVector(-1.48, 0.71, -0.38);
}

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
        const uint32 Seed,
        const double Value)
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

    FVector3d CompilePointRelativeToCom(
        const FVector& AuthoredPoint,
        const FVector& AuthoredCenterOfMass)
    {
        return FVector3d(
            AuthoredPoint - AuthoredCenterOfMass);
    }

    FTADoubleWishboneSolverConfig BuildFrontGeometry(
        const FTAFrontSuspensionDefinition& Definition,
        const FVector& AuthoredCenterOfMass)
    {
        FTADoubleWishboneSolverConfig Config;
        FTADoubleWishboneHardpoints& H =
            Config.Hardpoints;

        H.UpperInnerA =
            CompilePointRelativeToCom(
                Definition.UpperInnerA,
                AuthoredCenterOfMass);

        H.UpperInnerB =
            CompilePointRelativeToCom(
                Definition.UpperInnerB,
                AuthoredCenterOfMass);

        H.LowerInnerA =
            CompilePointRelativeToCom(
                Definition.LowerInnerA,
                AuthoredCenterOfMass);

        H.LowerInnerB =
            CompilePointRelativeToCom(
                Definition.LowerInnerB,
                AuthoredCenterOfMass);

        H.TieRodInner =
            CompilePointRelativeToCom(
                Definition.TieRodInner,
                AuthoredCenterOfMass);

        H.DamperChassis =
            CompilePointRelativeToCom(
                Definition.DamperChassis,
                AuthoredCenterOfMass);

        H.DamperLowerArmReference =
            CompilePointRelativeToCom(
                Definition.DamperLowerArm,
                AuthoredCenterOfMass);

        H.UpperBallJointReference =
            CompilePointRelativeToCom(
                Definition.UpperBallJoint,
                AuthoredCenterOfMass);

        H.LowerBallJointReference =
            CompilePointRelativeToCom(
                Definition.LowerBallJoint,
                AuthoredCenterOfMass);

        H.TieRodOuterReference =
            CompilePointRelativeToCom(
                Definition.TieRodOuter,
                AuthoredCenterOfMass);

        H.WheelCenterReference =
            CompilePointRelativeToCom(
                Definition.WheelCenter,
                AuthoredCenterOfMass);

        H.WheelForwardReference =
            FVector3d(1.0, 0.0, 0.0);

        H.WheelUpReference =
            FVector3d(0.0, 0.0, 1.0);

        H.SteeringRackAxisLocal =
            FVector3d(0.0, 1.0, 0.0);

        H.SideSign = 1.0;

        Config.MinTravelM =
            Definition.MinTravelM;

        Config.MaxTravelM =
            Definition.MaxTravelM;

        Config.MaxIterations = 80;
        Config.PositionToleranceM = 0.0005;

        return Config;
    }

    FTAMultiLinkSolverConfig BuildRearGeometry(
        const FTARearSuspensionDefinition& Definition,
        const FVector& AuthoredCenterOfMass)
    {
        FTAMultiLinkSolverConfig Config;

        const FTARearLinkDefinition* Links[TARearMultiLinkCount] =
        {
            &Definition.Link0,
            &Definition.Link1,
            &Definition.Link2,
            &Definition.Link3,
            &Definition.Link4
        };

        for (int32 Index = 0;
             Index < TARearMultiLinkCount;
             ++Index)
        {
            Config.Links[Index].ChassisPickupReference =
                CompilePointRelativeToCom(
                    Links[Index]->ChassisPickup,
                    AuthoredCenterOfMass);

            Config.Links[Index].UprightPickupReference =
                CompilePointRelativeToCom(
                    Links[Index]->UprightPickup,
                    AuthoredCenterOfMass);
        }

        Config.WheelCenterReference =
            CompilePointRelativeToCom(
                Definition.WheelCenter,
                AuthoredCenterOfMass);

        Config.DamperChassisReference =
            CompilePointRelativeToCom(
                Definition.DamperChassis,
                AuthoredCenterOfMass);

        Config.DamperUprightReference =
            CompilePointRelativeToCom(
                Definition.DamperUpright,
                AuthoredCenterOfMass);

        Config.WheelForwardReference =
            FVector3d(1.0, 0.0, 0.0);

        Config.WheelUpReference =
            FVector3d(0.0, 0.0, 1.0);

        Config.SideSign = 1.0;

        Config.MinTravelM =
            Definition.MinTravelM;

        Config.MaxTravelM =
            Definition.MaxTravelM;

        Config.MaxIterations = 180;
        Config.PositionToleranceM = 0.001;

        return Config;
    }

    FTASuspensionRuntimeConfig BuildFrontSuspensionForceConfig(
        const FTAFrontSuspensionDefinition& Definition)
    {
        FTASuspensionRuntimeConfig Config;

        Config.SpringRateNPerM =
            Definition.SpringRateNPerM;

        Config.StaticSpringCompressionM =
            Definition.StaticSpringCompressionM;

        Config.BumpDampingNsPerM =
            Definition.BumpDampingNsPerM;

        Config.ReboundDampingNsPerM =
            Definition.ReboundDampingNsPerM;

        Config.BumpStopTravelM =
            Definition.MaxTravelM;

        Config.DroopStopTravelM =
            Definition.MinTravelM;

        return Config;
    }

    FTASuspensionRuntimeConfig BuildRearSuspensionForceConfig(
        const FTARearSuspensionDefinition& Definition)
    {
        FTASuspensionRuntimeConfig Config;

        Config.SpringRateNPerM =
            Definition.SpringRateNPerM;

        Config.StaticSpringCompressionM =
            Definition.StaticSpringCompressionM;

        Config.BumpDampingNsPerM =
            Definition.BumpDampingNsPerM;

        Config.ReboundDampingNsPerM =
            Definition.ReboundDampingNsPerM;

        Config.BumpStopTravelM =
            Definition.MaxTravelM;

        Config.DroopStopTravelM =
            Definition.MinTravelM;

        return Config;
    }

    void ApplyTireDefinition(
        const FTAPrototypeTireDefinition& Definition,
        FTATireRuntimeConfig& OutConfig)
    {
        OutConfig.UnloadedRadiusM =
            Definition.UnloadedRadiusM;

        OutConfig.ReferenceLoadN =
            Definition.ReferenceLoadN;

        OutConfig.ReferencePressureKPa =
            Definition.ReferencePressureKPa;

        OutConfig.DryPeakMu =
            Definition.DryPeakMu;

        OutConfig.NewTreadDepthMm =
            Definition.NewTreadDepthMm;

        OutConfig.RadialStiffnessNPerM =
            Definition.RadialStiffnessNPerM;

        OutConfig.RadialProgressiveStiffnessNPerM2 =
            Definition.RadialProgressiveStiffnessNPerM2;

        OutConfig.RadialDampingNsPerM =
            Definition.RadialDampingNsPerM;

        OutConfig.MaxRadialDeflectionM =
            Definition.MaxRadialDeflectionM;
    }
}

bool UTAVehicleDefinition::BuildCompiledConfig(
    FTAVehicleCompiledConfig& OutConfig,
    FTAValidationResult& OutValidation) const
{
    OutConfig = FTAVehicleCompiledConfig{};
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
            TEXT("All principal inertia components must be greater than zero."));
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
            TEXT("The current high-fidelity prototype runtime requires exactly four wheels."));
    }

    if (Tire.UnloadedRadiusM <= 0.0 ||
        Tire.ReferencePressureKPa <= 0.0 ||
        Tire.ReferenceLoadN <= 0.0 ||
        Tire.RadialStiffnessNPerM <= 0.0 ||
        Tire.MaxRadialDeflectionM <= 0.0)
    {
        AddValidation(
            OutValidation,
            ETAValidationSeverity::Error,
            TEXT("Vehicle.InvalidTire"),
            TEXT("Tire radius, reference pressure/load, radial stiffness and max deflection must be positive."));
    }

    if (FrontSuspension.MaxTravelM <=
        FrontSuspension.MinTravelM)
    {
        AddValidation(
            OutValidation,
            ETAValidationSeverity::Error,
            TEXT("Vehicle.InvalidFrontTravel"),
            TEXT("Front suspension max travel must exceed min travel."));
    }

    if (RearSuspension.MaxTravelM <=
        RearSuspension.MinTravelM)
    {
        AddValidation(
            OutValidation,
            ETAValidationSeverity::Error,
            TEXT("Vehicle.InvalidRearTravel"),
            TEXT("Rear suspension max travel must exceed min travel."));
    }

    if (Drivetrain.IdleRPM <= 0.0 ||
        Drivetrain.RedlineRPM <= Drivetrain.IdleRPM ||
        Drivetrain.LimiterRPM < Drivetrain.RedlineRPM)
    {
        AddValidation(
            OutValidation,
            ETAValidationSeverity::Error,
            TEXT("Vehicle.InvalidEngineSpeedRange"),
            TEXT("Engine idle, redline and limiter RPM values are inconsistent."));
    }

    if (Drivetrain.ForwardGearRatios.Num() == 0 ||
        Drivetrain.FinalDriveRatio <= 0.0 ||
        Drivetrain.ReverseGearRatio <= 0.0)
    {
        AddValidation(
            OutValidation,
            ETAValidationSeverity::Error,
            TEXT("Vehicle.InvalidGearbox"),
            TEXT("Gearbox requires positive forward, reverse and final-drive ratios."));
    }

    for (const double Ratio :
         Drivetrain.ForwardGearRatios)
    {
        if (Ratio <= 0.0)
        {
            AddValidation(
                OutValidation,
                ETAValidationSeverity::Error,
                TEXT("Vehicle.InvalidForwardGearRatio"),
                TEXT("All forward gear ratios must be positive."));

            break;
        }
    }

    const FTADoubleWishboneSolverConfig FrontGeometry =
        BuildFrontGeometry(
            FrontSuspension,
            Mass.CenterOfMassMeters);

    const FTAMultiLinkSolverConfig RearGeometry =
        BuildRearGeometry(
            RearSuspension,
            Mass.CenterOfMassMeters);

    if (!TADoubleWishboneSolver::ValidateConfig(
            FrontGeometry))
    {
        AddValidation(
            OutValidation,
            ETAValidationSeverity::Error,
            TEXT("Vehicle.InvalidFrontSuspensionGeometry"),
            TEXT("Front double-wishbone geometry is degenerate or inconsistent."));
    }

    if (!TAMultiLinkSolver::ValidateConfig(
            RearGeometry))
    {
        AddValidation(
            OutValidation,
            ETAValidationSeverity::Error,
            TEXT("Vehicle.InvalidRearSuspensionGeometry"),
            TEXT("Rear five-link geometry is degenerate or inconsistent."));
    }

    if (OutValidation.HasErrors())
    {
        return false;
    }

    OutConfig.Version = Version;
    OutConfig.DefinitionId = DefinitionId;

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

    for (int32 Index = 0;
         Index < 4;
         ++Index)
    {
        ApplyTireDefinition(
            Tire,
            VehicleRuntime.Tires[Index]);

        VehicleRuntime.Wheels[Index].RadiusM =
            Tire.UnloadedRadiusM;

        VehicleRuntime.Wheels[Index].InertiaKgm2 =
            Wheel.InertiaKgm2;

        VehicleRuntime.Wheels[Index].MaxBrakeTorqueNm =
            Wheel.MaxBrakeTorqueNm;
    }

    const int32 DrivenLeft =
        Drivetrain.bRearWheelDrive ? 2 : 0;

    const int32 DrivenRight =
        Drivetrain.bRearWheelDrive ? 3 : 1;

    VehicleRuntime.Wheels[DrivenLeft].bDriven = true;
    VehicleRuntime.Wheels[DrivenRight].bDriven = true;

    VehicleRuntime.Engine.IdleRPM =
        Drivetrain.IdleRPM;

    VehicleRuntime.Engine.RedlineRPM =
        Drivetrain.RedlineRPM;

    VehicleRuntime.Engine.LimiterRPM =
        Drivetrain.LimiterRPM;

    VehicleRuntime.Engine.CrankInertiaKgm2 =
        Drivetrain.CrankInertiaKgm2;

    VehicleRuntime.Clutch.MaxTorqueCapacityNm =
        Drivetrain.ClutchMaxTorqueNm;

    VehicleRuntime.Gearbox.ForwardGearRatios =
        Drivetrain.ForwardGearRatios;

    VehicleRuntime.Gearbox.ReverseGearRatio =
        Drivetrain.ReverseGearRatio;

    VehicleRuntime.Gearbox.FinalDriveRatio =
        Drivetrain.FinalDriveRatio;

    VehicleRuntime.Gearbox.MechanicalEfficiency =
        Drivetrain.MechanicalEfficiency;

    FTAFourWheelRuntimeConfig& FourWheelRuntime =
        OutConfig.FourWheelRuntime;

    FourWheelRuntime.FrontAxle.RightGeometry =
        FrontGeometry;

    FourWheelRuntime.FrontAxle.LeftSuspension =
        BuildFrontSuspensionForceConfig(
            FrontSuspension);

    FourWheelRuntime.FrontAxle.RightSuspension =
        FourWheelRuntime.FrontAxle.LeftSuspension;

    FourWheelRuntime.FrontAxle.AntiRollBar.CouplingRateNPerM =
        FrontSuspension.AntiRollCouplingRateNPerM;

    FourWheelRuntime.FrontAxle.AntiRollBar.MaxTransferForceN =
        FrontSuspension.AntiRollMaxTransferForceN;

    FourWheelRuntime.FrontAxle.SteeringRack.MaxRackDisplacementM =
        FrontSuspension.MaxRackDisplacementM;

    FourWheelRuntime.FrontAxle.SteeringRack.InputExponent =
        FrontSuspension.SteeringInputExponent;

    FourWheelRuntime.FrontAxle.SteeringRack.SteeringSign =
        FrontSuspension.SteeringSign;

    FourWheelRuntime.RearAxle.RightGeometry =
        RearGeometry;

    FourWheelRuntime.RearAxle.LeftSuspension =
        BuildRearSuspensionForceConfig(
            RearSuspension);

    FourWheelRuntime.RearAxle.RightSuspension =
        FourWheelRuntime.RearAxle.LeftSuspension;

    FourWheelRuntime.RearAxle.AntiRollBar.CouplingRateNPerM =
        RearSuspension.AntiRollCouplingRateNPerM;

    FourWheelRuntime.RearAxle.AntiRollBar.MaxTransferForceN =
        RearSuspension.AntiRollMaxTransferForceN;

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

    Hash = HashDouble(Hash, OutConfig.MassKg);
    Hash = HashVector(Hash, OutConfig.CenterOfMassMeters);
    Hash = HashVector(Hash, OutConfig.PrincipalInertiaKgm2);

    Hash = HashDouble(Hash, OutConfig.LengthMeters);
    Hash = HashDouble(Hash, OutConfig.WidthMeters);
    Hash = HashDouble(Hash, OutConfig.HeightMeters);
    Hash = HashDouble(Hash, OutConfig.WheelbaseMeters);
    Hash = HashDouble(Hash, OutConfig.TrackFrontMeters);
    Hash = HashDouble(Hash, OutConfig.TrackRearMeters);

    Hash = HashDouble(Hash, Tire.UnloadedRadiusM);
    Hash = HashDouble(Hash, Tire.ReferenceLoadN);
    Hash = HashDouble(Hash, Tire.ReferencePressureKPa);
    Hash = HashDouble(Hash, Tire.DryPeakMu);
    Hash = HashDouble(Hash, Tire.NewTreadDepthMm);
    Hash = HashDouble(Hash, Tire.RadialStiffnessNPerM);
    Hash = HashDouble(Hash, Tire.RadialProgressiveStiffnessNPerM2);
    Hash = HashDouble(Hash, Tire.RadialDampingNsPerM);
    Hash = HashDouble(Hash, Tire.MaxRadialDeflectionM);

    Hash = HashDouble(Hash, Wheel.InertiaKgm2);
    Hash = HashDouble(Hash, Wheel.MaxBrakeTorqueNm);

    const FTADoubleWishboneHardpoints& FrontH =
        FrontGeometry.Hardpoints;

    Hash = HashVector(Hash, FrontH.UpperInnerA);
    Hash = HashVector(Hash, FrontH.UpperInnerB);
    Hash = HashVector(Hash, FrontH.LowerInnerA);
    Hash = HashVector(Hash, FrontH.LowerInnerB);
    Hash = HashVector(Hash, FrontH.TieRodInner);
    Hash = HashVector(Hash, FrontH.DamperChassis);
    Hash = HashVector(Hash, FrontH.DamperLowerArmReference);
    Hash = HashVector(Hash, FrontH.UpperBallJointReference);
    Hash = HashVector(Hash, FrontH.LowerBallJointReference);
    Hash = HashVector(Hash, FrontH.TieRodOuterReference);
    Hash = HashVector(Hash, FrontH.WheelCenterReference);

    Hash = HashDouble(Hash, FrontSuspension.MinTravelM);
    Hash = HashDouble(Hash, FrontSuspension.MaxTravelM);
    Hash = HashDouble(Hash, FrontSuspension.SpringRateNPerM);
    Hash = HashDouble(Hash, FrontSuspension.StaticSpringCompressionM);
    Hash = HashDouble(Hash, FrontSuspension.BumpDampingNsPerM);
    Hash = HashDouble(Hash, FrontSuspension.ReboundDampingNsPerM);
    Hash = HashDouble(Hash, FrontSuspension.AntiRollCouplingRateNPerM);
    Hash = HashDouble(Hash, FrontSuspension.AntiRollMaxTransferForceN);
    Hash = HashDouble(Hash, FrontSuspension.MaxRackDisplacementM);
    Hash = HashDouble(Hash, FrontSuspension.SteeringInputExponent);
    Hash = HashDouble(Hash, FrontSuspension.SteeringSign);

    for (int32 Index = 0;
         Index < TARearMultiLinkCount;
         ++Index)
    {
        Hash = HashVector(
            Hash,
            RearGeometry.Links[Index].ChassisPickupReference);

        Hash = HashVector(
            Hash,
            RearGeometry.Links[Index].UprightPickupReference);
    }

    Hash = HashVector(
        Hash,
        RearGeometry.WheelCenterReference);

    Hash = HashVector(
        Hash,
        RearGeometry.DamperChassisReference);

    Hash = HashVector(
        Hash,
        RearGeometry.DamperUprightReference);

    Hash = HashDouble(Hash, RearSuspension.MinTravelM);
    Hash = HashDouble(Hash, RearSuspension.MaxTravelM);
    Hash = HashDouble(Hash, RearSuspension.SpringRateNPerM);
    Hash = HashDouble(Hash, RearSuspension.StaticSpringCompressionM);
    Hash = HashDouble(Hash, RearSuspension.BumpDampingNsPerM);
    Hash = HashDouble(Hash, RearSuspension.ReboundDampingNsPerM);
    Hash = HashDouble(Hash, RearSuspension.AntiRollCouplingRateNPerM);
    Hash = HashDouble(Hash, RearSuspension.AntiRollMaxTransferForceN);

    Hash = HashDouble(Hash, Drivetrain.IdleRPM);
    Hash = HashDouble(Hash, Drivetrain.RedlineRPM);
    Hash = HashDouble(Hash, Drivetrain.LimiterRPM);
    Hash = HashDouble(Hash, Drivetrain.CrankInertiaKgm2);
    Hash = HashDouble(Hash, Drivetrain.ClutchMaxTorqueNm);

    for (const double Ratio :
         Drivetrain.ForwardGearRatios)
    {
        Hash = HashDouble(Hash, Ratio);
    }

    Hash = HashDouble(Hash, Drivetrain.ReverseGearRatio);
    Hash = HashDouble(Hash, Drivetrain.FinalDriveRatio);
    Hash = HashDouble(Hash, Drivetrain.MechanicalEfficiency);

    Hash = HashCombineFast(
        Hash,
        GetTypeHash(
            Drivetrain.bRearWheelDrive));

    OutConfig.PhysicsConfigHash = Hash;
    return true;
}
