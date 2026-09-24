#include "Misc/AutomationTest.h"
#include "TAStaticLoadEvidencePipeline.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAStaticLoadEvidencePipelineTest,
    "TorqueAtlas.Vehicle.StaticLoad.EvidencePipeline",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAStaticLoadEvidencePipelineTest::RunTest(const FString& Parameters)
{
    FTAStaticLoadEvidencePipelineConfig Config;
    Config.OracleInput.MassKg = 1000.0;
    Config.OracleInput.GravityMagnitudeMps2 = 10.0;
    Config.OracleInput.WheelbaseM = 2.0;
    Config.OracleInput.FrontTrackM = 2.0;
    Config.OracleInput.RearTrackM = 2.0;
    Config.OracleInput.ComFromRearAxleM = 1.0;
    Config.OracleInput.ComLateralM = 0.0;
    Config.Settled.MinimumQualifiedSamples = 3;

    FTAFourWheelRuntimeState State;
    FTAFourWheelStepOutput Output;
    Output.bContactsSolved = true;
    Output.FrontAxle.LeftVehicleContact.VerticalLoadN = 2500.0;
    Output.FrontAxle.RightVehicleContact.VerticalLoadN = 2500.0;
    Output.RearAxle.LeftVehicleContact.VerticalLoadN = 2500.0;
    Output.RearAxle.RightVehicleContact.VerticalLoadN = 2500.0;

    FTAStaticLoadEvidencePipelineState PipelineState;
    FTAStaticLoadEvidencePipelineResult Result;

    TestTrue(TEXT("First evidence tick accepted"), TAStaticLoadEvidencePipeline::Tick(
        Config, State, Output, Config.Adapter.ExpectedDeltaTimeSeconds, PipelineState, Result));
    TestFalse(TEXT("First tick is not qualified"), Result.bQualified);
    TestTrue(TEXT("Second evidence tick accepted"), TAStaticLoadEvidencePipeline::Tick(
        Config, State, Output, Config.Adapter.ExpectedDeltaTimeSeconds, PipelineState, Result));
    TestFalse(TEXT("Second tick is not qualified"), Result.bQualified);
    TestTrue(TEXT("Third evidence tick accepted"), TAStaticLoadEvidencePipeline::Tick(
        Config, State, Output, Config.Adapter.ExpectedDeltaTimeSeconds, PipelineState, Result));
    TestTrue(TEXT("Third tick qualifies"), Result.bQualified);
    TestTrue(TEXT("Balanced evidence passes"), Result.Acceptance.bPass);

    State.Vehicle.Chassis.LinearVelocityWorldMps = FVector3d(1.0, 0.0, 0.0);
    TestTrue(TEXT("Motion tick is processed as sampler reset"), TAStaticLoadEvidencePipeline::Tick(
        Config, State, Output, Config.Adapter.ExpectedDeltaTimeSeconds, PipelineState, Result));
    TestFalse(TEXT("Motion clears qualification"), Result.bQualified);
    TestEqual(TEXT("Motion reset clears consecutive count"), PipelineState.Settled.QualifiedSampleCount, 0);

    State.Vehicle.Chassis.LinearVelocityWorldMps = FVector3d::ZeroVector;
    Output.FrontAxle.LeftVehicleContact.VerticalLoadN = 2750.0;
    Output.FrontAxle.RightVehicleContact.VerticalLoadN = 2750.0;
    Output.RearAxle.LeftVehicleContact.VerticalLoadN = 2250.0;
    Output.RearAxle.RightVehicleContact.VerticalLoadN = 2250.0;
    for (int32 Index = 0; Index < 3; ++Index)
    {
        TestTrue(TEXT("Biased tick accepted"), TAStaticLoadEvidencePipeline::Tick(
            Config, State, Output, Config.Adapter.ExpectedDeltaTimeSeconds, PipelineState, Result));
    }
    TestTrue(TEXT("Biased evidence still qualifies"), Result.bQualified);
    TestFalse(TEXT("Deliberate axle bias fails acceptance"), Result.Acceptance.bPass);
    TestFalse(TEXT("Front axle gate detects bias"), Result.Acceptance.bFrontAxlePass);

    TestFalse(TEXT("Cadence drift fails closed"), TAStaticLoadEvidencePipeline::Tick(
        Config, State, Output, Config.Adapter.ExpectedDeltaTimeSeconds + 0.001, PipelineState, Result));
    TestEqual(TEXT("Cadence failure resets state"), PipelineState.Settled.QualifiedSampleCount, 0);

    return true;
}

#endif
