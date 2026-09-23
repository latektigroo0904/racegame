#include "TATelemetryBuffer.h"
#include "TAFrontAxleRuntime.h"

bool FTATelemetryRingBuffer::Initialize(const int32 InCapacity)
{
    if (InCapacity <= 0)
    {
        Samples.Reset();
        WriteIndex = 0;
        Count = 0;
        return false;
    }

    Samples.SetNum(InCapacity);
    WriteIndex = 0;
    Count = 0;
    return true;
}

void FTATelemetryRingBuffer::ResetKeepCapacity()
{
    WriteIndex = 0;
    Count = 0;
}

bool FTATelemetryRingBuffer::Push(
    const FTAVehicleTelemetrySample& Sample)
{
    if (Samples.Num() <= 0)
    {
        return false;
    }

    Samples[WriteIndex] = Sample;

    WriteIndex = (WriteIndex + 1) % Samples.Num();
    Count = FMath::Min(Count + 1, Samples.Num());

    return true;
}

const FTAVehicleTelemetrySample* FTATelemetryRingBuffer::GetChronological(
    const int32 Index) const
{
    if (Index < 0 || Index >= Count || Samples.Num() <= 0)
    {
        return nullptr;
    }

    const int32 OldestIndex =
        (WriteIndex - Count + Samples.Num()) % Samples.Num();

    const int32 PhysicalIndex =
        (OldestIndex + Index) % Samples.Num();

    return &Samples[PhysicalIndex];
}

FTAVehicleTelemetrySample TATelemetry::CaptureVehicleSample(
    const FTAVehicleRuntimeState& State,
    const FTAVehicleStepOutput& Output)
{
    FTAVehicleTelemetrySample Sample;

    Sample.SimulationTick = State.SimulationTick;

    Sample.EngineRPM = Output.EngineRPM;
    Sample.SelectedGear = State.SelectedGear;

    Sample.ClutchTemperatureC = State.Clutch.TemperatureC;
    Sample.CoolantTemperatureC = Output.EngineCoolantTemperatureC;
    Sample.CoolingEfficiency01 = Output.CoolingEfficiency01;

    Sample.ChassisLinearVelocityWorldMps =
        State.Chassis.LinearVelocityWorldMps;

    Sample.ChassisAngularVelocityWorldRadPerSec =
        State.Chassis.AngularVelocityWorldRadPerSec;

    Sample.TotalLongitudinalForceN =
        Output.TotalLongitudinalForceN;

    Sample.TotalLateralForceN =
        Output.TotalLateralForceN;

    const int32 WheelCount =
        FMath::Min(
            State.Wheels.Num(),
            TAPrototypeTelemetryWheelCount);

    for (int32 Index = 0; Index < WheelCount; ++Index)
    {
        const FTATireSolveOutput& Tire =
            State.Wheels[Index].LastTireOutput;

        Sample.WheelSlipRatio[Index] =
            Tire.SlipRatio;

        Sample.WheelSlipAngleRad[Index] =
            Tire.SlipAngleRad;

        Sample.TireLongitudinalForceN[Index] =
            Tire.LongitudinalForceN;

        Sample.TireLateralForceN[Index] =
            Tire.LateralForceN;
    }

    return Sample;
}


void TATelemetry::ApplyFrontAxleSample(
    const FTAFrontAxleSolveOutput& FrontAxle,
    FTAVehicleTelemetrySample& InOutSample)
{
    InOutSample.SteeringRackDisplacementM =
        FrontAxle.RackDisplacementM;

    InOutSample.FrontLeftSteeringAngleRad =
        FrontAxle.LeftSteeringAngleRad;

    InOutSample.FrontRightSteeringAngleRad =
        FrontAxle.RightSteeringAngleRad;

    InOutSample.FrontLeftBumpSteerRad =
        FrontAxle.LeftBumpSteerRad;

    InOutSample.FrontRightBumpSteerRad =
        FrontAxle.RightBumpSteerRad;

    InOutSample.FrontAckermannDeltaRad =
        FrontAxle.AckermannDeltaRad;
}
