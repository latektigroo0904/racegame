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

#endif
