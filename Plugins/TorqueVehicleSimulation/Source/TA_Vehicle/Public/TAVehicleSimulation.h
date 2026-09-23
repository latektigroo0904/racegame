#pragma once

#include "CoreMinimal.h"
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
    double ClutchEngagement01 = 1.0;

    int32 SelectedGear = 0;
};

struct TA_VEHICLE_API FTAVehicleRuntimeConfig
{
    double ReferenceMassKg = 1420.0;

    TArray<FTAWheelRuntimeConfig> Wheels;
    TArray<FTATireRuntimeConfig> Tires;

    FTAEngineRuntimeConfig Engine;
    FTAClutchRuntimeConfig Clutch;
    FTAGearboxRuntimeConfig Gearbox;
    FTADrivelineComplianceConfig Driveline;
};

struct TA_VEHICLE_API FTAVehicleRuntimeState
{
    uint64 SimulationTick = 0;

    FTAEngineRuntimeState Engine;
    FTAClutchRuntimeState Clutch;
    FTADrivelineComplianceState Driveline;

    int32 SelectedGear = 0;

    TArray<FTAWheelRuntimeState> Wheels;
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
