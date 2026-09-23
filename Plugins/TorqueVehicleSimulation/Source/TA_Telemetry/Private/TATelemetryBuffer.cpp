#include "TATelemetryBuffer.h"
#include "TAFrontAxleRuntime.h"
#include "TAFourWheelVehicleRuntime.h"
#include "TAVehicleRuntime.h"

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

FString FTATelemetryRingBuffer::ExportCsv() const
{
    FString Csv;

    Csv += TEXT(
        "tick,physics_config_hash,engine_rpm,gear,"
        "vx_mps,vy_mps,vz_mps,"
        "wx_radps,wy_radps,wz_radps,"
        "fx_total_n,fy_total_n,"
        "rack_m,rack_damage,rack_authority,rack_freeplay_m,"
        "steer_fl_rad,steer_fr_rad,"
        "bumpsteer_fl_rad,bumpsteer_fr_rad,ackermann_delta_rad");

    const TCHAR* WheelNames[TAPrototypeTelemetryWheelCount] =
    {
        TEXT("fl"),
        TEXT("fr"),
        TEXT("rl"),
        TEXT("rr")
    };

    for (int32 Wheel = 0;
         Wheel < TAPrototypeTelemetryWheelCount;
         ++Wheel)
    {
        Csv.Appendf(
            TEXT(
                ",load_%s_n,travel_%s_m,camber_%s_rad,toe_%s_rad,"
                "hubdamage_%s,hubbrake_%s,hubdrive_%s,hubdrag_%s_nm,"
                "slipratio_%s,slipangle_%s_rad,"
                "tirefx_%s_n,tirefy_%s_n,"
                "tiretemp_%s_c,tirepressure_%s_kpa,"
                "tirewear_%s,tiredeflection_%s_m"),
            WheelNames[Wheel],
            WheelNames[Wheel],
            WheelNames[Wheel],
            WheelNames[Wheel],
            WheelNames[Wheel],
            WheelNames[Wheel],
            WheelNames[Wheel],
            WheelNames[Wheel],
            WheelNames[Wheel],
            WheelNames[Wheel],
            WheelNames[Wheel],
            WheelNames[Wheel],
            WheelNames[Wheel],
            WheelNames[Wheel],
            WheelNames[Wheel],
            WheelNames[Wheel]);
    }

    Csv += TEXT("\n");

    for (int32 SampleIndex = 0;
         SampleIndex < Count;
         ++SampleIndex)
    {
        const FTAVehicleTelemetrySample* Sample =
            GetChronological(SampleIndex);

        if (!Sample)
        {
            continue;
        }

        Csv.Appendf(
            TEXT(
                "%llu,%u,%.9g,%d,"
                "%.9g,%.9g,%.9g,"
                "%.9g,%.9g,%.9g,"
                "%.9g,%.9g,"
                "%.9g,%.9g,%.9g,%.9g,"
                "%.9g,%.9g,"
                "%.9g,%.9g,%.9g"),
            static_cast<unsigned long long>(
                Sample->SimulationTick),
            Sample->PhysicsConfigHash,
            Sample->EngineRPM,
            Sample->SelectedGear,
            Sample->ChassisLinearVelocityWorldMps.X,
            Sample->ChassisLinearVelocityWorldMps.Y,
            Sample->ChassisLinearVelocityWorldMps.Z,
            Sample->ChassisAngularVelocityWorldRadPerSec.X,
            Sample->ChassisAngularVelocityWorldRadPerSec.Y,
            Sample->ChassisAngularVelocityWorldRadPerSec.Z,
            Sample->TotalLongitudinalForceN,
            Sample->TotalLateralForceN,
            Sample->SteeringRackDisplacementM,
            Sample->SteeringRackDamage01,
            Sample->SteeringCommandAuthority01,
            Sample->SteeringRackFreePlayM,
            Sample->FrontLeftSteeringAngleRad,
            Sample->FrontRightSteeringAngleRad,
            Sample->FrontLeftBumpSteerRad,
            Sample->FrontRightBumpSteerRad,
            Sample->FrontAckermannDeltaRad);

        for (int32 Wheel = 0;
             Wheel < TAPrototypeTelemetryWheelCount;
             ++Wheel)
        {
            Csv.Appendf(
                TEXT(
                    ",%.9g,%.9g,%.9g,%.9g,"
                    "%.9g,%.9g,%.9g,%.9g,"
                    "%.9g,%.9g,%.9g,%.9g,"
                    "%.9g,%.9g,%.9g,%.9g"),
                Sample->WheelVerticalLoadN[Wheel],
                Sample->SuspensionTravelM[Wheel],
                Sample->WheelCamberRad[Wheel],
                Sample->WheelToeRad[Wheel],
                Sample->WheelHubDamage01[Wheel],
                Sample->WheelHubBrakeEfficiency01[Wheel],
                Sample->WheelHubDriveEfficiency01[Wheel],
                Sample->WheelHubBearingDragTorqueNm[Wheel],
                Sample->WheelSlipRatio[Wheel],
                Sample->WheelSlipAngleRad[Wheel],
                Sample->TireLongitudinalForceN[Wheel],
                Sample->TireLateralForceN[Wheel],
                Sample->TireSurfaceTemperatureC[Wheel],
                Sample->TirePressureKPa[Wheel],
                Sample->TireWear01[Wheel],
                Sample->TireRadialDeflectionM[Wheel]);
        }

        Csv += TEXT("\n");
    }

    return Csv;
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

    Sample.SteeringRackDamage01 =
        State.SteeringRackDamage.Damage01;

    Sample.SteeringCommandAuthority01 =
        State.SteeringRackDamage.CommandAuthority01;

    Sample.SteeringRackFreePlayM =
        State.SteeringRackDamage.FreePlayM;

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

        const FTATireRuntimeState& TireState =
            State.Wheels[Index].TireState;

        Sample.TireSurfaceTemperatureC[Index] =
            TireState.SurfaceTemperatureC;

        Sample.TirePressureKPa[Index] =
            TireState.PressureKPa;

        Sample.TireWear01[Index] =
            TireState.Wear01;

        Sample.TireRadialDeflectionM[Index] =
            TireState.RadialDeflectionM;

        if (State.WheelHubDamage.IsValidIndex(Index))
        {
            const FTAWheelHubFunctionalDamageState& Hub =
                State.WheelHubDamage[Index];

            Sample.WheelHubDamage01[Index] =
                Hub.Damage01;

            Sample.WheelHubBrakeEfficiency01[Index] =
                Hub.BrakeEfficiency01;

            Sample.WheelHubDriveEfficiency01[Index] =
                Hub.DriveEfficiency01;

            Sample.WheelHubBearingDragTorqueNm[Index] =
                Hub.BearingDragTorqueNm;
        }
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


void TATelemetry::ApplyFourWheelSample(
    const FTAFourWheelStepOutput& FourWheel,
    FTAVehicleTelemetrySample& InOutSample)
{
    ApplyFrontAxleSample(
        FourWheel.FrontAxle,
        InOutSample);

    const FTAResolvedWheelContact* FrontContacts[2] =
    {
        &FourWheel.FrontAxle.LeftContact,
        &FourWheel.FrontAxle.RightContact
    };

    for (int32 Index = 0; Index < 2; ++Index)
    {
        InOutSample.WheelVerticalLoadN[Index] =
            FrontContacts[Index]->VerticalLoadN;

        InOutSample.SuspensionTravelM[Index] =
            FrontContacts[Index]->TravelM;

        InOutSample.WheelCamberRad[Index] =
            FrontContacts[Index]->Geometry.CamberRad;

        InOutSample.WheelToeRad[Index] =
            FrontContacts[Index]->Geometry.ToeRad;

        InOutSample.TireRadialDeflectionM[Index] =
            FrontContacts[Index]->TireRadialDeflectionM;
    }

    const FTAResolvedMultiLinkContact* RearContacts[2] =
    {
        &FourWheel.RearAxle.LeftContact,
        &FourWheel.RearAxle.RightContact
    };

    for (int32 Index = 0; Index < 2; ++Index)
    {
        const int32 WheelIndex = Index + 2;

        InOutSample.WheelVerticalLoadN[WheelIndex] =
            RearContacts[Index]->VerticalLoadN;

        InOutSample.SuspensionTravelM[WheelIndex] =
            RearContacts[Index]->TravelM;

        InOutSample.WheelCamberRad[WheelIndex] =
            RearContacts[Index]->Geometry.CamberRad;

        InOutSample.WheelToeRad[WheelIndex] =
            RearContacts[Index]->Geometry.ToeRad;

        InOutSample.TireRadialDeflectionM[WheelIndex] =
            RearContacts[Index]->TireRadialDeflectionM;
    }
}

void TATelemetry::ApplyCompiledConfigMetadata(
    const FTAVehicleCompiledConfig& Config,
    FTAVehicleTelemetrySample& InOutSample)
{
    InOutSample.PhysicsConfigHash =
        Config.PhysicsConfigHash;
}
