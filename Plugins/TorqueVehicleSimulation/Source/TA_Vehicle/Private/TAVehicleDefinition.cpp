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

    uint32 HashDouble(uint32 Seed, const double Value)
    {
        return HashCombineFast(Seed, GetTypeHash(Value));
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

    if (WheelCount < 2)
    {
        AddValidation(
            OutValidation,
            ETAValidationSeverity::Error,
            TEXT("Vehicle.InvalidWheelCount"),
            TEXT("Vehicle requires at least two wheels."));
    }

    if (OutValidation.HasErrors())
    {
        return false;
    }

    OutConfig.Version = Version;
    OutConfig.DefinitionId = DefinitionId;

    OutConfig.MassKg = Mass.ReferenceMassKg;
    OutConfig.CenterOfMassMeters = FVector3d(Mass.CenterOfMassMeters);
    OutConfig.PrincipalInertiaKgm2 = FVector3d(Mass.PrincipalInertiaKgm2);

    OutConfig.LengthMeters = Dimensions.LengthMeters;
    OutConfig.WidthMeters = Dimensions.WidthMeters;
    OutConfig.HeightMeters = Dimensions.HeightMeters;
    OutConfig.WheelbaseMeters = Dimensions.WheelbaseMeters;
    OutConfig.TrackFrontMeters = Dimensions.TrackFrontMeters;
    OutConfig.TrackRearMeters = Dimensions.TrackRearMeters;
    OutConfig.WheelCount = WheelCount;

    uint32 Hash = GetTypeHash(DefinitionId);
    Hash = HashCombineFast(Hash, GetTypeHash(Version.SchemaVersion));
    Hash = HashCombineFast(Hash, GetTypeHash(Version.PhysicsVersion));
    Hash = HashCombineFast(Hash, GetTypeHash(Version.DamageModelVersion));
    Hash = HashDouble(Hash, OutConfig.MassKg);
    Hash = HashDouble(Hash, OutConfig.CenterOfMassMeters.X);
    Hash = HashDouble(Hash, OutConfig.CenterOfMassMeters.Y);
    Hash = HashDouble(Hash, OutConfig.CenterOfMassMeters.Z);
    Hash = HashDouble(Hash, OutConfig.WheelbaseMeters);
    Hash = HashDouble(Hash, OutConfig.TrackFrontMeters);
    Hash = HashDouble(Hash, OutConfig.TrackRearMeters);
    Hash = HashCombineFast(Hash, GetTypeHash(OutConfig.WheelCount));

    OutConfig.PhysicsConfigHash = Hash;
    return true;
}
