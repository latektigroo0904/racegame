#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TATelemetryBuffer.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTATelemetryRingBufferTest,
    "TorqueAtlas.Telemetry.RingBuffer.Wraparound",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTATelemetryRingBufferTest::RunTest(const FString& Parameters)
{
    FTATelemetryRingBuffer Buffer;

    TestTrue(
        TEXT("Buffer initializes"),
        Buffer.Initialize(3));

    for (uint64 Tick = 1; Tick <= 4; ++Tick)
    {
        FTAVehicleTelemetrySample Sample;
        Sample.SimulationTick = Tick;

        TestTrue(
            TEXT("Push succeeds"),
            Buffer.Push(Sample));
    }

    TestEqual(
        TEXT("Buffer retains configured capacity"),
        Buffer.Num(),
        3);

    const FTAVehicleTelemetrySample* Oldest =
        Buffer.GetChronological(0);

    const FTAVehicleTelemetrySample* Middle =
        Buffer.GetChronological(1);

    const FTAVehicleTelemetrySample* Newest =
        Buffer.GetChronological(2);

    TestNotNull(TEXT("Oldest sample exists"), Oldest);
    TestNotNull(TEXT("Middle sample exists"), Middle);
    TestNotNull(TEXT("Newest sample exists"), Newest);

    if (Oldest && Middle && Newest)
    {
        TestEqual(
            TEXT("Oldest retained tick after wraparound"),
            Oldest->SimulationTick,
            static_cast<uint64>(2));

        TestEqual(
            TEXT("Middle retained tick"),
            Middle->SimulationTick,
            static_cast<uint64>(3));

        TestEqual(
            TEXT("Newest retained tick"),
            Newest->SimulationTick,
            static_cast<uint64>(4));
    }

    Buffer.ResetKeepCapacity();

    TestEqual(
        TEXT("Reset clears logical count but keeps storage"),
        Buffer.Num(),
        0);

    TestEqual(
        TEXT("Capacity remains after reset"),
        Buffer.Capacity(),
        3);

    return true;
}

#endif
