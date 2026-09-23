#pragma once

#include "CoreMinimal.h"
#include "TACoreTypes.h"
#include "TAFourWheelVehicleRuntime.h"

struct TA_VEHICLE_API FTAVehicleCompiledConfig
{
    FTASimulationVersion Version;

    FName DefinitionId = NAME_None;

    double MassKg = 0.0;
    FVector3d CenterOfMassMeters = FVector3d::ZeroVector;
    FVector3d PrincipalInertiaKgm2 = FVector3d::ZeroVector;

    double LengthMeters = 0.0;
    double WidthMeters = 0.0;
    double HeightMeters = 0.0;
    double WheelbaseMeters = 0.0;
    double TrackFrontMeters = 0.0;
    double TrackRearMeters = 0.0;

    int32 WheelCount = 4;

    FTAVehicleRuntimeConfig VehicleRuntime;
    FTAFourWheelRuntimeConfig FourWheelRuntime;

    uint32 PhysicsConfigHash = 0;
};
