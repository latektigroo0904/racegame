#pragma once

#include "CoreMinimal.h"
#include "TAVehicleSimulation.h"

struct TA_TELEMETRY_API FTATelemetrySample
{
    uint64 SimulationTick = 0;

    double EngineRPM = 0.0;
    double EngineCoolantTemperatureC = 0.0;
    double EngineThermalTorqueFactor = 1.0;
    double CoolingEfficiency01 = 1.0;
    double CoolantMassKg = 0.0;

    double ClutchSlipRadPerSec = 0.0;

    double TotalLongitudinalForceN = 0.0;
    double TotalLateralForceN = 0.0;
    double TotalAligningMomentNm = 0.0;

    double LeftDrivenWheelTorqueNm = 0.0;
    double RightDrivenWheelTorqueNm = 0.0;
};

class TA_TELEMETRY_API FTATelemetryRingBuffer
{
public:
    void Initialize(int32 InCapacity);
    void Reset();

    void Push(const FTATelemetrySample& Sample);

    int32 Num() const;
    int32 Capacity() const;

    bool GetOldest(int32 LogicalIndex, FTATelemetrySample& OutSample) const;

private:
    TArray<FTATelemetrySample> Samples;
    int32 WriteIndex = 0;
    int32 Count = 0;
};

namespace TATelemetry
{
    TA_TELEMETRY_API FTATelemetrySample MakeSample(
        const FTAVehicleRuntimeState& State,
        const FTAVehicleStepOutput& Output);
}
