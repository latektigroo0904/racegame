#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TAVehicleDefinition.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDefinitionCompileTest,
    "TorqueAtlas.Vehicle.Definition.CompilesDefaultPrototype",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDefinitionCompileTest::RunTest(const FString& Parameters)
{
    UTAVehicleDefinition* Definition = NewObject<UTAVehicleDefinition>();

    FTAVehicleCompiledConfig Config;
    FTAValidationResult Validation;

    const bool bCompiled = Definition->BuildCompiledConfig(Config, Validation);

    TestTrue(TEXT("Default prototype definition compiles"), bCompiled);
    TestFalse(TEXT("Default prototype has no validation errors"), Validation.HasErrors());
    TestEqual(TEXT("Default wheel count"), Config.WheelCount, 4);
    TestTrue(TEXT("Default mass is positive"), Config.MassKg > 0.0);
    TestTrue(TEXT("Physics config hash is populated"), Config.PhysicsConfigHash != 0u);

    TestEqual(
        TEXT("Compiled runtime has four wheel configs"),
        Config.VehicleRuntime.Wheels.Num(),
        4);

    TestEqual(
        TEXT("Compiled runtime has four tire configs"),
        Config.VehicleRuntime.Tires.Num(),
        4);

    TestTrue(
        TEXT("Compiled front geometry validates"),
        TADoubleWishboneSolver::ValidateConfig(
            Config.FourWheelRuntime.FrontAxle.RightGeometry));

    TestTrue(
        TEXT("Compiled rear five-link geometry validates"),
        TAMultiLinkSolver::ValidateConfig(
            Config.FourWheelRuntime.RearAxle.RightGeometry));

    TestTrue(
        TEXT("Default prototype is rear-wheel drive"),
        !Config.VehicleRuntime.Wheels[0].bDriven
        && !Config.VehicleRuntime.Wheels[1].bDriven
        && Config.VehicleRuntime.Wheels[2].bDriven
        && Config.VehicleRuntime.Wheels[3].bDriven);

    const uint32 OriginalHash =
        Config.PhysicsConfigHash;

    Definition->FrontSuspension.SpringRateNPerM +=
        1000.0;

    FTAVehicleCompiledConfig ModifiedConfig;
    FTAValidationResult ModifiedValidation;

    TestTrue(
        TEXT("Modified handling definition still compiles"),
        Definition->BuildCompiledConfig(
            ModifiedConfig,
            ModifiedValidation));

    TestTrue(
        TEXT("Handling-critical spring change alters physics hash"),
        ModifiedConfig.PhysicsConfigHash != OriginalHash);

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDefinitionHashSensitivityTest,
    "TorqueAtlas.Vehicle.Definition.HashChangesWithPhysics",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDefinitionHashSensitivityTest::RunTest(const FString& Parameters)
{
    UTAVehicleDefinition* Definition =
        NewObject<UTAVehicleDefinition>();

    FTAVehicleCompiledConfig Baseline;
    FTAValidationResult BaselineValidation;

    TestTrue(
        TEXT("Baseline definition compiles"),
        Definition->BuildCompiledConfig(
            Baseline,
            BaselineValidation));

    Definition->Tire.RadialStiffnessNPerM +=
        1000.0;

    FTAVehicleCompiledConfig Modified;
    FTAValidationResult ModifiedValidation;

    TestTrue(
        TEXT("Modified definition compiles"),
        Definition->BuildCompiledConfig(
            Modified,
            ModifiedValidation));

    TestTrue(
        TEXT("Handling-critical tire change changes physics config hash"),
        Modified.PhysicsConfigHash
            != Baseline.PhysicsConfigHash);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDefinitionGeometryValidationTest,
    "TorqueAtlas.Vehicle.Definition.RejectsDegenerateSuspension",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDefinitionGeometryValidationTest::RunTest(const FString& Parameters)
{
    UTAVehicleDefinition* Definition =
        NewObject<UTAVehicleDefinition>();

    Definition->FrontSuspension.UpperInnerB =
        Definition->FrontSuspension.UpperInnerA;

    FTAVehicleCompiledConfig Config;
    FTAValidationResult Validation;

    TestFalse(
        TEXT("Degenerate front suspension fails asset compilation"),
        Definition->BuildCompiledConfig(
            Config,
            Validation));

    TestTrue(
        TEXT("Degenerate front suspension emits validation errors"),
        Validation.HasErrors());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDefinitionComTransformTest,
    "TorqueAtlas.Vehicle.Definition.CompilesHardpointsToComLocal",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDefinitionComTransformTest::RunTest(const FString& Parameters)
{
    UTAVehicleDefinition* Definition =
        NewObject<UTAVehicleDefinition>();

    const FVector AuthoredWheelCenter =
        Definition->FrontSuspension.WheelCenter;

    Definition->Mass.CenterOfMassMeters =
        FVector(0.10, 0.0, 0.05);

    FTAVehicleCompiledConfig Config;
    FTAValidationResult Validation;

    TestTrue(
        TEXT("Definition with non-zero COM compiles"),
        Definition->BuildCompiledConfig(
            Config,
            Validation));

    const FVector3d CompiledWheelCenter =
        Config.FourWheelRuntime
            .FrontAxle.RightGeometry
            .Hardpoints.WheelCenterReference;

    TestTrue(
        TEXT("Compiled hardpoint is translated into COM-local frame"),
        CompiledWheelCenter.Equals(
            FVector3d(AuthoredWheelCenter)
                - FVector3d(Definition->Mass.CenterOfMassMeters),
            1.0e-9));

    return true;
}

#endif
