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

    TestTrue(
        TEXT("Clutch torque is capacity limited"),
        FMath::Abs(ClutchTorque) <= 400.0 + KINDA_SMALL_NUMBER);

    TestTrue(
        TEXT("Clutch transmits non-zero reaction torque under slip"),
        FMath::Abs(ClutchTorque) > 0.0);

    const double TemperatureBefore = ClutchState.TemperatureC;

    TAPowertrainSolver::UpdateClutchThermalAndWear(
        ClutchConfig,
        ClutchTorque,
        300.0,
        1.0,
        ClutchState);

    TestTrue(
        TEXT("Clutch slip raises clutch temperature"),
        ClutchState.TemperatureC > TemperatureBefore);

    TestTrue(
        TEXT("Clutch slip accumulates wear"),
        ClutchState.Wear01 > 0.0);

    FTAGearboxRuntimeConfig GearboxConfig;

    const double FirstGear =
        TAPowertrainSolver::GetSelectedGearRatio(GearboxConfig, 1);

    TestTrue(TEXT("First gear is a reduction ratio greater than one"), FirstGear > 1.0);

    const double GearboxInputSpeed = 345.0;
    const double GearboxOutputSpeed =
        TAPowertrainSolver::CalculateGearboxOutputSpeedRadPerSec(
            GearboxInputSpeed,
            FirstGear);

    TestTrue(
        TEXT("Reduction gear lowers output angular speed"),
        GearboxOutputSpeed < GearboxInputSpeed);

    const double GearboxOutputTorque =
        TAPowertrainSolver::CalculateGearboxOutputTorqueNm(
            GearboxConfig,
            100.0,
            FirstGear);

    TestTrue(
        TEXT("Reduction gear multiplies torque before losses"),
        GearboxOutputTorque > 100.0);

    const double AxleSpeed =
        TAPowertrainSolver::CalculateFinalDriveOutputSpeedRadPerSec(
            GearboxConfig,
            GearboxOutputSpeed);

    TestTrue(
        TEXT("Final drive further reduces angular speed"),
        FMath::Abs(AxleSpeed) < FMath::Abs(GearboxOutputSpeed));

    const FTAOpenDifferentialTorqueOutput Diff =
        TAPowertrainSolver::CalculateOpenDifferentialTorque(
            600.0,
            1000.0,
            120.0);

    TestTrue(
        TEXT("Open differential transmits equal wheel torque in quasi-static v0.1 model"),
        FMath::IsNearlyEqual(Diff.LeftWheelTorqueNm, Diff.RightWheelTorqueNm));

    TestTrue(
        TEXT("Low reaction-capacity side limits equal transmitted torque"),
        FMath::Abs(Diff.LeftWheelTorqueNm) <= 120.0 + KINDA_SMALL_NUMBER);

    TestTrue(
        TEXT("Untransmitted torque is reported for later coupled dynamics"),
        Diff.UntransmittedInputTorqueNm > 0.0);

    return true;
}

#endif
