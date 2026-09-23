#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TATelemetry.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTATelemetryRingBufferTest,
    "TorqueAtlas.Telemetry.RingBuffer.WrapAndOrder",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTATelemetryRingBufferTest::RunTest(const FString& Parameters)
{
    FTATelemetryRingBuffer Buffer;
    Buffer.Initialize(3);

    for (uint64 Tick = 1; Tick <= 5; ++Tick)
    {
        FTATelemetrySample Sample;
        Sample.SimulationTick = Tick;
        Buffer.Push(Sample);
    }

    TestEqual(TEXT("Buffer remains at fixed capacity"), Buffer.Num(), 3);
    TestEqual(TEXT("Capacity remains unchanged"), Buffer.Capacity(), 3);

    FTATelemetrySample Oldest;
    FTATelemetrySample Middle;
    FTATelemetrySample Newest;

    TestTrue(TEXT("Oldest sample available"), Buffer.GetOldest(0, Oldest));
    TestTrue(TEXT("Middle sample available"), Buffer.GetOldest(1, Middle));
    TestTrue(TEXT("Newest sample available"), Buffer.GetOldest(2, Newest));

    TestEqual(TEXT("Oldest retained tick after wrap"), Oldest.SimulationTick, static_cast<uint64>(3));
    TestEqual(TEXT("Middle retained tick after wrap"), Middle.SimulationTick, static_cast<uint64>(4));
    TestEqual(TEXT("Newest retained tick after wrap"), Newest.SimulationTick, static_cast<uint64>(5));

    return true;
}

#endif
