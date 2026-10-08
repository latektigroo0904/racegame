#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TABrakeControlStack.h"

namespace
{
    int32 Corner(const ETABrakeCornerIndex Index)
    {
        return static_cast<int32>(Index);
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTABrakeControlStackSplitMuTest,
    "TorqueAtlas.Vehicle.Brakes.ControlStack.SplitMuModulatesCornersIndependently",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTABrakeControlStackSplitMuTest::RunTest(
    const FString& Parameters)
{
    FTABrakeControlStackConfig Config;
    Config.Hydraulics.PressureRiseRatePaPerSec = 1.0e12;
    Config.Hydraulics.PressureReleaseRatePaPerSec = 1.0e12;
    Config.Abs.PressureReleaseRate01PerSec = 20.0;

    FTABrakeControlStackState State;
    TABrakeControlStack::InitializeState(
        Config,
        State);

    FTABrakeControlStackInput Input;
    Input.BrakePedal01 = 1.0;
    Input.VehicleSpeedMps = 25.0;
    Input.DeltaTimeSeconds = 0.05;

    const int32 FrontLeft =
        Corner(ETABrakeCornerIndex::FrontLeft);

    const int32 FrontRight =
        Corner(ETABrakeCornerIndex::FrontRight);

    Input.SlipRatio[FrontLeft] = -0.30;
    Input.SlipRatio[FrontRight] = -0.05;

    FTABrakeControlStackOutput Output;

    TestTrue(
        TEXT("Split-mu brake control step succeeds"),
        TABrakeControlStack::Step(
            Config,
            Input,
            State,
            Output));

    TestTrue(
        TEXT("Locking front-left enters ABS release"),
        Output.Abs[FrontLeft].Mode
            == ETAAbsMode::Release);

    TestTrue(
        TEXT("Stable front-right remains build mode"),
        Output.Abs[FrontRight].Mode
            == ETAAbsMode::Build);

    TestTrue(
        TEXT("Front-left pressure modulation is lower than front-right"),
        Output.Abs[FrontLeft].PressureModulation01
            < Output.Abs[FrontRight].PressureModulation01);

    TestTrue(
        TEXT("Front-left hydraulic target pressure is lower than front-right"),
        Output.Hydraulics.CornerTargetPressurePa[FrontLeft]
            < Output.Hydraulics.CornerTargetPressurePa[FrontRight]);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTABrakeControlStackNoBrakeTest,
    "TorqueAtlas.Vehicle.Brakes.ControlStack.NoBrakeProducesZeroPressure",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTABrakeControlStackNoBrakeTest::RunTest(
    const FString& Parameters)
{
    FTABrakeControlStackConfig Config;

    FTABrakeControlStackState State;
    TABrakeControlStack::InitializeState(
        Config,
        State);

    FTABrakeControlStackInput Input;
    Input.BrakePedal01 = 0.0;
    Input.VehicleSpeedMps = 20.0;

    FTABrakeControlStackOutput Output;

    TestTrue(
        TEXT("No-brake control stack step succeeds"),
        TABrakeControlStack::Step(
            Config,
            Input,
            State,
            Output));

    for (int32 CornerIndex = 0;
         CornerIndex < TABrakeCornerCount;
         ++CornerIndex)
    {
        TestEqual(
            TEXT("No-brake target pressure is zero"),
            Output.Hydraulics.CornerTargetPressurePa[CornerIndex],
            0.0);
    }

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTABrakeControlStackLowSpeedTest,
    "TorqueAtlas.Vehicle.Brakes.ControlStack.LowSpeedAbsTransparent",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTABrakeControlStackLowSpeedTest::RunTest(
    const FString& Parameters)
{
    FTABrakeControlStackConfig Config;
    Config.Hydraulics.PressureRiseRatePaPerSec = 1.0e12;

    FTABrakeControlStackState State;
    TABrakeControlStack::InitializeState(
        Config,
        State);

    FTABrakeControlStackInput Input;
    Input.BrakePedal01 = 1.0;
    Input.VehicleSpeedMps = 0.5;

    for (int32 CornerIndex = 0;
         CornerIndex < TABrakeCornerCount;
         ++CornerIndex)
    {
        Input.SlipRatio[CornerIndex] = -1.0;
    }

    FTABrakeControlStackOutput Output;

    TestTrue(
        TEXT("Low-speed brake stack step succeeds"),
        TABrakeControlStack::Step(
            Config,
            Input,
            State,
            Output));

    for (int32 CornerIndex = 0;
         CornerIndex < TABrakeCornerCount;
         ++CornerIndex)
    {
        TestTrue(
            TEXT("ABS remains inactive at low speed"),
            Output.Abs[CornerIndex].Mode
                == ETAAbsMode::Inactive);

        TestTrue(
            TEXT("Full pressure modulation remains available at low speed"),
            FMath::IsNearlyEqual(
                Output.Abs[CornerIndex].PressureModulation01,
                1.0,
                1.0e-12));
    }

    return true;
}

#endif
