#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TAVehicleDefinition.h"

namespace
{
    UTAVehicleDefinition* MakeAeroAssetFixture()
    {
        UTAVehicleDefinition* Definition = NewObject<UTAVehicleDefinition>();
        Definition->Mass.CenterOfMassMeters = FVector(0.14, -0.01, 0.19);
        Definition->Aerodynamics.ReferenceAreaM2 = 2.37;
        Definition->Aerodynamics.DragCoefficient = 0.287;
        Definition->Aerodynamics.FrontLiftCoefficient = -0.089;
        Definition->Aerodynamics.RearLiftCoefficient = -0.130;
        Definition->Aerodynamics.DragApplicationPointVehicleLocalM = FVector(0.46, -0.04, 0.52);
        Definition->Aerodynamics.FrontLiftApplicationPointVehicleLocalM = FVector(1.24, -0.01, 0.24);
        Definition->Aerodynamics.RearLiftApplicationPointVehicleLocalM = FVector(-1.02, -0.01, 0.24);
        return Definition;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDefinitionAerodynamicsPropagationTest,
    "TorqueAtlas.Vehicle.Definition.Aerodynamics.NonDefaultPropagation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDefinitionAerodynamicsPropagationTest::RunTest(const FString& Parameters)
{
    UTAVehicleDefinition* Definition = MakeAeroAssetFixture();

    FTAVehicleCompiledConfig Config;
    FTAValidationResult Validation;
    const bool bCompiled = Definition->BuildCompiledConfig(Config, Validation);

    TestTrue(TEXT("Non-default aero vehicle asset compiles"), bCompiled);
    TestFalse(TEXT("Non-default aero fixture has no validation errors"), Validation.HasErrors());

    const FTAAerodynamicsConfig& Runtime = Config.VehicleRuntime.Aerodynamics;
    TestEqual(TEXT("Reference area propagates"), Runtime.ReferenceAreaM2, 2.37);
    TestEqual(TEXT("Drag coefficient propagates"), Runtime.DragCoefficient, 0.287);
    TestEqual(TEXT("Front lift coefficient propagates"), Runtime.FrontLiftCoefficient, -0.089);
    TestEqual(TEXT("Rear lift coefficient propagates"), Runtime.RearLiftCoefficient, -0.130);
    TestTrue(
        TEXT("Application point is converted exactly once to COM-local coordinates"),
        Runtime.DragApplicationPointBodyM.Equals(FVector3d(0.32, -0.03, 0.33), 1.0e-12));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDefinitionAerodynamicsInvalidTest,
    "TorqueAtlas.Vehicle.Definition.Aerodynamics.RejectsInvalidAuthoring",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDefinitionAerodynamicsInvalidTest::RunTest(const FString& Parameters)
{
    UTAVehicleDefinition* Definition = MakeAeroAssetFixture();
    Definition->Aerodynamics.ReferenceAreaM2 = 0.0;

    FTAVehicleCompiledConfig Config;
    FTAValidationResult Validation;

    TestFalse(
        TEXT("Invalid authored aero fails canonical vehicle compilation"),
        Definition->BuildCompiledConfig(Config, Validation));
    TestTrue(TEXT("Invalid authored aero emits a validation error"), Validation.HasErrors());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDefinitionAerodynamicsHashSensitivityTest,
    "TorqueAtlas.Vehicle.Definition.Aerodynamics.HashSensitivity",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDefinitionAerodynamicsHashSensitivityTest::RunTest(const FString& Parameters)
{
    UTAVehicleDefinition* Definition = MakeAeroAssetFixture();

    FTAVehicleCompiledConfig Baseline;
    FTAValidationResult BaselineValidation;
    TestTrue(
        TEXT("Baseline aero fixture compiles"),
        Definition->BuildCompiledConfig(Baseline, BaselineValidation));

    Definition->Aerodynamics.DragCoefficient += 0.013;

    FTAVehicleCompiledConfig Modified;
    FTAValidationResult ModifiedValidation;
    TestTrue(
        TEXT("Modified aero fixture compiles"),
        Definition->BuildCompiledConfig(Modified, ModifiedValidation));

    TestNotEqual(
        TEXT("Effective aerodynamic change alters canonical physics hash"),
        Baseline.PhysicsConfigHash,
        Modified.PhysicsConfigHash);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDefinitionAerodynamicsDeterminismTest,
    "TorqueAtlas.Vehicle.Definition.Aerodynamics.DeterministicCompileAndHash",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDefinitionAerodynamicsDeterminismTest::RunTest(const FString& Parameters)
{
    UTAVehicleDefinition* Definition = MakeAeroAssetFixture();

    FTAVehicleCompiledConfig First;
    FTAVehicleCompiledConfig Second;
    FTAValidationResult FirstValidation;
    FTAValidationResult SecondValidation;

    TestTrue(
        TEXT("First compile succeeds"),
        Definition->BuildCompiledConfig(First, FirstValidation));
    TestTrue(
        TEXT("Second compile succeeds"),
        Definition->BuildCompiledConfig(Second, SecondValidation));

    TestEqual(
        TEXT("Repeated canonical compilation produces the same physics hash"),
        First.PhysicsConfigHash,
        Second.PhysicsConfigHash);
    TestTrue(
        TEXT("Repeated compilation produces the same COM-local aero point"),
        First.VehicleRuntime.Aerodynamics.DragApplicationPointBodyM.Equals(
            Second.VehicleRuntime.Aerodynamics.DragApplicationPointBodyM,
            1.0e-12));

    return true;
}

#endif
