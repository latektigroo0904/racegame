#pragma once

#include "CoreMinimal.h"
#include "TAVehicleSimulation.h"

constexpr int32 TAPrototypeTelemetryWheelCount = 4;

struct TA_TELEMETRY_API FTAVehicleTelemetrySample
{
    uint64 SimulationTick = 0;

    double EngineRPM = 0.0;
    int32 SelectedGear = 0;

    double ClutchTemperatureC = 0.0;
    double CoolantTemperatureC = 0.0;
    double CoolingEfficiency01 = 1.0;

    FVector3d ChassisLinearVelocityWorldMps = FVector3d::ZeroVector;
    FVector3d ChassisAngularVelocityWorldRadPerSec = FVector3d::ZeroVector;

    double TotalLongitudinalForceN = 0.0;
    double TotalLateralForceN = 0.0;

    double WheelSlipRatio[TAPrototypeTelemetryWheelCount] = {};
    double WheelSlipAngleRad[TAPrototypeTelemetryWheelCount] = {};
    double TireLongitudinalForceN[TAPrototypeTelemetryWheelCount] = {};
    double TireLateralForceN[TAPrototypeTelemetryWheelCount] = {};

    // Profiling fields are populated by higher-level instrumentation later.
    double VehicleSolverMs = 0.0;
    double TireSolverMs = 0.0;
    double PowertrainSolverMs = 0.0;
    double StructureSolverMs = 0.0;
    double DamageSolverMs = 0.0;
};

class TA_TELEMETRY_API FTATelemetryRingBuffer
{
public:
    bool Initialize(int32 InCapacity);

    void ResetKeepCapacity();

    bool Push(const FTAVehicleTelemetrySample& Sample);

    int32 Num() const
    {
        return Count;
    }

    int32 Capacity() const
    {
        return Samples.Num();
    }

    const FTAVehicleTelemetrySample* GetChronological(int32 Index) const;

private:
    TArray<FTAVehicleTelemetrySample> Samples;

    int32 WriteIndex = 0;
    int32 Count = 0;
};

namespace TATelemetry
{
    TA_TELEMETRY_API FTAVehicleTelemetrySample CaptureVehicleSample(
        const FTAVehicleRuntimeState& State,
        const FTAVehicleStepOutput& Output);
}
