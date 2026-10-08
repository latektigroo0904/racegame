#pragma once

#include "CoreMinimal.h"
#include "TAAerodynamics.h"
#include "TAAerodynamicsDefinition.generated.h"

USTRUCT(BlueprintType)
struct TA_VEHICLE_API FTAAerodynamicsDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Aerodynamics", meta=(ClampMin="0.01", Units="m^2"))
    double ReferenceAreaM2 = 2.10;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Aerodynamics", meta=(ClampMin="0.0"))
    double DragCoefficient = 0.32;

    // Negative values produce downforce in the current solver convention.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Aerodynamics|Front")
    double FrontLiftCoefficient = -0.05;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Aerodynamics|Rear")
    double RearLiftCoefficient = -0.07;

    // Authored in vehicle-origin-local metres. Compilation converts to COM-local.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Aerodynamics|Drag")
    FVector DragApplicationPointVehicleLocalM =
        FVector(0.0, 0.0, 0.10);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Aerodynamics|Front")
    FVector FrontLiftApplicationPointVehicleLocalM =
        FVector(1.10, 0.0, 0.0);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Aerodynamics|Rear")
    FVector RearLiftApplicationPointVehicleLocalM =
        FVector(-1.10, 0.0, 0.0);
};

namespace TAAerodynamicsDefinition
{
    TA_VEHICLE_API bool Validate(
        const FTAAerodynamicsDefinition& Definition);

    TA_VEHICLE_API bool Compile(
        const FTAAerodynamicsDefinition& Definition,
        const FVector3d& CenterOfMassVehicleLocalM,
        FTAAerodynamicsConfig& OutConfig);

    // Hashes effective runtime values, not authored coordinate representation.
    TA_VEHICLE_API uint32 HashRuntimeConfig(
        uint32 Seed,
        const FTAAerodynamicsConfig& Config);
}
