#include "TAAerodynamicsDefinition.h"

namespace
{
    uint32 HashDouble(
        uint32 Seed,
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

    bool IsFiniteVector(
        const FVector3d& Value)
    {
        return
            FMath::IsFinite(Value.X)
            && FMath::IsFinite(Value.Y)
            && FMath::IsFinite(Value.Z);
    }
}

bool TAAerodynamicsDefinition::Validate(
    const FTAAerodynamicsDefinition& Definition)
{
    return
        FMath::IsFinite(Definition.ReferenceAreaM2)
        && Definition.ReferenceAreaM2 > 0.0
        && FMath::IsFinite(Definition.DragCoefficient)
        && Definition.DragCoefficient >= 0.0
        && FMath::IsFinite(Definition.FrontLiftCoefficient)
        && FMath::IsFinite(Definition.RearLiftCoefficient)
        && IsFiniteVector(
            FVector3d(
                Definition.DragApplicationPointVehicleLocalM))
        && IsFiniteVector(
            FVector3d(
                Definition.FrontLiftApplicationPointVehicleLocalM))
        && IsFiniteVector(
            FVector3d(
                Definition.RearLiftApplicationPointVehicleLocalM));
}

bool TAAerodynamicsDefinition::Compile(
    const FTAAerodynamicsDefinition& Definition,
    const FVector3d& CenterOfMassVehicleLocalM,
    FTAAerodynamicsConfig& OutConfig)
{
    OutConfig =
        FTAAerodynamicsConfig{};

    if (!Validate(Definition)
        || !IsFiniteVector(
            CenterOfMassVehicleLocalM))
    {
        return false;
    }

    OutConfig.ReferenceAreaM2 =
        Definition.ReferenceAreaM2;

    OutConfig.DragCoefficient =
        Definition.DragCoefficient;

    OutConfig.FrontLiftCoefficient =
        Definition.FrontLiftCoefficient;

    OutConfig.RearLiftCoefficient =
        Definition.RearLiftCoefficient;

    OutConfig.DragApplicationPointBodyM =
        FVector3d(
            Definition.DragApplicationPointVehicleLocalM)
        - CenterOfMassVehicleLocalM;

    OutConfig.FrontLiftApplicationPointBodyM =
        FVector3d(
            Definition.FrontLiftApplicationPointVehicleLocalM)
        - CenterOfMassVehicleLocalM;

    OutConfig.RearLiftApplicationPointBodyM =
        FVector3d(
            Definition.RearLiftApplicationPointVehicleLocalM)
        - CenterOfMassVehicleLocalM;

    return TAAerodynamics::ValidateConfig(
        OutConfig);
}

uint32 TAAerodynamicsDefinition::HashRuntimeConfig(
    uint32 Seed,
    const FTAAerodynamicsConfig& Config)
{
    Seed = HashDouble(
        Seed,
        Config.ReferenceAreaM2);

    Seed = HashDouble(
        Seed,
        Config.DragCoefficient);

    Seed = HashDouble(
        Seed,
        Config.FrontLiftCoefficient);

    Seed = HashDouble(
        Seed,
        Config.RearLiftCoefficient);

    Seed = HashVector(
        Seed,
        Config.DragApplicationPointBodyM);

    Seed = HashVector(
        Seed,
        Config.FrontLiftApplicationPointBodyM);

    Seed = HashVector(
        Seed,
        Config.RearLiftApplicationPointBodyM);

    return Seed;
}
