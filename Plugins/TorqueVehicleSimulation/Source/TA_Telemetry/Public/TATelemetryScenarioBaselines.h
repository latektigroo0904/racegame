#pragma once

#include "CoreMinimal.h"
#include "TATelemetryRegression.h"

enum class ETARegressionScenarioKind : uint8
{
    StaticSettle,
    Acceleration,
    Braking,
    ConstantSteer,
    AsymmetricRoad,
    SyntheticFrontCornerCrash
};

struct TA_TELEMETRY_API FTATelemetryScenarioBaseline
{
    ETARegressionScenarioKind Kind =
        ETARegressionScenarioKind::StaticSettle;

    FName ScenarioId = NAME_None;

    int32 BaselineVersion = 1;

    // False until ranges have been measured and accepted from a real UE run.
    bool bTrustedBaseline = false;

    FString Purpose;

    FTATelemetryRegressionConfig Regression;

    // Rich min/max/steady-state report definition for the same scenario.
    FTATelemetryScenarioProfileConfig Profile;
};

namespace TATelemetryScenarioBaselines
{
    TA_TELEMETRY_API bool BuildProvisional(
        ETARegressionScenarioKind Kind,
        uint32 ExpectedPhysicsConfigHash,
        FTATelemetryScenarioBaseline& OutBaseline);

    TA_TELEMETRY_API void BuildAllProvisional(
        uint32 ExpectedPhysicsConfigHash,
        TArray<FTATelemetryScenarioBaseline>& OutBaselines);

    TA_TELEMETRY_API bool ValidateBaseline(
        const FTATelemetryScenarioBaseline& Baseline);
}
