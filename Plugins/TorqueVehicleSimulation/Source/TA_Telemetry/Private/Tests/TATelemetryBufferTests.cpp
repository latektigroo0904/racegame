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
        Csv.Contains(TEXT("tick,physics_config_hash,engine_rpm")));

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


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTATelemetryFunctionalDamageCaptureTest,
    "TorqueAtlas.Telemetry.FunctionalDamageCapture",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTATelemetryFunctionalDamageCaptureTest::RunTest(
    const FString& Parameters)
{
    FTAVehicleRuntimeState State;

    State.Wheels.SetNum(4);
    State.WheelHubDamage.SetNum(4);
    State.SuspensionDamage.SetNum(4);

    State.SteeringRackDamage.Damage01 =
        0.60;

    State.SteeringRackDamage.CommandAuthority01 =
        0.55;

    State.SteeringRackDamage.FreePlayM =
        0.004;

    State.WheelHubDamage[1].Damage01 =
        0.70;

    State.WheelHubDamage[1].BrakeEfficiency01 =
        0.45;

    State.WheelHubDamage[1].DriveEfficiency01 =
        0.25;

    State.WheelHubDamage[1].BearingDragTorqueNm =
        52.0;

    State.SuspensionDamage[0].SpringDamperDamage01 =
        0.65;

    State.SuspensionDamage[0].SpringEfficiency01 =
        0.45;

    State.SuspensionDamage[0].DampingEfficiency01 =
        0.20;

    State.SuspensionDamage[0].StopEfficiency01 =
        0.85;

    State.SuspensionDamage[0].AntiRollLinkDamage01 =
        0.80;

    State.SuspensionDamage[0].AntiRollLinkEfficiency01 =
        0.15;

    State.Wheels[0].BrakeThermal.TemperatureC =
        525.0;

    State.Wheels[0].BrakeThermal.ThermalTorqueFactor01 =
        0.72;

    State.Wheels[0].BrakeThermal.Wear01 =
        0.35;

    State.Wheels[0].BrakeThermal.WearTorqueFactor01 =
        0.91;

    FTAVehicleStepOutput Output;

    const FTAVehicleTelemetrySample Sample =
        TATelemetry::CaptureVehicleSample(
            State,
            Output);

    TestTrue(
        TEXT("Steering damage severity maps"),
        FMath::IsNearlyEqual(
            Sample.SteeringRackDamage01,
            0.60,
            1.0e-9));

    TestTrue(
        TEXT("Steering authority maps"),
        FMath::IsNearlyEqual(
            Sample.SteeringCommandAuthority01,
            0.55,
            1.0e-9));

    TestTrue(
        TEXT("Steering free-play maps"),
        FMath::IsNearlyEqual(
            Sample.SteeringRackFreePlayM,
            0.004,
            1.0e-9));

    TestTrue(
        TEXT("Front-right hub damage severity maps"),
        FMath::IsNearlyEqual(
            Sample.WheelHubDamage01[1],
            0.70,
            1.0e-9));

    TestTrue(
        TEXT("Front-right hub brake efficiency maps"),
        FMath::IsNearlyEqual(
            Sample.WheelHubBrakeEfficiency01[1],
            0.45,
            1.0e-9));

    TestTrue(
        TEXT("Front-right hub drive efficiency maps"),
        FMath::IsNearlyEqual(
            Sample.WheelHubDriveEfficiency01[1],
            0.25,
            1.0e-9));

    TestTrue(
        TEXT("Front-right hub bearing drag maps"),
        FMath::IsNearlyEqual(
            Sample.WheelHubBearingDragTorqueNm[1],
            52.0,
            1.0e-9));

    TestTrue(
        TEXT("Front-left suspension damage severity maps"),
        FMath::IsNearlyEqual(
            Sample.SuspensionSpringDamperDamage01[0],
            0.65,
            1.0e-9));

    TestTrue(
        TEXT("Front-left spring efficiency maps"),
        FMath::IsNearlyEqual(
            Sample.SuspensionSpringEfficiency01[0],
            0.45,
            1.0e-9));

    TestTrue(
        TEXT("Front-left damping efficiency maps"),
        FMath::IsNearlyEqual(
            Sample.SuspensionDampingEfficiency01[0],
            0.20,
            1.0e-9));

    TestTrue(
        TEXT("Front-left stop efficiency maps"),
        FMath::IsNearlyEqual(
            Sample.SuspensionStopEfficiency01[0],
            0.85,
            1.0e-9));

    TestTrue(
        TEXT("Front-left anti-roll link damage maps"),
        FMath::IsNearlyEqual(
            Sample.AntiRollLinkDamage01[0],
            0.80,
            1.0e-9));

    TestTrue(
        TEXT("Front-left anti-roll link efficiency maps"),
        FMath::IsNearlyEqual(
            Sample.AntiRollLinkEfficiency01[0],
            0.15,
            1.0e-9));

    TestTrue(
        TEXT("Front-left brake temperature maps"),
        FMath::IsNearlyEqual(
            Sample.BrakeTemperatureC[0],
            525.0,
            1.0e-9));

    TestTrue(
        TEXT("Front-left brake fade factor maps"),
        FMath::IsNearlyEqual(
            Sample.BrakeThermalTorqueFactor01[0],
            0.72,
            1.0e-9));

    TestTrue(
        TEXT("Front-left brake wear maps"),
        FMath::IsNearlyEqual(
            Sample.BrakeWear01[0],
            0.35,
            1.0e-9));

    TestTrue(
        TEXT("Front-left brake wear torque factor maps"),
        FMath::IsNearlyEqual(
            Sample.BrakeWearTorqueFactor01[0],
            0.91,
            1.0e-9));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTATelemetryFunctionalDamageCsvTest,
    "TorqueAtlas.Telemetry.FunctionalDamageCsvChannels",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTATelemetryFunctionalDamageCsvTest::RunTest(
    const FString& Parameters)
{
    FTATelemetryRingBuffer Buffer;

    TestTrue(
        TEXT("Buffer initializes"),
        Buffer.Initialize(1));

    FTAVehicleTelemetrySample Sample;
    Sample.SteeringRackDamage01 = 0.5;
    Sample.SteeringCommandAuthority01 = 0.6;
    Sample.SteeringRackFreePlayM = 0.003;
    Sample.WheelHubDamage01[1] = 0.7;
    Sample.WheelHubBrakeEfficiency01[1] = 0.4;
    Sample.WheelHubDriveEfficiency01[1] = 0.2;
    Sample.WheelHubBearingDragTorqueNm[1] = 50.0;
    Sample.SuspensionSpringDamperDamage01[0] = 0.7;
    Sample.SuspensionSpringEfficiency01[0] = 0.4;
    Sample.SuspensionDampingEfficiency01[0] = 0.2;
    Sample.SuspensionStopEfficiency01[0] = 0.9;
    Sample.AntiRollLinkDamage01[0] = 1.0;
    Sample.AntiRollLinkEfficiency01[0] = 0.0;
    Sample.BrakeTemperatureC[0] = 600.0;
    Sample.BrakeThermalTorqueFactor01[0] = 0.5;
    Sample.BrakeWear01[0] = 0.4;
    Sample.BrakeWearTorqueFactor01[0] = 0.9;

    TestTrue(
        TEXT("Damage sample pushes"),
        Buffer.Push(Sample));

    const FString Csv =
        Buffer.ExportCsv();

    TestTrue(
        TEXT("CSV exposes steering damage channel"),
        Csv.Contains(TEXT("rack_damage")));

    TestTrue(
        TEXT("CSV exposes steering authority channel"),
        Csv.Contains(TEXT("rack_authority")));

    TestTrue(
        TEXT("CSV exposes front-right hub damage channel"),
        Csv.Contains(TEXT("hubdamage_fr")));

    TestTrue(
        TEXT("CSV exposes front-right hub drag channel"),
        Csv.Contains(TEXT("hubdrag_fr_nm")));

    TestTrue(
        TEXT("CSV exposes front-left spring health channel"),
        Csv.Contains(TEXT("susspring_fl")));

    TestTrue(
        TEXT("CSV exposes front-left damping health channel"),
        Csv.Contains(TEXT("susdamping_fl")));

    TestTrue(
        TEXT("CSV exposes front-left anti-roll link health channel"),
        Csv.Contains(TEXT("arblink_fl")));

    TestTrue(
        TEXT("CSV exposes front-left brake temperature channel"),
        Csv.Contains(TEXT("braketemp_fl_c")));

    TestTrue(
        TEXT("CSV exposes front-left brake fade channel"),
        Csv.Contains(TEXT("brakefade_fl")));

    TestTrue(
        TEXT("CSV exposes front-left brake wear channel"),
        Csv.Contains(TEXT("brakewear_fl")));

    return true;
}

#endif
