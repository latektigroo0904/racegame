#include "TAAerodynamicsDefinition.h"

namespace
{
    uint32 HashDouble(uint32 Seed, const double Value)
    {
        return HashCombineFast(Seed, GetTypeHash(Value));
    }
}

bool TAAerodynamicsDefinition::Validate(
    const FTAAerodynamicsDefinition& Definition)
{
    const FVector3d Point(Definition.ApplicationPointVehicleLocalM);

    return
        FMath::IsFinite(Definition.ReferenceAreaM2)
        && Definition.ReferenceAreaM2 > 0.0
        && FMath::IsFinite(Definition.DragCoefficient)
        && Definition.DragCoefficient >= 0.0
        && FMath::IsFinite(Definition.LiftCoefficient)
        && FMath::IsFinite(Point.X)
        && FMath::IsFinite(Point.Y)
        && FMath::IsFinite(Point.Z);
}

bool TAAerodynamicsDefinition::Compile(
    const FTAAerodynamicsDefinition& Definition,
    const FVector3d& CenterOfMassVehicleLocalM,
    FTAAerodynamicsConfig& OutConfig)
{
    OutConfig = FTAAerodynamicsConfig{};

    if (!Validate(Definition)
        || !FMath::IsFinite(CenterOfMassVehicleLocalM.X)
        || !FMath::IsFinite(CenterOfMassVehicleLocalM.Y)
        || !FMath::IsFinite(CenterOfMassVehicleLocalM.Z))
    {
        return false;
    }

    OutConfig.ReferenceAreaM2 = Definition.ReferenceAreaM2;
    OutConfig.DragCoefficient = Definition.DragCoefficient;
    OutConfig.LiftCoefficient = Definition.LiftCoefficient;
    OutConfig.ApplicationPointBodyM =
        FVector3d(Definition.ApplicationPointVehicleLocalM)
        - CenterOfMassVehicleLocalM;

    return TAAerodynamics::ValidateConfig(OutConfig);
}

uint32 TAAerodynamicsDefinition::HashRuntimeConfig(
    uint32 Seed,
    const FTAAerodynamicsConfig& Config)
{
    Seed = HashDouble(Seed, Config.ReferenceAreaM2);
    Seed = HashDouble(Seed, Config.DragCoefficient);
    Seed = HashDouble(Seed, Config.LiftCoefficient);
    Seed = HashDouble(Seed, Config.ApplicationPointBodyM.X);
    Seed = HashDouble(Seed, Config.ApplicationPointBodyM.Y);
    Seed = HashDouble(Seed, Config.ApplicationPointBodyM.Z);
    return Seed;
}
