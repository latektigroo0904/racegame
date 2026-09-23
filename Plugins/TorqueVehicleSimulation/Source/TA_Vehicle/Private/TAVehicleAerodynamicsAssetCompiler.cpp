#include "TAVehicleAerodynamicsAssetCompiler.h"

bool TAVehicleAerodynamicsAssetCompiler::CompileValidatedAndHash(
    const FTAAerodynamicsDefinition& Definition,
    const FVector3d& CenterOfMassVehicleLocalM,
    uint32& InOutPhysicsHash,
    FTAAerodynamicsConfig& OutRuntimeConfig)
{
    if (!TAAerodynamicsDefinition::Validate(Definition))
    {
        return false;
    }

    FTAAerodynamicsConfig Compiled;
    if (!TAAerodynamicsDefinition::Compile(
            Definition,
            CenterOfMassVehicleLocalM,
            Compiled))
    {
        return false;
    }

    const uint32 CompiledHash =
        TAAerodynamicsDefinition::HashRuntimeConfig(
            InOutPhysicsHash,
            Compiled);

    OutRuntimeConfig = Compiled;
    InOutPhysicsHash = CompiledHash;
    return true;
}
