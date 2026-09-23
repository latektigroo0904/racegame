#include "Misc/AutomationTest.h"
#include "TAVehicleAerodynamicsAssetCompiler.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleAeroAssetCompilerNonDefaultTest,
    "TorqueAtlas.Vehicle.Aerodynamics.AssetCompiler.NonDefaultCompileAndHash",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleAeroAssetCompilerNonDefaultTest::RunTest(const FString& Parameters)
{
    FTAAerodynamicsDefinition Definition;
    Definition.ReferenceAreaM2 = 2.37;
    Definition.DragCoefficient = 0.287;
    Definition.LiftCoefficient = -0.219;
    Definition.ApplicationPointVehicleLocalM = FVector(0.46, -0.04, 0.52);

    const FVector3d CenterOfMass(0.14, -0.01, 0.19);
    uint32 Hash = 0x51A7E001u;
    const uint32 OriginalHash = Hash;
    FTAAerodynamicsConfig Runtime;

    const bool bCompiled =
        TAVehicleAerodynamicsAssetCompiler::CompileValidatedAndHash(
            Definition,
            CenterOfMass,
            Hash,
            Runtime);

    TestTrue(TEXT("Non-default authored aero compiles"), bCompiled);
    TestEqual(TEXT("Area propagates"), Runtime.ReferenceAreaM2, 2.37);
    TestEqual(TEXT("Cd propagates"), Runtime.DragCoefficient, 0.287);
    TestEqual(TEXT("Cl propagates"), Runtime.LiftCoefficient, -0.219);
    TestTrue(
        TEXT("Vehicle-origin application point is converted exactly once to COM-local"),
        Runtime.ApplicationPointBodyLocalM.Equals(
            FVector3d(0.32, -0.03, 0.33),
            1.0e-12));
    TestNotEqual(TEXT("Effective aero changes physics hash"), Hash, OriginalHash);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleAeroAssetCompilerInvalidTransactionalTest,
    "TorqueAtlas.Vehicle.Aerodynamics.AssetCompiler.InvalidIsTransactional",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleAeroAssetCompilerInvalidTransactionalTest::RunTest(const FString& Parameters)
{
    FTAAerodynamicsDefinition Definition;
    Definition.ReferenceAreaM2 = 0.0;

    uint32 Hash = 0xC0FFEE11u;
    const uint32 OriginalHash = Hash;

    FTAAerodynamicsConfig Runtime;
    Runtime.ReferenceAreaM2 = 9.0;
    Runtime.DragCoefficient = 9.0;
    Runtime.LiftCoefficient = 9.0;
    Runtime.ApplicationPointBodyLocalM = FVector3d(9.0, 9.0, 9.0);
    const FTAAerodynamicsConfig OriginalRuntime = Runtime;

    const bool bCompiled =
        TAVehicleAerodynamicsAssetCompiler::CompileValidatedAndHash(
            Definition,
            FVector3d(0.14, -0.01, 0.19),
            Hash,
            Runtime);

    TestFalse(TEXT("Invalid authored aero is rejected"), bCompiled);
    TestEqual(TEXT("Failed compilation does not mutate hash"), Hash, OriginalHash);
    TestEqual(TEXT("Failed compilation preserves runtime area"), Runtime.ReferenceAreaM2, OriginalRuntime.ReferenceAreaM2);
    TestEqual(TEXT("Failed compilation preserves runtime Cd"), Runtime.DragCoefficient, OriginalRuntime.DragCoefficient);
    TestEqual(TEXT("Failed compilation preserves runtime Cl"), Runtime.LiftCoefficient, OriginalRuntime.LiftCoefficient);
    TestTrue(
        TEXT("Failed compilation preserves runtime application point"),
        Runtime.ApplicationPointBodyLocalM.Equals(
            OriginalRuntime.ApplicationPointBodyLocalM,
            0.0));
    return true;
}

#endif
