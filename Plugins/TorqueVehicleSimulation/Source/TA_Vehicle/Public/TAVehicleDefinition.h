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

USTRUCT(BlueprintType)
struct TA_VEHICLE_API FTAPrototypeTireDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire", meta=(ClampMin="0.10", Units="m"))
    double UnloadedRadiusM = 0.327;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire", meta=(ClampMin="1.0", Units="N"))
    double ReferenceLoadN = 3500.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire", meta=(ClampMin="1.0", Units="kPa"))
    double ReferencePressureKPa = 230.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire", meta=(ClampMin="0.1"))
    double DryPeakMu = 1.05;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire", meta=(ClampMin="0.0", Units="mm"))
    double NewTreadDepthMm = 7.5;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Vertical", meta=(ClampMin="1.0", Units="N/m"))
    double RadialStiffnessNPerM = 220000.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Vertical", meta=(ClampMin="0.0", Units="N/m^2"))
    double RadialProgressiveStiffnessNPerM2 = 1200000.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Vertical", meta=(ClampMin="0.0", Units="N*s/m"))
    double RadialDampingNsPerM = 2200.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Vertical", meta=(ClampMin="0.005", Units="m"))
    double MaxRadialDeflectionM = 0.060;
};

USTRUCT(BlueprintType)
struct TA_VEHICLE_API FTAPrototypeWheelDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Wheel", meta=(ClampMin="0.01", Units="kg*m^2"))
    double InertiaKgm2 = 1.20;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Brakes", meta=(ClampMin="0.0", Units="N*m"))
    double MaxBrakeTorqueNm = 2200.0;
};

USTRUCT(BlueprintType)
struct TA_VEHICLE_API FTAFrontSuspensionDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Front|Hardpoints")
    FVector UpperInnerA = FVector(1.48, 0.38, -0.22);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Front|Hardpoints")
    FVector UpperInnerB = FVector(1.14, 0.38, -0.22);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Front|Hardpoints")
    FVector LowerInnerA = FVector(1.50, 0.35, -0.48);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Front|Hardpoints")
    FVector LowerInnerB = FVector(1.12, 0.35, -0.48);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Front|Hardpoints")
    FVector TieRodInner = FVector(1.12, 0.35, -0.38);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Front|Hardpoints")
    FVector DamperChassis = FVector(1.31, 0.40, -0.05);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Front|Hardpoints")
    FVector DamperLowerArm = FVector(1.31, 0.52, -0.43);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Front|Hardpoints")
    FVector UpperBallJoint = FVector(1.31, 0.70, -0.25);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Front|Hardpoints")
    FVector LowerBallJoint = FVector(1.31, 0.73, -0.50);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Front|Hardpoints")
    FVector TieRodOuter = FVector(1.12, 0.71, -0.39);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Front|Hardpoints")
    FVector WheelCenter = FVector(1.31, 0.775, -0.45);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Front|Travel", meta=(Units="m"))
    double MinTravelM = -0.07;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Front|Travel", meta=(Units="m"))
    double MaxTravelM = 0.09;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Front|Spring", meta=(ClampMin="0.0", Units="N/m"))
    double SpringRateNPerM = 100000.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Front|Spring", meta=(ClampMin="0.0", Units="m"))
    double StaticSpringCompressionM = 0.0810;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Front|Damper", meta=(ClampMin="0.0", Units="N*s/m"))
    double BumpDampingNsPerM = 4500.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Front|Damper", meta=(ClampMin="0.0", Units="N*s/m"))
    double ReboundDampingNsPerM = 6500.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Front|AntiRoll", meta=(ClampMin="0.0", Units="N/m"))
    double AntiRollCouplingRateNPerM = 12000.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Front|AntiRoll", meta=(ClampMin="0.0", Units="N"))
    double AntiRollMaxTransferForceN = 3000.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Steering", meta=(ClampMin="0.0", Units="m"))
    double MaxRackDisplacementM = 0.035;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Steering", meta=(ClampMin="0.05"))
    double SteeringInputExponent = 1.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Steering", meta=(ClampMin="-1.0", ClampMax="1.0"))
    double SteeringSign = -1.0;
};

USTRUCT(BlueprintType)
struct TA_VEHICLE_API FTARearLinkDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rear|Link")
    FVector ChassisPickup = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rear|Link")
    FVector UprightPickup = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct TA_VEHICLE_API FTARearSuspensionDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rear|Links")
    FTARearLinkDefinition Link0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rear|Links")
    FTARearLinkDefinition Link1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rear|Links")
    FTARearLinkDefinition Link2;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rear|Links")
    FTARearLinkDefinition Link3;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rear|Links")
    FTARearLinkDefinition Link4;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rear|Hardpoints")
    FVector WheelCenter = FVector(-1.31, 0.765, -0.45);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rear|Hardpoints")
    FVector DamperChassis = FVector(-1.31, 0.40, -0.05);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rear|Hardpoints")
    FVector DamperUpright = FVector(-1.31, 0.64, -0.40);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rear|Travel", meta=(Units="m"))
    double MinTravelM = -0.07;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rear|Travel", meta=(Units="m"))
    double MaxTravelM = 0.09;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rear|Spring", meta=(ClampMin="0.0", Units="N/m"))
    double SpringRateNPerM = 50000.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rear|Spring", meta=(ClampMin="0.0", Units="m"))
    double StaticSpringCompressionM = 0.0987;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rear|Damper", meta=(ClampMin="0.0", Units="N*s/m"))
    double BumpDampingNsPerM = 3800.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rear|Damper", meta=(ClampMin="0.0", Units="N*s/m"))
    double ReboundDampingNsPerM = 5200.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rear|AntiRoll", meta=(ClampMin="0.0", Units="N/m"))
    double AntiRollCouplingRateNPerM = 9000.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rear|AntiRoll", meta=(ClampMin="0.0", Units="N"))
    double AntiRollMaxTransferForceN = 2500.0;
};

USTRUCT(BlueprintType)
struct TA_VEHICLE_API FTAPrototypeDrivetrainDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Powertrain", meta=(ClampMin="100.0", Units="rpm"))
    double IdleRPM = 850.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Powertrain", meta=(ClampMin="1000.0", Units="rpm"))
    double RedlineRPM = 7200.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Powertrain", meta=(ClampMin="1000.0", Units="rpm"))
    double LimiterRPM = 7400.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Powertrain", meta=(ClampMin="0.01", Units="kg*m^2"))
    double CrankInertiaKgm2 = 0.20;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Clutch", meta=(ClampMin="0.0", Units="N*m"))
    double ClutchMaxTorqueNm = 500.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Gearbox")
    TArray<double> ForwardGearRatios = { 3.45, 2.15, 1.52, 1.16, 0.92, 0.76 };

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Gearbox")
    double ReverseGearRatio = 3.20;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Gearbox")
    double FinalDriveRatio = 3.90;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Gearbox", meta=(ClampMin="0.1", ClampMax="1.0"))
    double MechanicalEfficiency = 0.96;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Drivetrain")
    bool bRearWheelDrive = true;
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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physics|Tire")
    FTAPrototypeTireDefinition Tire;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physics|Wheel")
    FTAPrototypeWheelDefinition Wheel;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physics|Front")
    FTAFrontSuspensionDefinition FrontSuspension;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physics|Rear")
    FTARearSuspensionDefinition RearSuspension;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physics|Drivetrain")
    FTAPrototypeDrivetrainDefinition Drivetrain;

    bool BuildCompiledConfig(FTAVehicleCompiledConfig& OutConfig, FTAValidationResult& OutValidation) const;
};
