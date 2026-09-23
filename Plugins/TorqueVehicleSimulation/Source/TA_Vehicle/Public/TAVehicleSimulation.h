#pragma once

#include "CoreMinimal.h"
#include "TAChassisDynamics.h"
#include "TADamageTypes.h"
#include "TAPowertrainSolver.h"
#include "TATireSolver.h"

struct TA_VEHICLE_API FTAWheelRuntimeConfig
{
    double RadiusM = 0.327;
    double InertiaKgm2 = 1.20;
    double MaxBrakeTorqueNm = 2200.0;

    bool bDriven = false;
};

struct TA_VEHICLE_API FTAWheelContactInput
{
    double VerticalLoadN = 0.0;
    double LongitudinalVelocityMps = 0.0;
    double LateralVelocityMps = 0.0;
    double CamberRad = 0.0;

    FVector3d ContactPointWorldM = FVector3d::ZeroVector;
    FVector3d ForwardDirectionWorld = FVector3d(1.0, 0.0, 0.0);
    FVector3d RightDirectionWorld = FVector3d(0.0, 1.0, 0.0);

    // Reaction from spring/damper/anti-roll/contact stack, supplied by suspension runtime.
    FVector3d SuspensionForceWorldN = FVector3d::ZeroVector;

    // Optional chassis-side force application point. If false, legacy/canonical
    // contact behavior applies suspension reaction at ContactPointWorldM.
    bool bHasSuspensionForceApplicationPoint = false;

    FVector3d SuspensionForceApplicationPointWorldM =
        FVector3d::ZeroVector;

    FTASurfaceSample Surface;
};

struct TA_VEHICLE_API FTAWheelRuntimeState
{
    double AngularSpeedRadPerSec = 0.0;

    FTATireRuntimeState TireState;
    FTATireSolveOutput LastTireOutput;
};

struct TA_VEHICLE_API FTADriverControls
{
    double Throttle01 = 0.0;
    double Brake01 = 0.0;
    double Steering01 = 0.0;
    double ClutchEngagement01 = 1.0;

    bool bStarterEngaged = false;

    int32 SelectedGear = 0;
};

struct TA_VEHICLE_API FTAVehicleRuntimeConfig
{
    double ReferenceMassKg = 1420.0;

    FTAChassisConfig Chassis;

    TArray<FTAWheelRuntimeConfig> Wheels;
    TArray<FTATireRuntimeConfig> Tires;

    FTAEngineRuntimeConfig Engine;
    FTAEngineThermalConfig EngineThermal;

    FTAClutchRuntimeConfig Clutch;
    FTAGearboxRuntimeConfig Gearbox;
    FTADrivelineComplianceConfig Driveline;

    FTARadiatorDamageConfig Radiator;
};

struct TA_VEHICLE_API FTASteeringRackFunctionalDamageState
{
    double Damage01 = 0.0;

    // Functional command authority only. Geometric rack/pickup displacement
    // remains owned by the structural binding path.
    double CommandAuthority01 = 1.0;

    // Mechanical lash/deadband applied to commanded rack travel.
    double FreePlayM = 0.0;
};

struct TA_VEHICLE_API FTAWheelHubFunctionalDamageState
{
    double Damage01 = 0.0;

    double BrakeEfficiency01 = 1.0;
    double DriveEfficiency01 = 1.0;

    // Opposes wheel rotation; does not replace tire rolling resistance.
    double BearingDragTorqueNm = 0.0;
};

struct TA_VEHICLE_API FTASuspensionFunctionalDamageState
{
    // Force-producing suspension assembly damage only. Hardpoint deformation
    // remains owned by the structural geometry binding path.
    double SpringDamperDamage01 = 0.0;

    double SpringEfficiency01 = 1.0;
    double DampingEfficiency01 = 1.0;
    double StopEfficiency01 = 1.0;

    // A drop-link/bar-link failure reduces axle anti-roll transfer. Because
    // both links are in the load path, axle effectiveness is limited by the
    // weaker side in the four-wheel runtime.
    double AntiRollLinkDamage01 = 0.0;
    double AntiRollLinkEfficiency01 = 1.0;
};

struct TA_VEHICLE_API FTAVehicleRuntimeState
{
    uint64 SimulationTick = 0;

    FTAChassisState Chassis;

    FTAEngineRuntimeState Engine;
    FTAEngineThermalState EngineThermal;

    FTAClutchRuntimeState Clutch;
    FTADrivelineComplianceState Driveline;

    FTARadiatorDamageState Radiator;

    FTASteeringRackFunctionalDamageState SteeringRackDamage;

    int32 SelectedGear = 0;

    TArray<FTAWheelRuntimeState> Wheels;
    TArray<FTAWheelHubFunctionalDamageState> WheelHubDamage;
    TArray<FTASuspensionFunctionalDamageState> SuspensionDamage;
};

struct TA_VEHICLE_API FTAVehicleStepInput
{
    FTADriverControls Controls;
    TArray<FTAWheelContactInput> WheelContacts;
};

struct TA_VEHICLE_API FTAVehicleStepOutput
{
    double TotalLongitudinalForceN = 0.0;
    double TotalLateralForceN = 0.0;
    double TotalAligningMomentNm = 0.0;

    double EngineRPM = 0.0;
    double EngineCoolantTemperatureC = 0.0;
    double EngineThermalTorqueFactor = 1.0;

    double CoolingEfficiency01 = 1.0;
    double CoolantMassKg = 0.0;

    double ClutchSlipRadPerSec = 0.0;

    double LeftDrivenWheelTorqueNm = 0.0;
    double RightDrivenWheelTorqueNm = 0.0;
};

namespace TAVehicleSimulation
{
    TA_VEHICLE_API bool Initialize(
        const FTAVehicleRuntimeConfig& Config,
        FTAVehicleRuntimeState& OutState);

    TA_VEHICLE_API double IntegrateWheelAngularSpeed(
        const FTAWheelRuntimeConfig& Config,
        double CurrentAngularSpeedRadPerSec,
        double DriveTorqueNm,
        double TireReactionTorqueNm,
        double BrakeInput01,
        double DeltaTimeSeconds);

    TA_VEHICLE_API bool Step(
        const FTAVehicleRuntimeConfig& Config,
        const FTAVehicleStepInput& Input,
        double DeltaTimeSeconds,
        FTAVehicleRuntimeState& InOutState,
        FTAVehicleStepOutput& OutOutput);
}
