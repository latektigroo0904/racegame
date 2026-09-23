#include "TADamageTypes.h"

void FTADamageEventQueue::SortDeterministic()
{
    Events.Sort(
        [](const FTADamageSignal& A, const FTADamageSignal& B)
        {
            if (A.SimulationTick != B.SimulationTick)
            {
                return A.SimulationTick < B.SimulationTick;
            }

            if (A.Substep != B.Substep)
            {
                return A.Substep < B.Substep;
            }

            if (A.TargetComponentIndex != B.TargetComponentIndex)
            {
                return A.TargetComponentIndex < B.TargetComponentIndex;
            }

            if (A.SourceElementIndex != B.SourceElementIndex)
            {
                return A.SourceElementIndex < B.SourceElementIndex;
            }

            if (A.Type != B.Type)
            {
                return static_cast<uint8>(A.Type) < static_cast<uint8>(B.Type);
            }

            return A.SequenceId < B.SequenceId;
        });
}

void TADamage::InitializeRadiatorState(
    const FTARadiatorDamageConfig& Config,
    FTARadiatorDamageState& OutState)
{
    OutState = FTARadiatorDamageState{};
    OutState.CoolantMassKg =
        FMath::Max(0.0, Config.InitialCoolantMassKg);
}

void TADamage::ApplyRadiatorImpact(
    const FTARadiatorDamageConfig& Config,
    const double ImpactEnergyJ,
    const double CrushFraction01,
    FTARadiatorDamageState& InOutState)
{
    const double Crush =
        FMath::Clamp(CrushFraction01, 0.0, 1.0);

    const double MinimumAirflow =
        FMath::Clamp(Config.MinimumAirflowEfficiency01, 0.0, 1.0);

    const double AirflowFromCrush =
        FMath::Lerp(1.0, MinimumAirflow, Crush);

    InOutState.AirflowEfficiency01 =
        FMath::Min(InOutState.AirflowEfficiency01, AirflowFromCrush);

    if (ImpactEnergyJ <= Config.PunctureThresholdEnergyJ)
    {
        return;
    }

    const double EnergyRange =
        FMath::Max(
            1.0,
            Config.FullLeakEnergyJ - Config.PunctureThresholdEnergyJ);

    const double PunctureSeverity =
        FMath::Clamp(
            (ImpactEnergyJ - Config.PunctureThresholdEnergyJ) / EnergyRange,
            0.0,
            1.0);

    const double NewLeakAreaMm2 =
        FMath::Max(0.0, Config.MaxLeakAreaMm2)
        * PunctureSeverity;

    InOutState.LeakAreaMm2 =
        FMath::Max(InOutState.LeakAreaMm2, NewLeakAreaMm2);

    InOutState.bPunctured =
        InOutState.LeakAreaMm2 > 0.0;
}

double TADamage::UpdateRadiatorFluidLoss(
    const FTARadiatorDamageConfig& Config,
    const double DeltaTimeSeconds,
    FTARadiatorDamageState& InOutState)
{
    if (DeltaTimeSeconds <= 0.0)
    {
        return 0.0;
    }

    const double RequestedLossKg =
        FMath::Max(0.0, InOutState.LeakAreaMm2)
        * FMath::Max(0.0, Config.LeakMassFlowKgPerSecPerMm2)
        * DeltaTimeSeconds;

    const double ActualLossKg =
        FMath::Min(
            FMath::Max(0.0, InOutState.CoolantMassKg),
            RequestedLossKg);

    InOutState.CoolantMassKg =
        FMath::Max(
            0.0,
            InOutState.CoolantMassKg - ActualLossKg);

    const double InitialCoolant =
        FMath::Max(UE_DOUBLE_SMALL_NUMBER, Config.InitialCoolantMassKg);

    const double CoolantFraction =
        FMath::Clamp(
            InOutState.CoolantMassKg / InitialCoolant,
            0.0,
            1.0);

    InOutState.CoolingEfficiency01 =
        FMath::Clamp(
            InOutState.AirflowEfficiency01 * CoolantFraction,
            0.0,
            1.0);

    return ActualLossKg;
}
