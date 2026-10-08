#pragma once

#include "CoreMinimal.h"

struct TA_VEHICLE_API FTATractionControlConfig
{
    double MinimumVehicleSpeedMps = 1.5;
    double MinimumThrottle01 = 0.05;

    double EnterSlipRatio = 0.16;
    double RecoverSlipRatio = 0.08;

    double TorqueReductionRate01PerSec = 6.0;
    double TorqueRecoveryRate01PerSec = 2.5;

    double MinimumEngineTorqueAuthority01 = 0.20;
};

struct TA_VEHICLE_API FTATractionControlState
{
    double EngineTorqueAuthority01 = 1.0;
};

struct TA_VEHICLE_API FTATractionControlInput
{
    double Throttle01 = 0.0;
    double VehicleSpeedMps = 0.0;

    // Maximum positive propulsion slip among driven wheels.
    double MaxDrivenSlipRatio = 0.0;

    bool bDrivenContactValid = true;

    double DeltaTimeSeconds = 1.0 / 240.0;
};

struct TA_VEHICLE_API FTATractionControlOutput
{
    double EngineTorqueAuthority01 = 1.0;
    bool bReducingTorque = false;
    bool bRecoveringTorque = false;
};

struct TA_VEHICLE_API FTAStabilityControlConfig
{
    double WheelbaseM = 2.62;
    double MinimumVehicleSpeedMps = 3.0;

    double MaxReferenceLateralAccelerationMps2 = 8.0;

    double YawRateDeadbandRadPerSec = 0.03;
    double YawMomentGainNmPerRadPerSec = 1800.0;
    double MaxCorrectiveYawMomentNm = 1800.0;
};

struct TA_VEHICLE_API FTAStabilityControlInput
{
    double RoadWheelSteerAngleRad = 0.0;
    double VehicleSpeedMps = 0.0;
    double MeasuredYawRateRadPerSec = 0.0;
};

struct TA_VEHICLE_API FTAStabilityControlOutput
{
    double ReferenceYawRateRadPerSec = 0.0;
    double YawRateErrorRadPerSec = 0.0;

    // Request only. A later actuator allocator must realize this through
    // hydraulic brakes and/or active differential torque.
    double DesiredCorrectiveYawMomentNm = 0.0;
};

namespace TADriverAssistControllers
{
    TA_VEHICLE_API bool ValidateTractionConfig(
        const FTATractionControlConfig& Config);

    TA_VEHICLE_API void InitializeTractionState(
        FTATractionControlState& OutState);

    TA_VEHICLE_API bool StepTractionControl(
        const FTATractionControlConfig& Config,
        const FTATractionControlInput& Input,
        FTATractionControlState& InOutState,
        FTATractionControlOutput& OutOutput);

    TA_VEHICLE_API bool ValidateStabilityConfig(
        const FTAStabilityControlConfig& Config);

    TA_VEHICLE_API bool CalculateStabilityRequest(
        const FTAStabilityControlConfig& Config,
        const FTAStabilityControlInput& Input,
        FTAStabilityControlOutput& OutOutput);
}
