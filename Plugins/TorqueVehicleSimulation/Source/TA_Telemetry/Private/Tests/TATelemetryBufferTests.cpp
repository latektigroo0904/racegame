#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TATelemetryBuffer.h"
#include "TAFourWheelVehicleRuntime.h"
#include "TAVehicleRuntime.h"

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


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTATelemetryCsvExportTest,
    "TorqueAtlas.Telemetry.CsvExport",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTATelemetryCsvExportTest::RunTest(const FString& Parameters)
{
    FTATelemetryRingBuffer Buffer;

    TestTrue(
        TEXT("Buffer initializes"),
        Buffer.Initialize(2));

    FTAVehicleTelemetrySample Sample;
    Sample.SimulationTick = 42;
    Sample.EngineRPM = 3150.0;
    Sample.WheelVerticalLoadN[0] = 3600.0;
    Sample.TireSurfaceTemperatureC[0] = 88.0;

    TestTrue(
        TEXT("Sample pushes"),
        Buffer.Push(Sample));

    const FString Csv =
        Buffer.ExportCsv();

    TestTrue(
        TEXT("CSV contains tick header"),
        Csv.Contains(TEXT("tick,engine_rpm")));

    TestTrue(
        TEXT("CSV contains four-wheel load channel"),
        Csv.Contains(TEXT("load_fl_n")));

    TestTrue(
        TEXT("CSV contains pushed tick"),
        Csv.Contains(TEXT("42,")));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTATelemetryFourWheelMappingTest,
    "TorqueAtlas.Telemetry.FourWheelMapping",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTATelemetryFourWheelMappingTest::RunTest(const FString& Parameters)
{
    FTAFourWheelStepOutput FourWheel;

    FourWheel.FrontAxle.RackDisplacementM = -0.012;
    FourWheel.FrontAxle.LeftSteeringAngleRad = 0.08;
    FourWheel.FrontAxle.RightSteeringAngleRad = 0.09;
    FourWheel.FrontAxle.AckermannDeltaRad = 0.01;

    FourWheel.FrontAxle.LeftContact.VerticalLoadN = 3500.0;
    FourWheel.FrontAxle.LeftContact.TravelM = 0.01;
    FourWheel.FrontAxle.LeftContact.Geometry.CamberRad = -0.02;
    FourWheel.FrontAxle.LeftContact.Geometry.ToeRad = 0.08;
    FourWheel.FrontAxle.LeftContact.TireRadialDeflectionM = 0.014;

    FourWheel.RearAxle.RightContact.VerticalLoadN = 3200.0;
    FourWheel.RearAxle.RightContact.TravelM = -0.005;
    FourWheel.RearAxle.RightContact.Geometry.CamberRad = -0.01;
    FourWheel.RearAxle.RightContact.Geometry.ToeRad = 0.002;

    FTAVehicleTelemetrySample Sample;

    TATelemetry::ApplyFourWheelSample(
        FourWheel,
        Sample);

    TestTrue(
        TEXT("Front rack maps into telemetry"),
        FMath::IsNearlyEqual(
            Sample.SteeringRackDisplacementM,
            -0.012,
            1.0e-9));

    TestTrue(
        TEXT("Front-left load maps to wheel zero"),
        FMath::IsNearlyEqual(
            Sample.WheelVerticalLoadN[0],
            3500.0,
            1.0e-9));

    TestTrue(
        TEXT("Rear-right load maps to wheel three"),
        FMath::IsNearlyEqual(
            Sample.WheelVerticalLoadN[3],
            3200.0,
            1.0e-9));

    TestTrue(
        TEXT("Ackermann metric maps"),
        FMath::IsNearlyEqual(
            Sample.FrontAckermannDeltaRad,
            0.01,
            1.0e-9));

    TestTrue(
        TEXT("Front tire radial deflection maps"),
        FMath::IsNearlyEqual(
            Sample.TireRadialDeflectionM[0],
            0.014,
            1.0e-9));

    FTAVehicleCompiledConfig Config;
    Config.PhysicsConfigHash = 0x1234ABCDu;

    TATelemetry::ApplyCompiledConfigMetadata(
        Config,
        Sample);

    TestEqual(
        TEXT("Physics config hash maps into telemetry"),
        Sample.PhysicsConfigHash,
        0x1234ABCDu);

    return true;
}

#endif
