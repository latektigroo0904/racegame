#pragma once

#include "CoreMinimal.h"
#include "TABatteryElectrical.h"
#include "TAStarterMotor.h"

struct TA_VEHICLE_API FTAStartingElectricalConfig
{
    FTABatteryConfig Battery;
    FTAStarterMotorConfig Starter;

    int32 MaxCouplingIterations = 8;
    double VoltageConvergenceToleranceV = 1.0e-4;
};

struct TA_VEHICLE_API FTAStartingElectricalState
{
    FTABatteryState Battery;
};

struct TA_VEHICLE_API FTAStartingElectricalInput
{
    double EngineAngularSpeedRadPerSec = 0.0;
    bool bStarterEngaged = false;
    double StarterHealth01 = 1.0;

    // Other 12 V loads active during cranking.
    double OtherLoadCurrentA = 0.0;

    // Usually small/zero during cranking, but explicit for completeness.
    double AlternatorCurrentA = 0.0;

    double DeltaTimeSeconds = 1.0 / 240.0;
};

struct TA_VEHICLE_API FTAStartingElectricalOutput
{
    double CoupledBusVoltageV = 0.0;
    int32 CouplingIterations = 0;
    bool bConverged = false;

    FTAStarterMotorOutput Starter;
    FTABatteryOutput Battery;
};

namespace TAStartingElectrical
{
    TA_VEHICLE_API bool ValidateConfig(
        const FTAStartingElectricalConfig& Config);

    TA_VEHICLE_API void InitializeState(
        FTAStartingElectricalState& OutState);

    TA_VEHICLE_API bool Step(
        const FTAStartingElectricalConfig& Config,
        const FTAStartingElectricalInput& Input,
        FTAStartingElectricalState& InOutState,
        FTAStartingElectricalOutput& OutOutput);
}
