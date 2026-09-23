#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TACoreTypes.h"
#include "TAVehicleRuntime.h"
#include "TAVehicleDefinition.generated.h"

USTRUCT(BlueprintType)
struct TA_VEHICLE_API FTAVehicleDimensionsDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dimensions", meta=(ClampMin="0.01", Units="m"))
    double LengthMeters = 4.3;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dimensions", meta=(ClampMin="0.01", Units="m"))
    double WidthMeters = 1.8;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dimensions", meta=(ClampMin="0.01", Units="m"))
    double HeightMeters = 1.3;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dimensions", meta=(ClampMin="0.01", Units="m"))
    double WheelbaseMeters = 2.62;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dimensions", meta=(ClampMin="0.01", Units="m"))
    double TrackFrontMeters = 1.55;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dimensions", meta=(ClampMin="0.01", Units="m"))
    double TrackRearMeters = 1.53;
};

USTRUCT(BlueprintType)
struct TA_VEHICLE_API FTAVehicleMassDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mass", meta=(ClampMin="1.0", Units="kg"))
    double ReferenceMassKg = 1420.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mass")
    FVector CenterOfMassMeters = FVector(0.0, 0.0, 0.45);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mass")
    FVector PrincipalInertiaKgm2 = FVector(650.0, 1800.0, 1900.0);
};

UCLASS(BlueprintType)
class TA_VEHICLE_API UTAVehicleDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Identity")
    FName DefinitionId = TEXT("TA-P01");

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Version")
    FTASimulationVersion Version;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Vehicle")
    FTAVehicleDimensionsDefinition Dimensions;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Vehicle")
    FTAVehicleMassDefinition Mass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Vehicle", meta=(ClampMin="2", ClampMax="16"))
    int32 WheelCount = 4;

    bool BuildCompiledConfig(FTAVehicleCompiledConfig& OutConfig, FTAValidationResult& OutValidation) const;
};
