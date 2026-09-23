#pragma once

#include "CoreMinimal.h"
#include "TAVehicleSimulation.h"

struct FTAFrontAxleSolveOutput;
struct FTAFourWheelStepOutput;
struct FTAVehicleCompiledConfig;

constexpr int32 TAPrototypeTelemetryWheelCount = 4;

struct TA_TELEMETRY_API FTAVehicleTelemetrySample
{
    uint64 SimulationTick = 0;
    uint32 PhysicsConfigHash = 0;

    double EngineRPM = 0.0;
    int32 SelectedGear = 0;

    double ClutchTemperatureC = 0.0;
    double CoolantTemperatureC = 0.0;
    double CoolingEfficiency01 = 1.0;

    FVector3d ChassisLinearVelocityWorldMps = FVector3d::ZeroVector;
    FVector3d ChassisAngularVelocityWorldRadPerSec = FVector3d::ZeroVector;

    double TotalLongitudinalForceN = 0.0;
    double TotalLateralForceN = 0.0;

    double SteeringRackDisplacementM = 0.0;

    double SteeringRackDamage01 = 0.0;
    double SteeringCommandAuthority01 = 1.0;
    double SteeringRackFreePlayM = 0.0;

    double FrontLeftSteeringAngleRad = 0.0;
    double FrontRightSteeringAngleRad = 0.0;
    double FrontLeftBumpSteerRad = 0.0;
    double FrontRightBumpSteerRad = 0.0;
    double FrontAckermannDeltaRad = 0.0;

    double WheelSlipRatio[TAPrototypeTelemetryWheelCount] = {};
    double WheelSlipAngleRad[TAPrototypeTelemetryWheelCount] = {};
    double TireLongitudinalForceN[TAPrototypeTelemetryWheelCount] = {};
    double TireLateralForceN[TAPrototypeTelemetryWheelCount] = {};

    double WheelVerticalLoadN[TAPrototypeTelemetryWheelCount] = {};
    double SuspensionTravelM[TAPrototypeTelemetryWheelCount] = {};
    double WheelCamberRad[TAPrototypeTelemetryWheelCount] = {};
    double WheelToeRad[TAPrototypeTelemetryWheelCount] = {};

    double WheelHubDamage01[TAPrototypeTelemetryWheelCount] = {};
    double WheelHubBrakeEfficiency01[TAPrototypeTelemetryWheelCount] =
        { 1.0, 1.0, 1.0, 1.0 };
    double WheelHubDriveEfficiency01[TAPrototypeTelemetryWheelCount] =
        { 1.0, 1.0, 1.0, 1.0 };
    double WheelHubBearingDragTorqueNm[TAPrototypeTelemetryWheelCount] = {};

    double SuspensionSpringDamperDamage01[TAPrototypeTelemetryWheelCount] = {};
    double SuspensionSpringEfficiency01[TAPrototypeTelemetryWheelCount] =
        { 1.0, 1.0, 1.0, 1.0 };
    double SuspensionDampingEfficiency01[TAPrototypeTelemetryWheelCount] =
        { 1.0, 1.0, 1.0, 1.0 };
    double SuspensionStopEfficiency01[TAPrototypeTelemetryWheelCount] =
        { 1.0, 1.0, 1.0, 1.0 };
    double AntiRollLinkDamage01[TAPrototypeTelemetryWheelCount] = {};
    double AntiRollLinkEfficiency01[TAPrototypeTelemetryWheelCount] =
        { 1.0, 1.0, 1.0, 1.0 };

    double TireSurfaceTemperatureC[TAPrototypeTelemetryWheelCount] = {};
    double TirePressureKPa[TAPrototypeTelemetryWheelCount] = {};
    double TireWear01[TAPrototypeTelemetryWheelCount] = {};
    double TireRadialDeflectionM[TAPrototypeTelemetryWheelCount] = {};

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

    FString ExportCsv() const;

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

    TA_TELEMETRY_API void ApplyFrontAxleSample(
        const FTAFrontAxleSolveOutput& FrontAxle,
        FTAVehicleTelemetrySample& InOutSample);

    TA_TELEMETRY_API void ApplyFourWheelSample(
        const FTAFourWheelStepOutput& FourWheel,
        FTAVehicleTelemetrySample& InOutSample);

    TA_TELEMETRY_API void ApplyCompiledConfigMetadata(
        const FTAVehicleCompiledConfig& Config,
        FTAVehicleTelemetrySample& InOutSample);
}
