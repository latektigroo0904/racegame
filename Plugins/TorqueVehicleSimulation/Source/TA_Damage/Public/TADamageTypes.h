#pragma once

#include "CoreMinimal.h"

enum class ETADamageSignalType : uint8
{
    ImpactImpulse,
    ImpactEnergy,
    StructuralDisplacement,
    StructuralFracture,
    TorsionalOverload,
    ThermalExposure,
    WearEnergy,
    FluidPressureLoss,
    ElectricalDisconnection
};

struct TA_DAMAGE_API FTADamageSignal
{
    uint64 SimulationTick = 0;
    uint16 Substep = 0;

    int32 TargetComponentIndex = INDEX_NONE;
    int32 SourceElementIndex = INDEX_NONE;

    uint32 SequenceId = 0;

    ETADamageSignalType Type = ETADamageSignalType::ImpactEnergy;

    double ScalarValue = 0.0;
    FVector3d VectorValue = FVector3d::ZeroVector;
};

class TA_DAMAGE_API FTADamageEventQueue
{
public:
    void Initialize(const int32 InMaxEvents)
    {
        MaxEvents = FMath::Max(0, InMaxEvents);
        Events.Reset(MaxEvents);
        Events.Reserve(MaxEvents);
    }

    void ResetKeepCapacity()
    {
        Events.Reset(MaxEvents);
    }

    bool Push(const FTADamageSignal& Signal)
    {
        if (Events.Num() >= MaxEvents)
        {
            return false;
        }

        Events.Add(Signal);
        return true;
    }

    void SortDeterministic();

    TConstArrayView<FTADamageSignal> GetEvents() const
    {
        return MakeArrayView(Events);
    }

    int32 Num() const
    {
        return Events.Num();
    }

    int32 CapacityLimit() const
    {
        return MaxEvents;
    }

private:
    TArray<FTADamageSignal> Events;
    int32 MaxEvents = 0;
};

struct TA_DAMAGE_API FTARadiatorDamageConfig
{
    double InitialCoolantMassKg = 6.0;

    double PunctureThresholdEnergyJ = 2500.0;
    double FullLeakEnergyJ = 20000.0;
    double MaxLeakAreaMm2 = 20.0;

    double LeakMassFlowKgPerSecPerMm2 = 0.003;

    double MinimumAirflowEfficiency01 = 0.15;
};

struct TA_DAMAGE_API FTARadiatorDamageState
{
    double CoolantMassKg = 6.0;

    double LeakAreaMm2 = 0.0;
    double AirflowEfficiency01 = 1.0;
    double CoolingEfficiency01 = 1.0;

    bool bPunctured = false;
};

namespace TADamage
{
    TA_DAMAGE_API void InitializeRadiatorState(
        const FTARadiatorDamageConfig& Config,
        FTARadiatorDamageState& OutState);

    TA_DAMAGE_API void ApplyRadiatorImpact(
        const FTARadiatorDamageConfig& Config,
        double ImpactEnergyJ,
        double CrushFraction01,
        FTARadiatorDamageState& InOutState);

    TA_DAMAGE_API double UpdateRadiatorFluidLoss(
        const FTARadiatorDamageConfig& Config,
        double DeltaTimeSeconds,
        FTARadiatorDamageState& InOutState);
}
