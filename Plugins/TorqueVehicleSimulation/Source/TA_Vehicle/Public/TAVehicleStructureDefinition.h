#pragma once

#include "CoreMinimal.h"
#include "TAVehicleStructureDefinition.generated.h"

UENUM(BlueprintType)
enum class ETAVehicleDamageConsumerAuthoringType : uint8
{
    Radiator,
    SteeringRack,
    WheelHub,
    SuspensionCorner,
    AntiRollLink
};

USTRUCT(BlueprintType)
struct TA_VEHICLE_API FTAStructureNodeAuthoringDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure")
    FVector PositionVehicleLocalM = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure", meta=(ClampMin="0.001", Units="kg"))
    double MassKg = 10.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure")
    bool bPinned = false;
};

USTRUCT(BlueprintType)
struct TA_VEHICLE_API FTAStructureConstraintAuthoringDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure|Constraint")
    int32 NodeA = INDEX_NONE;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure|Constraint")
    int32 NodeB = INDEX_NONE;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure|Constraint", meta=(ClampMin="0.0"))
    double Compliance = 0.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure|Constraint", meta=(ClampMin="0.0"))
    double YieldStrain = 0.08;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure|Constraint", meta=(ClampMin="0.0"))
    double FractureStrain = 0.30;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure|Constraint", meta=(ClampMin="0.0", ClampMax="1.0"))
    double PlasticFlowRate01 = 0.25;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure|Constraint")
    int32 TargetComponentIndex = INDEX_NONE;
};

USTRUCT(BlueprintType)
struct TA_VEHICLE_API FTAStructureDisplacementBindingAuthoringDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure|Binding")
    TArray<int32> NodeIndices;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure|Binding")
    TArray<double> Weights;
};

USTRUCT(BlueprintType)
struct TA_VEHICLE_API FTADoubleWishboneStructuralBindingsAuthoringDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure|FrontBinding")
    FTAStructureDisplacementBindingAuthoringDefinition UpperInnerA;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure|FrontBinding")
    FTAStructureDisplacementBindingAuthoringDefinition UpperInnerB;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure|FrontBinding")
    FTAStructureDisplacementBindingAuthoringDefinition LowerInnerA;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure|FrontBinding")
    FTAStructureDisplacementBindingAuthoringDefinition LowerInnerB;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure|FrontBinding")
    FTAStructureDisplacementBindingAuthoringDefinition TieRodInner;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure|FrontBinding")
    FTAStructureDisplacementBindingAuthoringDefinition DamperChassis;
};

USTRUCT(BlueprintType)
struct TA_VEHICLE_API FTAMultiLinkStructuralBindingsAuthoringDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure|RearBinding")
    FTAStructureDisplacementBindingAuthoringDefinition Link0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure|RearBinding")
    FTAStructureDisplacementBindingAuthoringDefinition Link1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure|RearBinding")
    FTAStructureDisplacementBindingAuthoringDefinition Link2;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure|RearBinding")
    FTAStructureDisplacementBindingAuthoringDefinition Link3;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure|RearBinding")
    FTAStructureDisplacementBindingAuthoringDefinition Link4;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure|RearBinding")
    FTAStructureDisplacementBindingAuthoringDefinition DamperChassis;
};

USTRUCT(BlueprintType)
struct TA_VEHICLE_API FTAStructureMountDamageAuthoringDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage|Mount")
    int32 TargetComponentIndex = INDEX_NONE;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage|Mount")
    TArray<int32> NodeIndices;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage|Mount")
    TArray<double> Weights;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage|Mount")
    TArray<double> DisplacementThresholdsM;
};

USTRUCT(BlueprintType)
struct TA_VEHICLE_API FTAVehicleDamageRouteAuthoringDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage|Route")
    int32 TargetComponentIndex = INDEX_NONE;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage|Route")
    ETAVehicleDamageConsumerAuthoringType Consumer =
        ETAVehicleDamageConsumerAuthoringType::Radiator;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage|Route")
    bool bAcceptImpactEnergy = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage|Route")
    bool bAcceptStructuralDisplacement = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage|Route")
    bool bAcceptStructuralFracture = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage|Route", meta=(ClampMin="0.0"))
    double ImpactEnergyScale = 1.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage|Route", meta=(ClampMin="0.001", Units="J"))
    double FullDamageEnergyJ = 12000.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage|Route", meta=(ClampMin="0.001", Units="m"))
    double FullCrushDisplacementM = 0.15;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage|Route")
    int32 WheelIndex = INDEX_NONE;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage|Route|Steering", meta=(ClampMin="0.0", ClampMax="1.0"))
    double MinimumSteeringAuthority01 = 0.20;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage|Route|Steering", meta=(ClampMin="0.0", Units="m"))
    double MaximumSteeringFreePlayM = 0.010;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage|Route|Hub", meta=(ClampMin="0.0", ClampMax="1.0"))
    double MinimumBrakeEfficiency01 = 0.20;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage|Route|Hub", meta=(ClampMin="0.0", ClampMax="1.0"))
    double MinimumDriveEfficiency01 = 0.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage|Route|Hub", meta=(ClampMin="0.0", Units="N*m"))
    double MaximumBearingDragTorqueNm = 80.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage|Route|Suspension", meta=(ClampMin="0.0", ClampMax="1.0"))
    double MinimumSpringEfficiency01 = 0.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage|Route|Suspension", meta=(ClampMin="0.0", ClampMax="1.0"))
    double MinimumDampingEfficiency01 = 0.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage|Route|Suspension", meta=(ClampMin="0.0", ClampMax="1.0"))
    double MinimumStopEfficiency01 = 0.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage|Route|AntiRoll", meta=(ClampMin="0.0", ClampMax="1.0"))
    double MinimumAntiRollLinkEfficiency01 = 0.0;
};

USTRUCT(BlueprintType)
struct TA_VEHICLE_API FTAVehicleStructureAuthoringDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure")
    TArray<FTAStructureNodeAuthoringDefinition> Nodes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure")
    TArray<FTAStructureConstraintAuthoringDefinition> Constraints;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure|Solver", meta=(ClampMin="1", ClampMax="64"))
    int32 ConstraintIterations = 8;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure|Solver", meta=(ClampMin="0.0", Units="m"))
    double MaxPositionCorrectionM = 0.10;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure|Solver", meta=(ClampMin="0.0"))
    double MaxPlasticRestChangeFractionPerStep = 0.03;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure|Impact", meta=(ClampMin="0.001", Units="m"))
    double ImpactDistributionRadiusM = 0.75;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure|Impact", meta=(ClampMin="0.01"))
    double ImpactDistanceFalloffExponent = 2.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure|Impact", meta=(ClampMin="0.0", ClampMax="1.0"))
    double DeformationImpulseFraction01 = 0.35;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure|Impact", meta=(ClampMin="0.0", ClampMax="1.0"))
    double MaxDeformationEnergyFraction01 = 0.65;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure|Impact", meta=(ClampMin="0.0", Units="m/s"))
    double MaxNodeDeltaVelocityMps = 35.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure|Damage")
    int32 ImpactTargetComponentIndex = INDEX_NONE;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure|Damage")
    TArray<FTAStructureMountDamageAuthoringDefinition> MountDamageBindings;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure|Damage")
    TArray<FTAVehicleDamageRouteAuthoringDefinition> DamageRoutes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure|Suspension")
    FTADoubleWishboneStructuralBindingsAuthoringDefinition FrontLeftSuspensionBindings;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure|Suspension")
    FTADoubleWishboneStructuralBindingsAuthoringDefinition FrontRightSuspensionBindings;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure|Suspension")
    FTAMultiLinkStructuralBindingsAuthoringDefinition RearLeftSuspensionBindings;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Structure|Suspension")
    FTAMultiLinkStructuralBindingsAuthoringDefinition RearRightSuspensionBindings;
};
