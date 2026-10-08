#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TAFluidPrimitives.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAFluidZeroPressureLeakTest,
    "TorqueAtlas.Vehicle.Fluids.ZeroPressureDifferenceNoLeak",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAFluidZeroPressureLeakTest::RunTest(
    const FString& Parameters)
{
    FTAFluidLeakConfig Config;

    TestEqual(
        TEXT("Equal pressure gives zero mass flow"),
        TAFluidPrimitives::CalculateOrificeMassFlowKgPerSec(
            Config,
            1.0e-5,
            100000.0,
            100000.0),
        0.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAFluidLeakAreaScalingTest,
    "TorqueAtlas.Vehicle.Fluids.LeakFlowScalesWithArea",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAFluidLeakAreaScalingTest::RunTest(
    const FString& Parameters)
{
    FTAFluidLeakConfig Config;

    const double Small =
        TAFluidPrimitives::CalculateOrificeMassFlowKgPerSec(
            Config,
            1.0e-6,
            300000.0,
            100000.0);

    const double Large =
        TAFluidPrimitives::CalculateOrificeMassFlowKgPerSec(
            Config,
            2.0e-6,
            300000.0,
            100000.0);

    TestTrue(
        TEXT("Doubling leak area doubles mass flow"),
        FMath::IsNearlyEqual(
            Large,
            2.0 * Small,
            1.0e-12));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAFluidReservoirConservationTest,
    "TorqueAtlas.Vehicle.Fluids.ReservoirLeakConservesMass",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAFluidReservoirConservationTest::RunTest(
    const FString& Parameters)
{
    FTAFluidLeakConfig Config;

    FTAFluidReservoirState State;
    State.MassKg = 1.0;

    const double InitialMassKg =
        State.MassKg;

    FTAFluidLeakOutput Output;

    TestTrue(
        TEXT("Leak integration succeeds"),
        TAFluidPrimitives::IntegrateReservoirLeak(
            Config,
            1.0e-6,
            300000.0,
            100000.0,
            0.1,
            State,
            Output));

    TestTrue(
        TEXT("Reservoir plus leaked mass is conserved"),
        FMath::IsNearlyEqual(
            State.MassKg
                + State.CumulativeLeakedMassKg,
            InitialMassKg,
            1.0e-12));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAFluidReservoirCannotGoNegativeTest,
    "TorqueAtlas.Vehicle.Fluids.LeakCannotExceedAvailableMass",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAFluidReservoirCannotGoNegativeTest::RunTest(
    const FString& Parameters)
{
    FTAFluidLeakConfig Config;

    FTAFluidReservoirState State;
    State.MassKg = 0.01;

    FTAFluidLeakOutput Output;

    TestTrue(
        TEXT("Large leak integration succeeds"),
        TAFluidPrimitives::IntegrateReservoirLeak(
            Config,
            1.0e-2,
            5.0e6,
            100000.0,
            1.0,
            State,
            Output));

    TestEqual(
        TEXT("Reservoir clamps at zero mass"),
        State.MassKg,
        0.0);

    TestTrue(
        TEXT("Leaked mass cannot exceed initial reservoir mass"),
        Output.LeakedMassThisStepKg
            <= 0.01 + 1.0e-12);

    return true;
}

#endif
