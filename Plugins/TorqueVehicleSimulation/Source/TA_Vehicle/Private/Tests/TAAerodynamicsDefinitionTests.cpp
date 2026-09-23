#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TAAerodynamicsDefinition.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAAerodynamicsDefinitionComLocalTest,
    "TorqueAtlas.Vehicle.Aerodynamics.Definition.ComLocalCompilation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAAerodynamicsDefinitionComLocalTest::RunTest(const FString& Parameters)
{
    FTAAerodynamicsDefinition Definition;
    Definition.ReferenceAreaM2 = 2.25;
    Definition.DragCoefficient = 0.29;
    Definition.LiftCoefficient = -0.18;
    Definition.ApplicationPointVehicleLocalM = FVector(0.30, 0.0, 0.45);

    FTAAerodynamicsConfig Runtime;
    const bool bCompiled = TAAerodynamicsDefinition::Compile(
        Definition,
        FVector3d(0.10, 0.0, 0.20),
        Runtime);

    TestTrue(TEXT("Valid aero authoring compiles"), bCompiled);
    TestEqual(TEXT("Area is preserved"), Runtime.ReferenceAreaM2, 2.25);
    TestEqual(TEXT("Cd is preserved"), Runtime.DragCoefficient, 0.29);
    TestEqual(TEXT("Cl is preserved"), Runtime.LiftCoefficient, -0.18);
    TestTrue(
        TEXT("Application point is converted from vehicle origin to COM-local"),
        Runtime.ApplicationPointBodyM.Equals(FVector3d(0.20, 0.0, 0.25), 1.0e-12));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAAerodynamicsDefinitionHashTest,
    "TorqueAtlas.Vehicle.Aerodynamics.Definition.HashSensitivity",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAAerodynamicsDefinitionHashTest::RunTest(const FString& Parameters)
{
    FTAAerodynamicsConfig A;
    FTAAerodynamicsConfig B = A;

    const uint32 HashA = TAAerodynamicsDefinition::HashRuntimeConfig(0u, A);
    B.LiftCoefficient -= 0.01;
    const uint32 HashB = TAAerodynamicsDefinition::HashRuntimeConfig(0u, B);

    TestNotEqual(TEXT("Effective aero coefficient changes physics hash contribution"), HashA, HashB);

    B = A;
    B.ApplicationPointBodyM.X += 0.01;
    const uint32 HashPoint = TAAerodynamicsDefinition::HashRuntimeConfig(0u, B);
    TestNotEqual(TEXT("Effective aero application point changes physics hash contribution"), HashA, HashPoint);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAAerodynamicsDefinitionEffectiveHashInvariantTest,
    "TorqueAtlas.Vehicle.Aerodynamics.Definition.EffectiveHashCoordinateInvariant",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAAerodynamicsDefinitionEffectiveHashInvariantTest::RunTest(const FString& Parameters)
{
    FTAAerodynamicsDefinition A;
    A.ReferenceAreaM2 = 2.31;
    A.DragCoefficient = 0.287;
    A.LiftCoefficient = -0.214;
    A.ApplicationPointVehicleLocalM = FVector(0.42, -0.03, 0.51);

    FTAAerodynamicsDefinition B = A;

    const FVector3d ComA(0.12, -0.01, 0.18);
    const FVector3d OriginShift(0.37, 0.08, -0.14);
    const FVector3d ComB = ComA + OriginShift;
    B.ApplicationPointVehicleLocalM =
        FVector(FVector3d(A.ApplicationPointVehicleLocalM) + OriginShift);

    FTAAerodynamicsConfig RuntimeA;
    FTAAerodynamicsConfig RuntimeB;

    const bool bCompiledA = TAAerodynamicsDefinition::Compile(A, ComA, RuntimeA);
    const bool bCompiledB = TAAerodynamicsDefinition::Compile(B, ComB, RuntimeB);

    TestTrue(TEXT("First coordinate representation compiles"), bCompiledA);
    TestTrue(TEXT("Shifted coordinate representation compiles"), bCompiledB);
    TestTrue(
        TEXT("Equivalent authored origin/COM shift preserves effective COM-local application point"),
        RuntimeA.ApplicationPointBodyM.Equals(RuntimeB.ApplicationPointBodyM, 1.0e-12));

    const uint32 HashA = TAAerodynamicsDefinition::HashRuntimeConfig(0x51A7u, RuntimeA);
    const uint32 HashB = TAAerodynamicsDefinition::HashRuntimeConfig(0x51A7u, RuntimeB);
    TestEqual(
        TEXT("Physics hash contribution depends on effective runtime aero, not authored origin representation"),
        HashA,
        HashB);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAAerodynamicsDefinitionRejectsInvalidTest,
    "TorqueAtlas.Vehicle.Aerodynamics.Definition.Validation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAAerodynamicsDefinitionRejectsInvalidTest::RunTest(const FString& Parameters)
{
    FTAAerodynamicsDefinition Definition;
    Definition.ReferenceAreaM2 = 0.0;

    FTAAerodynamicsConfig Runtime;
    TestFalse(
        TEXT("Zero reference area is rejected"),
        TAAerodynamicsDefinition::Compile(
            Definition,
            FVector3d::ZeroVector,
            Runtime));

    Definition = FTAAerodynamicsDefinition{};
    Definition.DragCoefficient = -0.1;
    TestFalse(
        TEXT("Negative drag coefficient is rejected"),
        TAAerodynamicsDefinition::Validate(Definition));
    return true;
}

#endif
