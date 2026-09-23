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

    // Exact aerodynamic result applied by the vehicle step. These values are
    // copied from FTAVehicleStepOutput::Aerodynamics and are never recomputed
    // inside telemetry, preserving single-source-of-truth physics reporting.
    double AeroRelativeAirSpeedMps = 0.0;
    double AeroDynamicPressurePa = 0.0;
    FVector3d AeroForceWorldN = FVector3d::ZeroVector;
    FVector3d AeroTorqueWorldNm = FVector3d::ZeroVector;
};

class TA_TELEMETRY_API FTACompactTelemetryRingBuffer
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
