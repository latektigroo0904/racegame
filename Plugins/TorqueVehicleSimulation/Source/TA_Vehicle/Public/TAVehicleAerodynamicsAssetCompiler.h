#pragma once

#include "CoreMinimal.h"
#include "TAAerodynamicsDefinition.h"

/**
 * Small canonical adapter for asset-side aerodynamics compilation.
 *
 * This deliberately keeps authoring validation, vehicle-origin -> COM-local
 * conversion and effective-runtime hashing in one place so
 * UTAVehicleDefinition cannot accidentally hash raw authored coordinates.
 */
namespace TAVehicleAerodynamicsAssetCompiler
{
    /**
     * Validates and compiles authored aero into effective runtime form.
     * Returns false without mutating InOutPhysicsHash when authoring is invalid.
     * On success, hashes the compiled runtime values exactly once.
     */
    TA_VEHICLE_API bool CompileValidatedAndHash(
        const FTAAerodynamicsDefinition& Definition,
        const FVector3d& CenterOfMassVehicleLocalM,
        uint32& InOutPhysicsHash,
        FTAAerodynamicsConfig& OutRuntimeConfig);
}
