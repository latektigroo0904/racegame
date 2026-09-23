#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TADamageTypes.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTADamageQueueOrderingTest,
    "TorqueAtlas.Damage.Queue.DeterministicOrdering",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTADamageQueueOrderingTest::RunTest(const FString& Parameters)
{
    FTADamageEventQueue Queue;
    Queue.Initialize(8);

    FTADamageSignal A;
    A.SimulationTick = 10;
    A.Substep = 2;
    A.TargetComponentIndex = 4;
    A.SourceElementIndex = 3;
    A.SequenceId = 2;

    FTADamageSignal B = A;
    B.Substep = 1;
    B.SequenceId = 1;

    FTADamageSignal C = A;
    C.TargetComponentIndex = 2;
    C.SequenceId = 3;

    TestTrue(TEXT("Push A"), Queue.Push(A));
    TestTrue(TEXT("Push B"), Queue.Push(B));
    TestTrue(TEXT("Push C"), Queue.Push(C));

    Queue.SortDeterministic();

    const TConstArrayView<FTADamageSignal> Events = Queue.GetEvents();

    TestEqual(TEXT("Queue size"), Events.Num(), 3);
    TestEqual(TEXT("Earlier substep sorts first"), Events[0].Substep, static_cast<uint16>(1));
    TestEqual(TEXT("Lower target index sorts before later target at same time"), Events[1].TargetComponentIndex, 2);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTARadiatorDamageChainTest,
    "TorqueAtlas.Damage.Radiator.ImpactLeakCooling",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTARadiatorDamageChainTest::RunTest(const FString& Parameters)
{
    FTARadiatorDamageConfig Config;
    FTARadiatorDamageState State;

    TADamage::InitializeRadiatorState(Config, State);

    const double InitialCoolant = State.CoolantMassKg;

    TADamage::ApplyRadiatorImpact(
        Config,
        12000.0,
        0.50,
        State);

    TestTrue(TEXT("Impact punctures radiator above threshold"), State.bPunctured);
    TestTrue(TEXT("Impact creates non-zero leak area"), State.LeakAreaMm2 > 0.0);
    TestTrue(TEXT("Crush reduces airflow efficiency"), State.AirflowEfficiency01 < 1.0);

    const double LostKg =
        TADamage::UpdateRadiatorFluidLoss(
            Config,
            10.0,
            State);

    TestTrue(TEXT("Puncture loses coolant over time"), LostKg > 0.0);
    TestTrue(TEXT("Coolant mass decreases"), State.CoolantMassKg < InitialCoolant);
    TestTrue(TEXT("Cooling efficiency degrades"), State.CoolingEfficiency01 < 1.0);

    return true;
}

#endif
