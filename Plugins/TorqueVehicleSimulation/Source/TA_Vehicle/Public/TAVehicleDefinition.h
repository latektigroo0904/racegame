#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TACoreTypes.h"
#include "TAAerodynamicsDefinition.h"
#include "TAVehicleRuntime.h"
#include "TAVehicleStructureDefinition.h"
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
    FVector CenterOfMassMeters = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mass")
    FVector PrincipalInertiaKgm2 = FVector(650.0, 1800.0, 1900.0);
};

USTRUCT(BlueprintType)
struct TA_VEHICLE_API FTAPrototypeTireDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Geometry", meta=(ClampMin="0.10", Units="m"))
    double UnloadedRadiusM = 0.327;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Force", meta=(ClampMin="1.0", Units="N"))
    double ReferenceLoadN = 3500.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Force", meta=(ClampMin="0.1"))
    double DryPeakMu = 1.05;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Force", meta=(ClampMin="0.0"))
    double LoadSensitivityExponent = 0.08;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Force", meta=(ClampMin="1.0", Units="N"))
    double LongitudinalStiffnessN = 90000.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Force", meta=(ClampMin="1.0", Units="N/rad"))
    double CorneringStiffnessNPerRad = 85000.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Force", meta=(ClampMin="0.0", Units="N/rad"))
    double CamberStiffnessNPerRad = 6000.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Force", meta=(ClampMin="0.1"))
    double SaturationExponent = 2.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Force", meta=(ClampMin="0.0", Units="m"))
    double PneumaticTrailM = 0.055;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Force", meta=(ClampMin="0.0"))
    double RollingResistanceCoefficient = 0.012;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Pressure", meta=(ClampMin="1.0", Units="kPa"))
    double ReferencePressureKPa = 230.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Pressure", meta=(Units="C"))
    double ReferencePressureTemperatureC = 20.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Vertical", meta=(ClampMin="1.0", Units="N/m"))
    double RadialStiffnessNPerM = 220000.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Vertical", meta=(ClampMin="0.0", Units="N/m^2"))
    double RadialProgressiveStiffnessNPerM2 = 1200000.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Vertical", meta=(ClampMin="0.0", Units="N*s/m"))
    double RadialDampingNsPerM = 2200.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Vertical", meta=(ClampMin="0.005", Units="m"))
    double MaxRadialDeflectionM = 0.060;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Vertical", meta=(ClampMin="0.0"))
    double PressureRadialStiffnessExponent = 0.45;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Wear", meta=(ClampMin="0.0", Units="mm"))
    double NewTreadDepthMm = 7.5;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Wear", meta=(ClampMin="0.0", Units="mm"))
    double MinimumTreadDepthMm = 0.8;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Grip", meta=(Units="C"))
    double OptimalSurfaceTemperatureC = 85.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Grip", meta=(ClampMin="0.0"))
    double ColdGripMultiplier = 0.78;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Grip", meta=(ClampMin="0.0"))
    double HotGripMultiplier = 0.82;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Grip", meta=(Units="C"))
    double HotGripTemperatureC = 135.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Grip", meta=(ClampMin="0.0"))
    double PressureGripSensitivity = 0.20;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Grip", meta=(ClampMin="0.0", ClampMax="1.0"))
    double WearGripLossAtEnd = 0.15;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Thermal", meta=(ClampMin="1.0", Units="J/C"))
    double SurfaceThermalMassJPerC = 4200.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Thermal", meta=(ClampMin="1.0", Units="J/C"))
    double CarcassThermalMassJPerC = 12000.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Thermal", meta=(ClampMin="0.0", Units="W/C"))
    double SurfaceToCarcassConductanceWPerC = 95.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Thermal", meta=(ClampMin="0.0", Units="W/C"))
    double CarcassToAmbientConductanceWPerC = 38.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Thermal", meta=(ClampMin="0.01", Units="s"))
    double InternalAirTimeConstantSeconds = 35.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Thermal", meta=(ClampMin="0.0", ClampMax="1.0"))
    double SlipHeatFraction = 0.18;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Thermal", meta=(ClampMin="0.0", ClampMax="1.0"))
    double RollingHeatFraction = 0.50;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Wear", meta=(ClampMin="1.0", Units="J"))
    double WearEnergyCapacityJ = 8.0e7;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Thermal", meta=(Units="C"))
    double ThermalDegradationStartC = 135.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Thermal", meta=(ClampMin="0.0"))
    double ThermalDegradationRatePerSecondAt170C = 0.0015;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Slip", meta=(ClampMin="0.01", Units="m/s"))
    double SlipReferenceVelocityMps = 0.5;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Slip", meta=(ClampMin="0.0", Units="m/s"))
    double DynamicBlendStartMps = 2.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Slip", meta=(ClampMin="0.0", Units="m/s"))
    double DynamicBlendEndMps = 5.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Hydro", meta=(ClampMin="0.1", Units="m/s"))
    double HydroReferenceOnsetSpeedMps = 32.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tire|Hydro", meta=(ClampMin="0.1", Units="mm"))
    double HydroReferenceWaterDepthMm = 4.0;
};

USTRUCT(BlueprintType)
struct TA_VEHICLE_API FTAPrototypeWheelDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Wheel", meta=(ClampMin="0.01", Units="kg*m^2"))
    double InertiaKgm2 = 1.20;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Brakes", meta=(ClampMin="0.0", Units="N*m"))
    double MaxBrakeTorqueNm = 2200.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Brakes|Thermal", meta=(ClampMin="1.0", Units="J/C"))
    double BrakeThermalMassJPerC = 35000.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Brakes|Thermal", meta=(ClampMin="0.0", Units="W/C"))
    double BrakeCoolingWPerC = 70.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Brakes|Thermal", meta=(Units="C"))
    double BrakeAmbientTemperatureC = 20.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Brakes|Thermal", meta=(ClampMin="0.0", ClampMax="1.0"))
    double BrakeHeatFraction01 = 0.95;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Brakes|Fade", meta=(Units="C"))
    double BrakeFadeStartTemperatureC = 450.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Brakes|Fade", meta=(Units="C"))
    double BrakeFadeEndTemperatureC = 750.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Brakes|Fade", meta=(ClampMin="0.0", ClampMax="1.0"))
    double MinimumBrakeFadeTorqueFactor01 = 0.35;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Brakes|Wear", meta=(ClampMin="1.0", Units="J"))
    double BrakeWearEnergyCapacityJ = 4.0e8;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Brakes|Wear", meta=(ClampMin="0.0", ClampMax="1.0"))
    double BrakeWearTorqueLossAtEnd01 = 0.25;
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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Front|Solver", meta=(ClampMin="1", ClampMax="512"))
    int32 MaxSolverIterations = 80;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Front|Solver", meta=(ClampMin="0.00001", Units="m"))
    double PositionToleranceM = 0.0005;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Front|Spring", meta=(ClampMin="0.0", Units="N/m"))
    double SpringRateNPerM = 100000.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Front|Spring", meta=(ClampMin="0.0", Units="m"))
    double StaticSpringCompressionM = 0.0810;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Front|Damper", meta=(ClampMin="0.0", Units="N*s/m"))
    double BumpDampingNsPerM = 4500.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Front|Damper", meta=(ClampMin="0.0", Units="N*s/m"))
    double ReboundDampingNsPerM = 6500.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Front|Stops", meta=(ClampMin="0.0", Units="N/m"))
    double BumpStopRateNPerM = 180000.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Front|Stops", meta=(ClampMin="0.0", Units="N/m"))
    double DroopStopRateNPerM = 120000.0;

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

    FTARearSuspensionDefinition();

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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rear|Solver", meta=(ClampMin="1", ClampMax="512"))
    int32 MaxSolverIterations = 180;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rear|Solver", meta=(ClampMin="0.00001", Units="m"))
    double PositionToleranceM = 0.001;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rear|Spring", meta=(ClampMin="0.0", Units="N/m"))
    double SpringRateNPerM = 50000.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rear|Spring", meta=(ClampMin="0.0", Units="m"))
    double StaticSpringCompressionM = 0.0987;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rear|Damper", meta=(ClampMin="0.0", Units="N*s/m"))
    double BumpDampingNsPerM = 3800.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rear|Damper", meta=(ClampMin="0.0", Units="N*s/m"))
    double ReboundDampingNsPerM = 5200.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rear|Stops", meta=(ClampMin="0.0", Units="N/m"))
    double BumpStopRateNPerM = 180000.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rear|Stops", meta=(ClampMin="0.0", Units="N/m"))
    double DroopStopRateNPerM = 120000.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rear|AntiRoll", meta=(ClampMin="0.0", Units="N/m"))
    double AntiRollCouplingRateNPerM = 9000.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rear|AntiRoll", meta=(ClampMin="0.0", Units="N"))
    double AntiRollMaxTransferForceN = 2500.0;
};

USTRUCT(BlueprintType)
struct TA_VEHICLE_API FTAEngineTorqueDefinitionPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Engine", meta=(ClampMin="0.0", Units="rpm"))
    double RPM = 0.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Engine", meta=(Units="N*m"))
    double TorqueNm = 0.0;
};

USTRUCT(BlueprintType)
struct TA_VEHICLE_API FTACoolingSystemAuthoringDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cooling", meta=(ClampMin="0.0", Units="kg"))
    double InitialCoolantMassKg = 6.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cooling|Damage", meta=(ClampMin="0.0", Units="J"))
    double PunctureThresholdEnergyJ = 2500.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cooling|Damage", meta=(ClampMin="0.0", Units="J"))
    double FullLeakEnergyJ = 20000.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cooling|Damage", meta=(ClampMin="0.0", Units="mm^2"))
    double MaxLeakAreaMm2 = 20.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cooling|Leak", meta=(ClampMin="0.0"))
    double LeakMassFlowKgPerSecPerMm2 = 0.003;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cooling|Damage", meta=(ClampMin="0.0", ClampMax="1.0"))
    double MinimumAirflowEfficiency01 = 0.15;
};

USTRUCT(BlueprintType)
struct TA_VEHICLE_API FTAEngineThermalAuthoringDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Engine|Thermal", meta=(Units="C"))
    double AmbientTemperatureC = 20.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Engine|Thermal", meta=(Units="C"))
    double InitialCoolantTemperatureC = 90.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Engine|Thermal", meta=(ClampMin="1.0", Units="J/C"))
    double EffectiveThermalMassJPerC = 70000.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Engine|Thermal", meta=(ClampMin="0.0", Units="W"))
    double BaseHeatGenerationW = 12000.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Engine|Thermal", meta=(ClampMin="0.0", Units="W"))
    double FullLoadAdditionalHeatW = 95000.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Engine|Thermal", meta=(ClampMin="0.0", Units="W/C"))
    double CoolingCapacityWPerC = 1450.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Engine|Thermal", meta=(Units="C"))
    double DerateStartTemperatureC = 110.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Engine|Thermal", meta=(Units="C"))
    double DerateFullTemperatureC = 135.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Engine|Thermal", meta=(ClampMin="0.0", ClampMax="1.0"))
    double MinimumThermalTorqueFactor = 0.40;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Engine|Thermal", meta=(Units="C"))
    double DamageStartTemperatureC = 125.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Engine|Thermal", meta=(ClampMin="0.0"))
    double DamageRatePerSecondAt150C = 0.0025;
};

USTRUCT(BlueprintType)
struct TA_VEHICLE_API FTAPrototypeDrivetrainDefinition
{
    GENERATED_BODY()

    FTAPrototypeDrivetrainDefinition();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Powertrain", meta=(ClampMin="100.0", Units="rpm"))
    double IdleRPM = 850.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Powertrain", meta=(ClampMin="1000.0", Units="rpm"))
    double RedlineRPM = 7200.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Powertrain", meta=(ClampMin="1000.0", Units="rpm"))
    double LimiterRPM = 7400.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Powertrain", meta=(ClampMin="0.01", Units="kg*m^2"))
    double CrankInertiaKgm2 = 0.20;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Engine|Friction", meta=(ClampMin="0.0", Units="N*m"))
    double FrictionConstantNm = 8.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Engine|Friction", meta=(ClampMin="0.0"))
    double FrictionLinearNms = 0.02;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Engine|Friction", meta=(ClampMin="0.0"))
    double FrictionQuadraticNms2 = 0.00002;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Engine|RunState", meta=(ClampMin="0.0", Units="rpm"))
    double StallRPM = 450.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Engine|RunState", meta=(ClampMin="0.0", Units="rpm"))
    double CombustionStartRPM = 650.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Engine|Starter", meta=(ClampMin="0.0", Units="N*m"))
    double StarterTorqueNm = 95.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Engine|Starter", meta=(ClampMin="0.0", Units="rpm"))
    double StarterMaxRPM = 900.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Engine|IdleControl", meta=(ClampMin="0.0"))
    double IdleControlGainNmPerRPM = 0.08;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Engine|IdleControl", meta=(ClampMin="0.0", Units="N*m"))
    double MaxIdleControlTorqueNm = 90.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Powertrain")
    TArray<FTAEngineTorqueDefinitionPoint> TorqueCurve;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Clutch", meta=(ClampMin="0.0", Units="N*m"))
    double ClutchMaxTorqueNm = 500.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Clutch", meta=(ClampMin="0.0"))
    double ClutchCouplingStiffnessNms = 20.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Clutch|Thermal", meta=(ClampMin="1.0", Units="J/C"))
    double ClutchThermalMassJPerC = 12000.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Clutch|Thermal", meta=(ClampMin="0.0", Units="W/C"))
    double ClutchCoolingWPerC = 18.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Clutch|Thermal", meta=(Units="C"))
    double ClutchAmbientTemperatureC = 20.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Clutch|Thermal", meta=(Units="C"))
    double ClutchFadeStartTemperatureC = 220.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Clutch|Thermal", meta=(Units="C"))
    double ClutchFadeEndTemperatureC = 420.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Clutch|Wear", meta=(ClampMin="1.0", Units="J"))
    double ClutchWearEnergyCapacityJ = 1.0e8;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Gearbox")
    TArray<double> ForwardGearRatios;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Gearbox")
    double ReverseGearRatio = 3.20;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Gearbox")
    double FinalDriveRatio = 3.90;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Gearbox", meta=(ClampMin="0.1", ClampMax="1.0"))
    double MechanicalEfficiency = 0.96;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Driveline|Compliance", meta=(ClampMin="0.0", Units="N*m/rad"))
    double DrivelineTorsionalStiffnessNmPerRad = 2500.0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Driveline|Compliance", meta=(ClampMin="0.0"))
    double DrivelineTorsionalDampingNmsPerRad = 25.0;

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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physics|Thermal")
    FTAEngineThermalAuthoringDefinition EngineThermal;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physics|Cooling")
    FTACoolingSystemAuthoringDefinition Cooling;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physics|Structure")
    FTAVehicleStructureAuthoringDefinition Structure;

    /**
     * Vehicle aerodynamic calibration. The authored application point is
     * vehicle-origin-local; BuildCompiledConfig converts it to COM-local
     * exactly once through TAVehicleAerodynamicsAssetCompiler.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physics|Aerodynamics")
    FTAAerodynamicsDefinition Aerodynamics;

    bool BuildCompiledConfig(FTAVehicleCompiledConfig& OutConfig, FTAValidationResult& OutValidation) const;
};
