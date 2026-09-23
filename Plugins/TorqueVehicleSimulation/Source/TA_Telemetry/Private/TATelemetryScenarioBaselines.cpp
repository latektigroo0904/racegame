#include "TATelemetryScenarioBaselines.h"

namespace
{
    void AddEnvelope(
        FTATelemetryRegressionConfig& Config,
        const ETATelemetryMetric Metric,
        const ETATelemetryStatistic Statistic,
        const double MinimumAllowed,
        const double MaximumAllowed,
        const int32 WheelIndex = INDEX_NONE,
        const double StartFraction01 = 0.0,
        const double EndFraction01 = 1.0)
    {
        FTATelemetryMetricEnvelope Envelope;

        Envelope.Metric =
            Metric;

        Envelope.Statistic =
            Statistic;

        Envelope.WheelIndex =
            WheelIndex;

        Envelope.MinimumAllowed =
            MinimumAllowed;

        Envelope.MaximumAllowed =
            MaximumAllowed;

        Envelope.StartFraction01 =
            StartFraction01;

        Envelope.EndFraction01 =
            EndFraction01;

        Config.Envelopes.Add(
            Envelope);
    }

    void AddProfile(
        FTATelemetryScenarioProfileConfig& Config,
        const ETATelemetryMetric Metric,
        const int32 WheelIndex,
        const double MinObservedLow,
        const double MinObservedHigh,
        const double MaxObservedLow,
        const double MaxObservedHigh,
        const double SteadyLow,
        const double SteadyHigh,
        const double StartFraction01 = 0.0,
        const double EndFraction01 = 1.0,
        const double SteadyStateFraction01 = 0.25)
    {
        FTATelemetryMetricProfile Profile;

        Profile.Metric =
            Metric;

        Profile.WheelIndex =
            WheelIndex;

        Profile.StartFraction01 =
            StartFraction01;

        Profile.EndFraction01 =
            EndFraction01;

        Profile.SteadyStateFraction01 =
            SteadyStateFraction01;

        Profile.MinimumEnvelope.MinInclusive =
            MinObservedLow;

        Profile.MinimumEnvelope.MaxInclusive =
            MinObservedHigh;

        Profile.MaximumEnvelope.MinInclusive =
            MaxObservedLow;

        Profile.MaximumEnvelope.MaxInclusive =
            MaxObservedHigh;

        Profile.SteadyStateEnvelope.MinInclusive =
            SteadyLow;

        Profile.SteadyStateEnvelope.MaxInclusive =
            SteadyHigh;

        Config.Metrics.Add(
            Profile);
    }

    void InitializeBaseline(
        FTATelemetryScenarioBaseline& OutBaseline,
        const ETARegressionScenarioKind Kind,
        const FName ScenarioId,
        const uint32 ExpectedPhysicsConfigHash,
        const int32 MinimumRequiredSamples,
        const FString& Purpose)
    {
        OutBaseline =
            FTATelemetryScenarioBaseline{};

        OutBaseline.Kind =
            Kind;

        OutBaseline.ScenarioId =
            ScenarioId;

        OutBaseline.BaselineVersion =
            1;

        // These are intentionally broad safety envelopes. They become trusted
        // only after measured UE traces are reviewed and committed.
        OutBaseline.bTrustedBaseline =
            false;

        OutBaseline.Purpose =
            Purpose;

        OutBaseline.Regression.ScenarioId =
            ScenarioId;

        OutBaseline.Regression.ExpectedPhysicsConfigHash =
            ExpectedPhysicsConfigHash;

        OutBaseline.Regression.MinimumRequiredSamples =
            MinimumRequiredSamples;

        OutBaseline.Profile.ScenarioId =
            ScenarioId;

        OutBaseline.Profile.BaselineVersion =
            OutBaseline.BaselineVersion;

        OutBaseline.Profile.bTrustedBaseline =
            OutBaseline.bTrustedBaseline;

        OutBaseline.Profile.ExpectedPhysicsConfigHash =
            ExpectedPhysicsConfigHash;

        OutBaseline.Profile.MinimumRequiredSamples =
            MinimumRequiredSamples;
    }

    void BuildStaticSettle(
        const uint32 Hash,
        FTATelemetryScenarioBaseline& Out)
    {
        InitializeBaseline(
            Out,
            ETARegressionScenarioKind::StaticSettle,
            TEXT("TA.StaticSettle.V1"),
            Hash,
            240,
            TEXT(
                "Flat-road zero-control settle. Provisional envelopes only detect "
                "gross instability before measured equilibrium ranges exist."));

        AddEnvelope(
            Out.Regression,
            ETATelemetryMetric::ChassisVerticalSpeedMps,
            ETATelemetryStatistic::AbsoluteMaximum,
            0.0,
            8.0);

        AddEnvelope(
            Out.Regression,
            ETATelemetryMetric::ChassisVerticalSpeedMps,
            ETATelemetryStatistic::Mean,
            -0.75,
            0.75,
            INDEX_NONE,
            0.75,
            1.0);

        AddProfile(
            Out.Profile,
            ETATelemetryMetric::ChassisVerticalSpeedMps,
            INDEX_NONE,
            -8.0,
            8.0,
            -8.0,
            8.0,
            -0.75,
            0.75,
            0.0,
            1.0,
            0.25);

        for (int32 Wheel = 0;
             Wheel < TAPrototypeTelemetryWheelCount;
             ++Wheel)
        {
            AddEnvelope(
                Out.Regression,
                ETATelemetryMetric::WheelVerticalLoadN,
                ETATelemetryStatistic::Mean,
                250.0,
                12000.0,
                Wheel,
                0.75,
                1.0);

            AddEnvelope(
                Out.Regression,
                ETATelemetryMetric::TireRadialDeflectionM,
                ETATelemetryStatistic::Mean,
                0.0001,
                0.080,
                Wheel,
                0.75,
                1.0);

            AddProfile(
                Out.Profile,
                ETATelemetryMetric::WheelVerticalLoadN,
                Wheel,
                0.0,
                15000.0,
                0.0,
                15000.0,
                250.0,
                12000.0,
                0.0,
                1.0,
                0.25);

            AddProfile(
                Out.Profile,
                ETATelemetryMetric::TireRadialDeflectionM,
                Wheel,
                0.0,
                0.080,
                0.0,
                0.080,
                0.0001,
                0.080,
                0.0,
                1.0,
                0.25);
        }
    }

    void BuildAcceleration(
        const uint32 Hash,
        FTATelemetryScenarioBaseline& Out)
    {
        InitializeBaseline(
            Out,
            ETARegressionScenarioKind::Acceleration,
            TEXT("TA.Acceleration.V1"),
            Hash,
            240,
            TEXT(
                "Flat-road first-gear acceleration. Provisional bounds check "
                "forward motion, traction force and gross yaw stability."));

        AddEnvelope(
            Out.Regression,
            ETATelemetryMetric::ChassisForwardSpeedMps,
            ETATelemetryStatistic::Final,
            0.10,
            100.0);

        AddEnvelope(
            Out.Regression,
            ETATelemetryMetric::TotalLongitudinalForceN,
            ETATelemetryStatistic::Maximum,
            50.0,
            40000.0);

        AddEnvelope(
            Out.Regression,
            ETATelemetryMetric::ChassisYawRateRadPerSec,
            ETATelemetryStatistic::AbsoluteMaximum,
            0.0,
            3.0);

        AddEnvelope(
            Out.Regression,
            ETATelemetryMetric::WheelSlipRatio,
            ETATelemetryStatistic::AbsoluteMaximum,
            0.0,
            5.0,
            2);

        AddEnvelope(
            Out.Regression,
            ETATelemetryMetric::WheelSlipRatio,
            ETATelemetryStatistic::AbsoluteMaximum,
            0.0,
            5.0,
            3);

        AddProfile(
            Out.Profile,
            ETATelemetryMetric::ChassisForwardSpeedMps,
            INDEX_NONE,
            -1.0,
            5.0,
            0.10,
            120.0,
            0.10,
            120.0);

        AddProfile(
            Out.Profile,
            ETATelemetryMetric::TotalLongitudinalForceN,
            INDEX_NONE,
            -20000.0,
            20000.0,
            50.0,
            40000.0,
            -20000.0,
            40000.0);

        AddProfile(
            Out.Profile,
            ETATelemetryMetric::ChassisYawRateRadPerSec,
            INDEX_NONE,
            -3.0,
            3.0,
            -3.0,
            3.0,
            -1.0,
            1.0);
    }

    void BuildBraking(
        const uint32 Hash,
        FTATelemetryScenarioBaseline& Out)
    {
        InitializeBaseline(
            Out,
            ETARegressionScenarioKind::Braking,
            TEXT("TA.Braking15Mps.V1"),
            Hash,
            120,
            TEXT(
                "Prototype scenario assumes approximately 15 m/s initial speed "
                "on flat road and strong braking. Ranges remain provisional."));

        AddEnvelope(
            Out.Regression,
            ETATelemetryMetric::ChassisForwardSpeedMps,
            ETATelemetryStatistic::Final,
            -0.5,
            14.9);

        AddEnvelope(
            Out.Regression,
            ETATelemetryMetric::ChassisYawRateRadPerSec,
            ETATelemetryStatistic::AbsoluteMaximum,
            0.0,
            3.0);

        for (int32 Wheel = 0;
             Wheel < TAPrototypeTelemetryWheelCount;
             ++Wheel)
        {
            AddEnvelope(
                Out.Regression,
                ETATelemetryMetric::WheelSlipRatio,
                ETATelemetryStatistic::AbsoluteMaximum,
                0.0,
                8.0,
                Wheel);
        }

        AddProfile(
            Out.Profile,
            ETATelemetryMetric::ChassisForwardSpeedMps,
            INDEX_NONE,
            -0.5,
            15.5,
            14.0,
            16.5,
            0.0,
            14.9,
            0.0,
            1.0,
            0.25);

        AddProfile(
            Out.Profile,
            ETATelemetryMetric::ChassisYawRateRadPerSec,
            INDEX_NONE,
            -3.0,
            3.0,
            -3.0,
            3.0,
            -1.0,
            1.0);
    }

    void BuildConstantSteer(
        const uint32 Hash,
        FTATelemetryScenarioBaseline& Out)
    {
        InitializeBaseline(
            Out,
            ETARegressionScenarioKind::ConstantSteer,
            TEXT("TA.ConstantSteer15Mps.V1"),
            Hash,
            120,
            TEXT(
                "Prototype scenario assumes approximately 15 m/s and a fixed "
                "front steering input. Safety envelopes require a finite steering/yaw response."));

        AddEnvelope(
            Out.Regression,
            ETATelemetryMetric::SteeringRackDisplacementM,
            ETATelemetryStatistic::AbsoluteMaximum,
            0.001,
            0.060);

        AddEnvelope(
            Out.Regression,
            ETATelemetryMetric::ChassisYawRateRadPerSec,
            ETATelemetryStatistic::AbsoluteMaximum,
            0.001,
            5.0);

        AddEnvelope(
            Out.Regression,
            ETATelemetryMetric::TotalLateralForceN,
            ETATelemetryStatistic::AbsoluteMaximum,
            50.0,
            50000.0);

        AddProfile(
            Out.Profile,
            ETATelemetryMetric::SteeringRackDisplacementM,
            INDEX_NONE,
            -0.060,
            0.060,
            -0.060,
            0.060,
            -0.060,
            0.060);

        AddProfile(
            Out.Profile,
            ETATelemetryMetric::ChassisYawRateRadPerSec,
            INDEX_NONE,
            -5.0,
            5.0,
            -5.0,
            5.0,
            -5.0,
            5.0);

        AddProfile(
            Out.Profile,
            ETATelemetryMetric::TotalLateralForceN,
            INDEX_NONE,
            -50000.0,
            50000.0,
            -50000.0,
            50000.0,
            -50000.0,
            50000.0);
    }

    void BuildAsymmetricRoad(
        const uint32 Hash,
        FTATelemetryScenarioBaseline& Out)
    {
        InitializeBaseline(
            Out,
            ETARegressionScenarioKind::AsymmetricRoad,
            TEXT("TA.AsymmetricRoad20mm.V1"),
            Hash,
            60,
            TEXT(
                "One-side 20 mm road-height split used to exercise suspension "
                "travel, anti-roll coupling and compliant tire load transfer."));

        AddEnvelope(
            Out.Regression,
            ETATelemetryMetric::SuspensionTravelM,
            ETATelemetryStatistic::AbsoluteMaximum,
            0.001,
            0.15,
            1);

        AddEnvelope(
            Out.Regression,
            ETATelemetryMetric::WheelVerticalLoadN,
            ETATelemetryStatistic::Maximum,
            250.0,
            15000.0,
            1);

        AddEnvelope(
            Out.Regression,
            ETATelemetryMetric::TireRadialDeflectionM,
            ETATelemetryStatistic::Maximum,
            0.0001,
            0.080,
            1);

        AddEnvelope(
            Out.Regression,
            ETATelemetryMetric::ChassisYawRateRadPerSec,
            ETATelemetryStatistic::AbsoluteMaximum,
            0.0,
            3.0);

        AddProfile(
            Out.Profile,
            ETATelemetryMetric::SuspensionTravelM,
            1,
            -0.15,
            0.15,
            -0.15,
            0.15,
            -0.15,
            0.15);

        AddProfile(
            Out.Profile,
            ETATelemetryMetric::WheelVerticalLoadN,
            1,
            0.0,
            15000.0,
            0.0,
            15000.0,
            250.0,
            15000.0);

        AddProfile(
            Out.Profile,
            ETATelemetryMetric::TireRadialDeflectionM,
            1,
            0.0,
            0.080,
            0.0,
            0.080,
            0.0,
            0.080);
    }

    void BuildSyntheticCrash(
        const uint32 Hash,
        FTATelemetryScenarioBaseline& Out)
    {
        InitializeBaseline(
            Out,
            ETARegressionScenarioKind::SyntheticFrontCornerCrash,
            TEXT("TA.SyntheticFrontRightCrash.V1"),
            Hash,
            30,
            TEXT(
                "Synthetic front-right collision impulse followed by post-impact "
                "driving samples. Provisional envelopes only establish finite damage response."));

        AddEnvelope(
            Out.Regression,
            ETATelemetryMetric::ChassisYawRateRadPerSec,
            ETATelemetryStatistic::AbsoluteMaximum,
            0.001,
            20.0);

        AddEnvelope(
            Out.Regression,
            ETATelemetryMetric::WheelToeRad,
            ETATelemetryStatistic::AbsoluteMaximum,
            0.0001,
            0.75,
            1);

        AddEnvelope(
            Out.Regression,
            ETATelemetryMetric::WheelCamberRad,
            ETATelemetryStatistic::AbsoluteMaximum,
            0.0001,
            0.75,
            1);

        AddEnvelope(
            Out.Regression,
            ETATelemetryMetric::TireLateralForceN,
            ETATelemetryStatistic::AbsoluteMaximum,
            1.0,
            50000.0,
            1);

        AddEnvelope(
            Out.Regression,
            ETATelemetryMetric::CoolingEfficiency01,
            ETATelemetryStatistic::Minimum,
            0.0,
            1.0);

        AddProfile(
            Out.Profile,
            ETATelemetryMetric::ChassisYawRateRadPerSec,
            INDEX_NONE,
            -20.0,
            20.0,
            -20.0,
            20.0,
            -20.0,
            20.0);

        AddProfile(
            Out.Profile,
            ETATelemetryMetric::WheelToeRad,
            1,
            -0.75,
            0.75,
            -0.75,
            0.75,
            -0.75,
            0.75);

        AddProfile(
            Out.Profile,
            ETATelemetryMetric::WheelCamberRad,
            1,
            -0.75,
            0.75,
            -0.75,
            0.75,
            -0.75,
            0.75);

        AddProfile(
            Out.Profile,
            ETATelemetryMetric::TireLateralForceN,
            1,
            -50000.0,
            50000.0,
            -50000.0,
            50000.0,
            -50000.0,
            50000.0);

        AddProfile(
            Out.Profile,
            ETATelemetryMetric::CoolingEfficiency01,
            INDEX_NONE,
            0.0,
            1.0,
            0.0,
            1.0,
            0.0,
            1.0);
    }
}

bool TATelemetryScenarioBaselines::BuildProvisional(
    const ETARegressionScenarioKind Kind,
    const uint32 ExpectedPhysicsConfigHash,
    FTATelemetryScenarioBaseline& OutBaseline)
{
    switch (Kind)
    {
    case ETARegressionScenarioKind::StaticSettle:
        BuildStaticSettle(
            ExpectedPhysicsConfigHash,
            OutBaseline);
        break;

    case ETARegressionScenarioKind::Acceleration:
        BuildAcceleration(
            ExpectedPhysicsConfigHash,
            OutBaseline);
        break;

    case ETARegressionScenarioKind::Braking:
        BuildBraking(
            ExpectedPhysicsConfigHash,
            OutBaseline);
        break;

    case ETARegressionScenarioKind::ConstantSteer:
        BuildConstantSteer(
            ExpectedPhysicsConfigHash,
            OutBaseline);
        break;

    case ETARegressionScenarioKind::AsymmetricRoad:
        BuildAsymmetricRoad(
            ExpectedPhysicsConfigHash,
            OutBaseline);
        break;

    case ETARegressionScenarioKind::SyntheticFrontCornerCrash:
        BuildSyntheticCrash(
            ExpectedPhysicsConfigHash,
            OutBaseline);
        break;

    default:
        return false;
    }

    return ValidateBaseline(
        OutBaseline);
}

void TATelemetryScenarioBaselines::BuildAllProvisional(
    const uint32 ExpectedPhysicsConfigHash,
    TArray<FTATelemetryScenarioBaseline>& OutBaselines)
{
    OutBaselines.Reset();
    OutBaselines.Reserve(6);

    constexpr ETARegressionScenarioKind Kinds[] =
    {
        ETARegressionScenarioKind::StaticSettle,
        ETARegressionScenarioKind::Acceleration,
        ETARegressionScenarioKind::Braking,
        ETARegressionScenarioKind::ConstantSteer,
        ETARegressionScenarioKind::AsymmetricRoad,
        ETARegressionScenarioKind::SyntheticFrontCornerCrash
    };

    for (const ETARegressionScenarioKind Kind : Kinds)
    {
        FTATelemetryScenarioBaseline Baseline;

        if (BuildProvisional(
                Kind,
                ExpectedPhysicsConfigHash,
                Baseline))
        {
            OutBaselines.Add(
                MoveTemp(Baseline));
        }
    }
}

bool TATelemetryScenarioBaselines::ValidateBaseline(
    const FTATelemetryScenarioBaseline& Baseline)
{
    if (Baseline.ScenarioId.IsNone() ||
        Baseline.BaselineVersion <= 0 ||
        Baseline.Regression.ScenarioId
            != Baseline.ScenarioId ||
        Baseline.Profile.ScenarioId
            != Baseline.ScenarioId ||
        Baseline.Profile.BaselineVersion
            != Baseline.BaselineVersion ||
        Baseline.Profile.bTrustedBaseline
            != Baseline.bTrustedBaseline ||
        !TATelemetryRegression::ValidateConfig(
            Baseline.Regression) ||
        !TATelemetryRegression::ValidateProfileConfig(
            Baseline.Profile))
    {
        return false;
    }

    if (Baseline.bTrustedBaseline &&
        (Baseline.Regression.ExpectedPhysicsConfigHash == 0 ||
         Baseline.Profile.ExpectedPhysicsConfigHash == 0))
    {
        return false;
    }

    return true;
}
