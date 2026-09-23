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

#endif
