#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TAPowertrainSolver.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAPowertrainSolverSmokeTest,
    "TorqueAtlas.Powertrain.Solver.BasicTorqueBalance",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAPowertrainSolverSmokeTest::RunTest(const FString& Parameters)
{
    FTAEngineRuntimeConfig EngineConfig;
    EngineConfig.CrankInertiaKgm2 = 0.20;

    const double StartOmega = 100.0;
    const double NextOmega =
        TAPowertrainSolver::IntegrateEngineAngularSpeed(
            EngineConfig,
            StartOmega,
            200.0,
            0.0,
            1.0 / 240.0);

    TestTrue(TEXT("Positive net torque accelerates crankshaft"), NextOmega > StartOmega);

    FTAClutchRuntimeConfig ClutchConfig;
    ClutchConfig.MaxTorqueCapacityNm = 400.0;
    ClutchConfig.CouplingStiffnessNms = 25.0;

    FTAClutchRuntimeState ClutchState;
    ClutchState.Engagement = 1.0;

    const double ClutchTorque =
        TAPowertrainSolver::CalculateClutchTorqueNm(
            ClutchConfig,
            ClutchState,
            300.0,
            0.0);

    TestTrue(TEXT("Clutch torque is capacity limited"), FMath::Abs(ClutchTorque) <= 400.0 + KINDA_SMALL_NUMBER);
    TestTrue(TEXT("Clutch opposes non-zero relative motion through reaction pairing"), FMath::Abs(ClutchTorque) > 0.0);

    return true;
}

#endif
