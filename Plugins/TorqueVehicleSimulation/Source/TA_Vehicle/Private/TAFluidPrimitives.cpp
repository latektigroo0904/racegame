#include "TAFluidPrimitives.h"

bool TAFluidPrimitives::ValidateLeakConfig(
    const FTAFluidLeakConfig& Config)
{
    return
        FMath::IsFinite(Config.FluidDensityKgPerM3)
        && Config.FluidDensityKgPerM3 > UE_DOUBLE_SMALL_NUMBER
        && FMath::IsFinite(Config.DischargeCoefficient)
        && Config.DischargeCoefficient >= 0.0
        && Config.DischargeCoefficient <= 1.0;
}

double TAFluidPrimitives::CalculateOrificeMassFlowKgPerSec(
    const FTAFluidLeakConfig& Config,
    const double LeakAreaM2,
    const double UpstreamPressurePa,
    const double DownstreamPressurePa)
{
    if (!ValidateLeakConfig(Config)
        || !FMath::IsFinite(LeakAreaM2)
        || LeakAreaM2 <= 0.0
        || !FMath::IsFinite(UpstreamPressurePa)
        || !FMath::IsFinite(DownstreamPressurePa))
    {
        return 0.0;
    }

    const double PressureDifferencePa =
        FMath::Max(
            0.0,
            UpstreamPressurePa
                - DownstreamPressurePa);

    if (PressureDifferencePa <= 0.0)
    {
        return 0.0;
    }

    return
        Config.DischargeCoefficient
        * LeakAreaM2
        * FMath::Sqrt(
            2.0
            * Config.FluidDensityKgPerM3
            * PressureDifferencePa);
}

bool TAFluidPrimitives::IntegrateReservoirLeak(
    const FTAFluidLeakConfig& Config,
    const double LeakAreaM2,
    const double UpstreamPressurePa,
    const double DownstreamPressurePa,
    const double DeltaTimeSeconds,
    FTAFluidReservoirState& InOutReservoir,
    FTAFluidLeakOutput& OutOutput)
{
    OutOutput =
        FTAFluidLeakOutput{};

    if (!ValidateLeakConfig(Config)
        || !FMath::IsFinite(LeakAreaM2)
        || LeakAreaM2 < 0.0
        || !FMath::IsFinite(UpstreamPressurePa)
        || !FMath::IsFinite(DownstreamPressurePa)
        || !FMath::IsFinite(DeltaTimeSeconds)
        || DeltaTimeSeconds <= 0.0
        || !FMath::IsFinite(InOutReservoir.MassKg)
        || InOutReservoir.MassKg < 0.0
        || !FMath::IsFinite(
            InOutReservoir.CumulativeLeakedMassKg)
        || InOutReservoir.CumulativeLeakedMassKg < 0.0)
    {
        return false;
    }

    OutOutput.RequestedMassFlowKgPerSec =
        CalculateOrificeMassFlowKgPerSec(
            Config,
            LeakAreaM2,
            UpstreamPressurePa,
            DownstreamPressurePa);

    const double RequestedMassKg =
        OutOutput.RequestedMassFlowKgPerSec
        * DeltaTimeSeconds;

    OutOutput.LeakedMassThisStepKg =
        FMath::Min(
            InOutReservoir.MassKg,
            RequestedMassKg);

    OutOutput.ActualMassFlowKgPerSec =
        OutOutput.LeakedMassThisStepKg
        / DeltaTimeSeconds;

    InOutReservoir.MassKg -=
        OutOutput.LeakedMassThisStepKg;

    InOutReservoir.CumulativeLeakedMassKg +=
        OutOutput.LeakedMassThisStepKg;

    return true;
}
