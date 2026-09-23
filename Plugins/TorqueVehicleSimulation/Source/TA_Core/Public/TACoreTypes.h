#pragma once

#include "CoreMinimal.h"
#include "TACoreTypes.generated.h"

USTRUCT(BlueprintType)
struct TA_CORE_API FTASimulationVersion
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Version")
    int32 SchemaVersion = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Version")
    int32 PhysicsVersion = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Version")
    int32 DamageModelVersion = 1;
};

UENUM(BlueprintType)
enum class ETAValidationSeverity : uint8
{
    Info,
    Warning,
    Error
};

USTRUCT(BlueprintType)
struct TA_CORE_API FTAValidationMessage
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Validation")
    ETAValidationSeverity Severity = ETAValidationSeverity::Info;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Validation")
    FName Code = NAME_None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Validation")
    FString Message;
};

struct TA_CORE_API FTAValidationResult
{
    TArray<FTAValidationMessage> Messages;

    bool HasErrors() const
    {
        for (const FTAValidationMessage& Entry : Messages)
        {
            if (Entry.Severity == ETAValidationSeverity::Error)
            {
                return true;
            }
        }
        return false;
    }
};

namespace TAUnits
{
    constexpr double CentimetersPerMeter = 100.0;
    constexpr double RadiansPerRevolution = 2.0 * UE_DOUBLE_PI;

    FORCEINLINE double MetersToCentimeters(const double Meters)
    {
        return Meters * CentimetersPerMeter;
    }

    FORCEINLINE double CentimetersToMeters(const double Centimeters)
    {
        return Centimeters / CentimetersPerMeter;
    }

    FORCEINLINE double RPMToRadiansPerSecond(const double RPM)
    {
        return RPM * RadiansPerRevolution / 60.0;
    }

    FORCEINLINE double RadiansPerSecondToRPM(const double RadiansPerSecond)
    {
        return RadiansPerSecond * 60.0 / RadiansPerRevolution;
    }
}
