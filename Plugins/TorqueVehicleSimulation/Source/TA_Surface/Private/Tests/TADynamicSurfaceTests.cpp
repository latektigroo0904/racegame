#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TADynamicSurface.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTADynamicSurfaceRainAccumulationTest,
    "TorqueAtlas.Surface.Dynamic.RainAccumulatesWater",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTADynamicSurfaceRainAccumulationTest::RunTest(
    const FString& Parameters)
{
    FTADynamicSurfaceConfig Config;
    Config.MaxDrainageRateMmPerSec = 0.0;
    Config.EvaporationRateMmPerSecAt20C = 0.0;

    FTADynamicSurfaceState State;
    TADynamicSurface::InitializeState(20.0, State);

    FTADynamicSurfaceInput Input;
    Input.PrecipitationRateMmPerHour = 36.0;
    Input.DeltaTimeSeconds = 10.0;

    FTADynamicSurfaceOutput Output;

    TestTrue(
        TEXT("Rain accumulation step succeeds"),
        TADynamicSurface::Step(
            Config,
            Input,
            State,
            Output));

    TestTrue(
        TEXT("Rain increases water depth"),
        Output.WaterDepthMm > 0.0);

    TestTrue(
        TEXT("Rain increases wetness"),
        Output.Wetness01 > 0.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTADynamicSurfaceDrainageTest,
    "TorqueAtlas.Surface.Dynamic.DrainageReducesWater",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTADynamicSurfaceDrainageTest::RunTest(
    const FString& Parameters)
{
    FTADynamicSurfaceConfig Config;
    Config.EvaporationRateMmPerSecAt20C = 0.0;

    FTADynamicSurfaceState LowDrainage;
    FTADynamicSurfaceState HighDrainage;

    LowDrainage.WaterDepthMm = 4.0;
    HighDrainage.WaterDepthMm = 4.0;

    FTADynamicSurfaceInput LowInput;
    LowInput.Drainage01 = 0.1;
    LowInput.DeltaTimeSeconds = 5.0;

    FTADynamicSurfaceInput HighInput =
        LowInput;

    HighInput.Drainage01 = 1.0;

    FTADynamicSurfaceOutput LowOutput;
    FTADynamicSurfaceOutput HighOutput;

    TestTrue(
        TEXT("Low-drainage step succeeds"),
        TADynamicSurface::Step(
            Config,
            LowInput,
            LowDrainage,
            LowOutput));

    TestTrue(
        TEXT("High-drainage step succeeds"),
        TADynamicSurface::Step(
            Config,
            HighInput,
            HighDrainage,
            HighOutput));

    TestTrue(
        TEXT("Higher drainage removes more water"),
        HighOutput.WaterDepthMm
            < LowOutput.WaterDepthMm);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTADynamicSurfaceEvaporationTest,
    "TorqueAtlas.Surface.Dynamic.WindAndHeatIncreaseEvaporation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTADynamicSurfaceEvaporationTest::RunTest(
    const FString& Parameters)
{
    FTADynamicSurfaceConfig Config;
    Config.MaxDrainageRateMmPerSec = 0.0;

    FTADynamicSurfaceState Calm;
    FTADynamicSurfaceState HotWindy;

    Calm.WaterDepthMm = 2.0;
    Calm.SurfaceTemperatureC = 20.0;

    HotWindy = Calm;
    HotWindy.SurfaceTemperatureC = 40.0;

    FTADynamicSurfaceInput CalmInput;
    CalmInput.WindSpeedMps = 0.0;
    CalmInput.DeltaTimeSeconds = 10.0;

    FTADynamicSurfaceInput HotWindyInput =
        CalmInput;

    HotWindyInput.WindSpeedMps = 10.0;

    FTADynamicSurfaceOutput CalmOutput;
    FTADynamicSurfaceOutput HotWindyOutput;

    TestTrue(
        TEXT("Calm evaporation step succeeds"),
        TADynamicSurface::Step(
            Config,
            CalmInput,
            Calm,
            CalmOutput));

    TestTrue(
        TEXT("Hot/windy evaporation step succeeds"),
        TADynamicSurface::Step(
            Config,
            HotWindyInput,
            HotWindy,
            HotWindyOutput));

    TestTrue(
        TEXT("Hot/windy surface evaporates faster"),
        HotWindyOutput.EvaporationRateMmPerSec
            > CalmOutput.EvaporationRateMmPerSec);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTADynamicSurfaceSampleBridgeTest,
    "TorqueAtlas.Surface.Dynamic.OutputMapsToSurfaceSample",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTADynamicSurfaceSampleBridgeTest::RunTest(
    const FString& Parameters)
{
    FTADynamicSurfaceOutput Dynamic;
    Dynamic.WaterDepthMm = 3.0;
    Dynamic.Wetness01 = 0.8;
    Dynamic.SurfaceTemperatureC = 32.0;

    FTASurfaceSample Sample;

    TADynamicSurface::ApplyToSurfaceSample(
        Dynamic,
        Sample);

    TestEqual(
        TEXT("Water depth maps exactly"),
        Sample.WaterDepthMm,
        3.0);

    TestEqual(
        TEXT("Wetness maps exactly"),
        Sample.Wetness01,
        0.8);

    TestEqual(
        TEXT("Temperature maps exactly"),
        Sample.TemperatureC,
        32.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTADynamicSurfaceValidationTest,
    "TorqueAtlas.Surface.Dynamic.ConfigValidation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTADynamicSurfaceValidationTest::RunTest(
    const FString& Parameters)
{
    FTADynamicSurfaceConfig Config;

    TestTrue(
        TEXT("Default dynamic surface config validates"),
        TADynamicSurface::ValidateConfig(Config));

    Config.WetnessSaturationDepthMm = 0.0;

    TestFalse(
        TEXT("Zero wetness saturation depth is rejected"),
        TADynamicSurface::ValidateConfig(Config));

    return true;
}

#endif
