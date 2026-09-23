#pragma once

#include "CoreMinimal.h"
#include "TACoreTypes.generated.h"

namespace TAVersion
{
    constexpr int32 CurrentSchemaVersion = 2;
    constexpr int32 CurrentPhysicsVersion = 2;
    constexpr int32 CurrentDamageModelVersion = 2;
}

USTRUCT(BlueprintType)
struct TA_CORE_API FTASimulationVersion
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Version")
    int32 SchemaVersion =
        TAVersion::CurrentSchemaVersion;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Version")
    int32 PhysicsVersion =
        TAVersion::CurrentPhysicsVersion;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Version")
    int32 DamageModelVersion =
        TAVersion::CurrentDamageModelVersion;
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
