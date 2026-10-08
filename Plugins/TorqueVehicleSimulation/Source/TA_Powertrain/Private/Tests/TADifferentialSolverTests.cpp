#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TADifferentialSolver.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTADifferentialOpenTest,
    "TorqueAtlas.Powertrain.Differential.OpenEqualTorque",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTADifferentialOpenTest::RunTest(
    const FString& Parameters)
{
    FTADifferentialConfig Config;
    Config.Type = ETADifferentialType::Open;
    Config.MechanicalEfficiency01 = 1.0;

    FTADifferentialInput Input;
    Input.InputTorqueNm = 400.0;
    Input.LeftReactionCapacityNm = 500.0;
    Input.RightReactionCapacityNm = 500.0;
    Input.LeftAngularSpeedRadPerSec = 10.0;
    Input.RightAngularSpeedRadPerSec = 30.0;

    FTADifferentialOutput Output;

    TestTrue(
        TEXT("Open differential solve succeeds"),
        TADifferentialSolver::Solve(
            Config,
            Input,
            Output));

    TestEqual(
        TEXT("Open differential left torque is half"),
        Output.LeftWheelTorqueNm,
        200.0);

    TestEqual(
        TEXT("Open differential right torque is half"),
        Output.RightWheelTorqueNm,
        200.0);

    TestEqual(
        TEXT("Open differential does not create lock torque"),
        Output.BiasTorqueNm,
        0.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTADifferentialOpenWeakSideTest,
    "TorqueAtlas.Powertrain.Differential.OpenWeakSideLimitsBoth",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTADifferentialOpenWeakSideTest::RunTest(
    const FString& Parameters)
{
    FTADifferentialConfig Config;
    Config.Type = ETADifferentialType::Open;
    Config.MechanicalEfficiency01 = 1.0;

    FTADifferentialInput Input;
    Input.InputTorqueNm = 400.0;
    Input.LeftReactionCapacityNm = 50.0;
    Input.RightReactionCapacityNm = 500.0;

    FTADifferentialOutput Output;

    TestTrue(
        TEXT("Weak-side open differential solve succeeds"),
        TADifferentialSolver::Solve(
            Config,
            Input,
            Output));

    TestEqual(
        TEXT("Weak side receives its reaction capacity"),
        Output.LeftWheelTorqueNm,
        50.0);

    TestEqual(
        TEXT("Strong side is limited to equal open-diff torque"),
        Output.RightWheelTorqueNm,
        50.0);

    TestEqual(
        TEXT("Untransmitted input torque is explicit"),
        Output.UntransmittedInputTorqueNm,
        300.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTADifferentialClutchLsdTest,
    "TorqueAtlas.Powertrain.Differential.ClutchLsdOpposesSpeedDifference",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTADifferentialClutchLsdTest::RunTest(
    const FString& Parameters)
{
    FTADifferentialConfig Config;
    Config.Type = ETADifferentialType::ClutchLsd;
    Config.MechanicalEfficiency01 = 1.0;
    Config.PreloadTorqueNm = 20.0;
    Config.LockStiffnessNmsPerRad = 10.0;
    Config.DriveRampGain = 0.0;
    Config.MaxLockTorqueNm = 200.0;

    FTADifferentialInput Input;
    Input.InputTorqueNm = 400.0;
    Input.LeftAngularSpeedRadPerSec = 30.0;
    Input.RightAngularSpeedRadPerSec = 10.0;

    FTADifferentialOutput Output;

    TestTrue(
        TEXT("Clutch LSD solve succeeds"),
        TADifferentialSolver::Solve(
            Config,
            Input,
            Output));

    TestTrue(
        TEXT("Faster left side is biased down"),
        Output.LeftWheelTorqueNm
            < Output.RightWheelTorqueNm);

    TestTrue(
        TEXT("Clutch LSD dissipates power while slipping"),
        Output.DissipatedPowerW > 0.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTADifferentialViscousTest,
    "TorqueAtlas.Powertrain.Differential.ViscousScalesWithSpeedDifference",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTADifferentialViscousTest::RunTest(
    const FString& Parameters)
{
    FTADifferentialConfig Config;
    Config.Type = ETADifferentialType::Viscous;
    Config.MechanicalEfficiency01 = 1.0;
    Config.ViscousCoefficientNmsPerRad = 5.0;
    Config.ViscousMaxTorqueNm = 500.0;

    FTADifferentialInput SmallInput;
    SmallInput.InputTorqueNm = 600.0;
    SmallInput.LeftAngularSpeedRadPerSec = 20.0;
    SmallInput.RightAngularSpeedRadPerSec = 10.0;

    FTADifferentialInput LargeInput =
        SmallInput;

    LargeInput.LeftAngularSpeedRadPerSec = 40.0;

    FTADifferentialOutput SmallOutput;
    FTADifferentialOutput LargeOutput;

    TestTrue(
        TEXT("Small viscous solve succeeds"),
        TADifferentialSolver::Solve(
            Config,
            SmallInput,
            SmallOutput));

    TestTrue(
        TEXT("Large viscous solve succeeds"),
        TADifferentialSolver::Solve(
            Config,
            LargeInput,
            LargeOutput));

    TestTrue(
        TEXT("Larger side-speed difference creates larger bias"),
        FMath::Abs(LargeOutput.BiasTorqueNm)
            > FMath::Abs(SmallOutput.BiasTorqueNm));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTADifferentialHelicalTest,
    "TorqueAtlas.Powertrain.Differential.HelicalRespectsTorqueBiasRatio",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTADifferentialHelicalTest::RunTest(
    const FString& Parameters)
{
    FTADifferentialConfig Config;
    Config.Type = ETADifferentialType::Helical;
    Config.MechanicalEfficiency01 = 1.0;
    Config.TorqueBiasRatio = 3.0;

    FTADifferentialInput Input;
    Input.InputTorqueNm = 400.0;
    Input.LeftReactionCapacityNm = 80.0;
    Input.RightReactionCapacityNm = 500.0;

    FTADifferentialOutput Output;

    TestTrue(
        TEXT("Helical differential solve succeeds"),
        TADifferentialSolver::Solve(
            Config,
            Input,
            Output));

    TestTrue(
        TEXT("Strong-side torque does not exceed configured TBR"),
        Output.RightWheelTorqueNm
            <= Config.TorqueBiasRatio
                * Output.LeftWheelTorqueNm
                + 1.0e-9);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTADifferentialActiveTest,
    "TorqueAtlas.Powertrain.Differential.ActiveBiasConservesAxleTorque",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTADifferentialActiveTest::RunTest(
    const FString& Parameters)
{
    FTADifferentialConfig Config;
    Config.Type = ETADifferentialType::Active;
    Config.MechanicalEfficiency01 = 1.0;
    Config.ActiveMaxBiasTorqueNm = 100.0;

    FTADifferentialInput Input;
    Input.InputTorqueNm = 400.0;
    Input.ActiveBiasRequestNm = 50.0;

    FTADifferentialOutput Output;

    TestTrue(
        TEXT("Active differential solve succeeds"),
        TADifferentialSolver::Solve(
            Config,
            Input,
            Output));

    TestEqual(
        TEXT("Active bias lowers left torque"),
        Output.LeftWheelTorqueNm,
        150.0);

    TestEqual(
        TEXT("Active bias raises right torque"),
        Output.RightWheelTorqueNm,
        250.0);

    TestTrue(
        TEXT("Active bias conserves transmitted axle torque"),
        FMath::IsNearlyEqual(
            Output.LeftWheelTorqueNm
                + Output.RightWheelTorqueNm,
            400.0,
            1.0e-9));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTADifferentialValidationTest,
    "TorqueAtlas.Powertrain.Differential.ConfigValidation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTADifferentialValidationTest::RunTest(
    const FString& Parameters)
{
    FTADifferentialConfig Config;

    TestTrue(
        TEXT("Default differential config validates"),
        TADifferentialSolver::ValidateConfig(Config));

    Config.TorqueBiasRatio = 0.5;

    TestFalse(
        TEXT("TBR below one is rejected"),
        TADifferentialSolver::ValidateConfig(Config));

    return true;
}

#endif
