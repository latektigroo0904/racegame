#pragma once

#include "CoreMinimal.h"
#include "TATelemetryBuffer.h"
#include "TARegressionEnvelope.h"

enum class ETATelemetryMetric : uint8
{
    EngineRPM,
    ChassisSpeedMps,
    ChassisForwardSpeedMps,
    ChassisVerticalSpeedMps,
    ChassisYawRateRadPerSec,
    TotalLongitudinalForceN,
    TotalLateralForceN,
    CoolingEfficiency01,
    CoolantTemperatureC,
    SteeringRackDisplacementM,
    SteeringRackDamage01,
    SteeringCommandAuthority01,
    SteeringRackFreePlayM,
    WheelVerticalLoadN,
    SuspensionTravelM,
    WheelCamberRad,
    WheelToeRad,
    WheelHubDamage01,
    WheelHubBrakeEfficiency01,
    WheelHubDriveEfficiency01,
    WheelHubBearingDragTorqueNm,
    SuspensionSpringDamperDamage01,
    SuspensionSpringEfficiency01,
    SuspensionDampingEfficiency01,
    SuspensionStopEfficiency01,
    AntiRollLinkDamage01,
    AntiRollLinkEfficiency01,
    BrakeTemperatureC,
    BrakeThermalTorqueFactor01,
    BrakeWear01,
    BrakeWearTorqueFactor01,
    TireRadialDeflectionM,
    WheelSlipRatio,
    TireLongitudinalForceN,
    TireLateralForceN,
    TireSurfaceTemperatureC
};

enum class ETATelemetryStatistic : uint8
{
    Minimum,
    Maximum,
    Mean,
    Final,
    AbsoluteMaximum
};

struct TA_TELEMETRY_API FTATelemetryMetricEnvelope
{
    ETATelemetryMetric Metric =
        ETATelemetryMetric::ChassisSpeedMps;

    ETATelemetryStatistic Statistic =
        ETATelemetryStatistic::Mean;

    // Required by wheel-indexed metrics. Ignored otherwise.
    int32 WheelIndex = INDEX_NONE;

    double MinimumAllowed = -TNumericLimits<double>::Max();
    double MaximumAllowed = TNumericLimits<double>::Max();

    // Evaluation window in chronological sample fraction.
    double StartFraction01 = 0.0;
    double EndFraction01 = 1.0;
};

struct TA_TELEMETRY_API FTATelemetryRegressionConfig
{
    FName ScenarioId = NAME_None;

    // Zero disables the hash check.
    uint32 ExpectedPhysicsConfigHash = 0;

    int32 MinimumRequiredSamples = 1;

    TArray<FTATelemetryMetricEnvelope> Envelopes;
};

struct TA_TELEMETRY_API FTATelemetryEnvelopeResult
{
    int32 EnvelopeIndex = INDEX_NONE;

    ETATelemetryMetric Metric =
        ETATelemetryMetric::ChassisSpeedMps;

    ETATelemetryStatistic Statistic =
        ETATelemetryStatistic::Mean;

    int32 WheelIndex = INDEX_NONE;

    double StartFraction01 = 0.0;
    double EndFraction01 = 1.0;

    bool bPassed = false;

    double ObservedValue = 0.0;
    double MinimumAllowed = 0.0;
    double MaximumAllowed = 0.0;
};

struct TA_TELEMETRY_API FTATelemetryRegressionResult
{
    FName ScenarioId = NAME_None;

    uint32 ExpectedPhysicsConfigHash = 0;
    uint32 ObservedPhysicsConfigHash = 0;

    bool bPassed = false;
    bool bPhysicsConfigHashConsistent = true;
    bool bPhysicsConfigHashMatched = true;

    int32 SampleCount = 0;
    int32 PassedEnvelopeCount = 0;
    int32 FailedEnvelopeCount = 0;

    TArray<FTATelemetryEnvelopeResult> EnvelopeResults;
    TArray<FString> Failures;
};

struct TA_TELEMETRY_API FTATelemetryMetricProfile
{
    ETATelemetryMetric Metric =
        ETATelemetryMetric::ChassisSpeedMps;

    int32 WheelIndex = INDEX_NONE;

    double StartFraction01 = 0.0;
    double EndFraction01 = 1.0;

    // Trailing fraction of the selected window.
    double SteadyStateFraction01 = 0.25;

    FTARegressionRange MinimumEnvelope;
    FTARegressionRange MaximumEnvelope;
    FTARegressionRange SteadyStateEnvelope;
};

struct TA_TELEMETRY_API FTATelemetryScenarioProfileConfig
{
    FName ScenarioId = NAME_None;

    int32 BaselineVersion = 1;

    bool bTrustedBaseline = false;

    uint32 ExpectedPhysicsConfigHash = 0;

    int32 MinimumRequiredSamples = 1;

    TArray<FTATelemetryMetricProfile> Metrics;
};

struct TA_TELEMETRY_API FTATelemetryMetricProfileResult
{
    ETATelemetryMetric Metric =
        ETATelemetryMetric::ChassisSpeedMps;

    int32 WheelIndex = INDEX_NONE;

    double StartFraction01 = 0.0;
    double EndFraction01 = 1.0;

    FTARegressionRange MinimumEnvelope;
    FTARegressionRange MaximumEnvelope;
    FTARegressionRange SteadyStateEnvelope;

    FTARegressionMetricResult Evaluation;
};

struct TA_TELEMETRY_API FTATelemetryScenarioProfileResult
{
    FName ScenarioId = NAME_None;

    int32 BaselineVersion = 0;

    bool bTrustedBaseline = false;

    uint32 ExpectedPhysicsConfigHash = 0;
    uint32 ObservedPhysicsConfigHash = 0;

    bool bPhysicsConfigHashConsistent = true;
    bool bPhysicsConfigHashMatched = true;
    bool bPassed = false;

    int32 SampleCount = 0;

    TArray<FTATelemetryMetricProfileResult> MetricResults;
    TArray<FString> Failures;
};

namespace TATelemetryRegression
{
    TA_TELEMETRY_API bool ValidateConfig(
        const FTATelemetryRegressionConfig& Config);

    TA_TELEMETRY_API bool Evaluate(
        const FTATelemetryRingBuffer& Buffer,
        const FTATelemetryRegressionConfig& Config,
        FTATelemetryRegressionResult& OutResult);

    // Machine-readable outputs for CI/regression artifact capture.
    TA_TELEMETRY_API FString ExportCsv(
        const FTATelemetryRegressionResult& Result);

    TA_TELEMETRY_API FString ExportJsonLines(
        const FTATelemetryRegressionResult& Result);

    TA_TELEMETRY_API bool ValidateProfileConfig(
        const FTATelemetryScenarioProfileConfig& Config);

    TA_TELEMETRY_API bool EvaluateProfile(
        const FTATelemetryRingBuffer& Buffer,
        const FTATelemetryScenarioProfileConfig& Config,
        FTATelemetryScenarioProfileResult& OutResult);

    TA_TELEMETRY_API FString ExportProfileCsv(
        const FTATelemetryScenarioProfileResult& Result);

    TA_TELEMETRY_API FString ExportProfileJsonLines(
        const FTATelemetryScenarioProfileResult& Result);
}
