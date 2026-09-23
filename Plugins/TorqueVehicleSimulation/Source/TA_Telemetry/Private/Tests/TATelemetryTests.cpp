#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TATelemetry.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTACompactTelemetryRingBufferTest,
    "TorqueAtlas.Telemetry.RingBuffer.WrapAndOrder",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTACompactTelemetryRingBufferTest::RunTest(const FString& Parameters)
{
    FTACompactTelemetryRingBuffer Buffer;
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTATelemetryAerodynamicsCaptureTest,
    "TorqueAtlas.Telemetry.Aerodynamics.CapturesAppliedStepOutput",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTATelemetryAerodynamicsCaptureTest::RunTest(const FString& Parameters)
{
    FTAVehicleRuntimeState State;
    State.SimulationTick = 42;

    FTAVehicleStepOutput Output;
    Output.Aerodynamics.RelativeAirVelocityWorldMps = FVector3d(-30.0, 4.0, 0.0);
    Output.Aerodynamics.DynamicPressurePa = 560.0;
    Output.Aerodynamics.ForceWorldN = FVector3d(-850.0, 15.0, -310.0);
    Output.Aerodynamics.TorqueWorldNm = FVector3d(2.0, 48.0, -6.0);

    const FTATelemetrySample Sample =
        TATelemetry::MakeSample(State, Output);

    TestEqual(TEXT("Simulation tick copied"), Sample.SimulationTick, static_cast<uint64>(42));
    TestTrue(TEXT("Relative air speed derived from exact output vector"),
        FMath::IsNearlyEqual(Sample.AeroRelativeAirSpeedMps, Output.Aerodynamics.RelativeAirVelocityWorldMps.Length(), 1.0e-9));
    TestEqual(TEXT("Dynamic pressure copied"), Sample.AeroDynamicPressurePa, 560.0);
    TestTrue(TEXT("Applied aero force copied exactly"), Sample.AeroForceWorldN.Equals(Output.Aerodynamics.ForceWorldN, 1.0e-9));
    TestTrue(TEXT("Applied aero torque copied exactly"), Sample.AeroTorqueWorldNm.Equals(Output.Aerodynamics.TorqueWorldNm, 1.0e-9));

    return true;
}

#endif
