#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TATireSolver.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTATireSolverSmokeTest,
    "TorqueAtlas.Tire.Solver.BasicForces",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTATireSolverSmokeTest::RunTest(const FString& Parameters)
{
    FTATireRuntimeConfig Config;
    FTATireRuntimeState State;

    FTATireSolveInput Input;
    Input.VerticalLoadN = 3500.0;
    Input.LongitudinalVelocityMps = 20.0;
    Input.LateralVelocityMps = 1.0;
    Input.WheelAngularSpeedRadPerSec =
        21.0 / Config.UnloadedRadiusM;

    const FTATireSolveOutput Dry = TATireSolver::Solve(Config, State, Input);

    TestTrue(TEXT("Positive drive slip produces positive longitudinal force"), Dry.LongitudinalForceN > 0.0);
    TestTrue(TEXT("Lateral velocity produces lateral tire force"), FMath::Abs(Dry.LateralForceN) > 0.0);
    TestTrue(TEXT("Dry tire retains road-supported load"), Dry.RoadSupportedLoadN > 0.0);

    Input.Surface.WaterDepthMm = 8.0;
    Input.LongitudinalVelocityMps = 45.0;
    Input.WheelAngularSpeedRadPerSec =
        46.0 / Config.UnloadedRadiusM;

    const FTATireSolveOutput WetHighSpeed = TATireSolver::Solve(Config, State, Input);

    TestTrue(TEXT("Standing water generates hydro fraction"), WetHighSpeed.HydroFraction01 > 0.0);
    TestTrue(TEXT("Hydro lift reduces road-supported load"), WetHighSpeed.RoadSupportedLoadN < Input.VerticalLoadN);

    State.TreadDepthMm = 1.5;
    const double WornHydro = TATireSolver::CalculateHydroFraction01(Config, State, Input);

    State.TreadDepthMm = Config.NewTreadDepthMm;
    const double NewHydro = TATireSolver::CalculateHydroFraction01(Config, State, Input);

    TestTrue(TEXT("Worn tire hydroplanes at least as strongly as new tire in same conditions"), WornHydro >= NewHydro);

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTATireThermalWearTest,
    "TorqueAtlas.Tire.Solver.ThermalPressureAndWear",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTATireThermalWearTest::RunTest(const FString& Parameters)
{
    FTATireRuntimeConfig Config;

    // Accelerated values keep the regression short while preserving the same equations.
    Config.SurfaceThermalMassJPerC = 600.0;
    Config.CarcassThermalMassJPerC = 1800.0;
    Config.SurfaceToCarcassConductanceWPerC = 140.0;
    Config.CarcassToAmbientConductanceWPerC = 20.0;
    Config.InternalAirTimeConstantSeconds = 2.0;
    Config.WearEnergyCapacityJ = 1.0e6;

    FTATireRuntimeState State;
    State.PressureKPa = Config.ReferencePressureKPa;
    State.TreadDepthMm = Config.NewTreadDepthMm;

    FTATireSolveInput Input;
    Input.VerticalLoadN = 3500.0;
    Input.LongitudinalVelocityMps = 20.0;
    Input.LateralVelocityMps = 2.0;
    Input.WheelAngularSpeedRadPerSec =
        30.0 / Config.UnloadedRadiusM;
    Input.Surface.TemperatureC = 20.0;

    const double InitialSurfaceTemperature =
        State.SurfaceTemperatureC;

    const double InitialPressure =
        State.PressureKPa;

    for (int32 Step = 0; Step < 600; ++Step)
    {
        const FTATireSolveOutput Output =
            TATireSolver::Solve(
                Config,
                State,
                Input);

        TATireSolver::UpdateThermalPressureAndWear(
            Config,
            Input,
            Output,
            1.0 / 60.0,
            State);
    }

    TestTrue(
        TEXT("Sustained slip heats tire surface"),
        State.SurfaceTemperatureC
            > InitialSurfaceTemperature);

    TestTrue(
        TEXT("Heat reaches carcass"),
        State.CarcassTemperatureC > 20.0);

    TestTrue(
        TEXT("Internal air warming increases pressure"),
        State.PressureKPa > InitialPressure);

    TestTrue(
        TEXT("Dissipated energy accumulates wear"),
        State.Wear01 > 0.0);

    TestTrue(
        TEXT("Wear reduces tread depth"),
        State.TreadDepthMm < Config.NewTreadDepthMm);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTATireTemperatureGripTest,
    "TorqueAtlas.Tire.Solver.TemperatureGripWindow",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTATireTemperatureGripTest::RunTest(const FString& Parameters)
{
    FTATireRuntimeConfig Config;

    const double Cold =
        TATireSolver::CalculateTemperatureGripFactor(
            Config,
            20.0);

    const double Optimal =
        TATireSolver::CalculateTemperatureGripFactor(
            Config,
            Config.OptimalSurfaceTemperatureC);

    const double Hot =
        TATireSolver::CalculateTemperatureGripFactor(
            Config,
            Config.HotGripTemperatureC);

    TestTrue(
        TEXT("Optimal temperature has more grip than cold tire"),
        Optimal > Cold);

    TestTrue(
        TEXT("Optimal temperature has more grip than overheated tire"),
        Optimal > Hot);

    TestTrue(
        TEXT("Optimal temperature factor is one"),
        FMath::IsNearlyEqual(Optimal, 1.0, 1.0e-9));

    return true;
}

#endif
