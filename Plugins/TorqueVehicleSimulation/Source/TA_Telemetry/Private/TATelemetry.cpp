#include "TATelemetry.h"

void FTATelemetryRingBuffer::Initialize(const int32 InCapacity)
{
    Samples.SetNum(FMath::Max(0, InCapacity));
    WriteIndex = 0;
    Count = 0;
}

void FTATelemetryRingBuffer::Reset()
{
    WriteIndex = 0;
    Count = 0;
}

void FTATelemetryRingBuffer::Push(const FTATelemetrySample& Sample)
{
    if (Samples.Num() == 0)
    {
        return;
    }

    Samples[WriteIndex] = Sample;
    WriteIndex = (WriteIndex + 1) % Samples.Num();
    Count = FMath::Min(Count + 1, Samples.Num());
}

int32 FTATelemetryRingBuffer::Num() const
{
    return Count;
}

int32 FTATelemetryRingBuffer::Capacity() const
{
    return Samples.Num();
}

bool FTATelemetryRingBuffer::GetOldest(
    const int32 LogicalIndex,
    FTATelemetrySample& OutSample) const
{
    if (LogicalIndex < 0 || LogicalIndex >= Count || Samples.Num() == 0)
    {
        return false;
    }

    const int32 OldestIndex =
        Count < Samples.Num()
        ? 0
        : WriteIndex;

    const int32 PhysicalIndex =
        (OldestIndex + LogicalIndex) % Samples.Num();

    OutSample = Samples[PhysicalIndex];
    return true;
}

FTATelemetrySample TATelemetry::MakeSample(
    const FTAVehicleRuntimeState& State,
    const FTAVehicleStepOutput& Output)
{
    FTATelemetrySample Sample;

    Sample.SimulationTick = State.SimulationTick;

    Sample.EngineRPM = Output.EngineRPM;
    Sample.EngineCoolantTemperatureC = Output.EngineCoolantTemperatureC;
    Sample.EngineThermalTorqueFactor = Output.EngineThermalTorqueFactor;
    Sample.CoolingEfficiency01 = Output.CoolingEfficiency01;
    Sample.CoolantMassKg = Output.CoolantMassKg;

    Sample.ClutchSlipRadPerSec = Output.ClutchSlipRadPerSec;

    Sample.TotalLongitudinalForceN = Output.TotalLongitudinalForceN;
    Sample.TotalLateralForceN = Output.TotalLateralForceN;
    Sample.TotalAligningMomentNm = Output.TotalAligningMomentNm;

    Sample.LeftDrivenWheelTorqueNm = Output.LeftDrivenWheelTorqueNm;
    Sample.RightDrivenWheelTorqueNm = Output.RightDrivenWheelTorqueNm;

    return Sample;
}
