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
    Definition.FrontLiftCoefficient = -0.089;
    Definition.RearLiftCoefficient = -0.130;
    Definition.DragApplicationPointVehicleLocalM = FVector(0.46, -0.04, 0.52);
    Definition.FrontLiftApplicationPointVehicleLocalM = FVector(1.24, -0.01, 0.24);
    Definition.RearLiftApplicationPointVehicleLocalM = FVector(-1.02, -0.01, 0.24);

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
    TestEqual(TEXT("Front Cl propagates"), Runtime.FrontLiftCoefficient, -0.089);
    TestEqual(TEXT("Rear Cl propagates"), Runtime.RearLiftCoefficient, -0.130);
    TestTrue(
        TEXT("Vehicle-origin application point is converted exactly once to COM-local"),
        Runtime.DragApplicationPointBodyM.Equals(
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
    Runtime.FrontLiftCoefficient = 9.0;
    Runtime.RearLiftCoefficient = 8.0;
    Runtime.DragApplicationPointBodyM = FVector3d(9.0, 9.0, 9.0);
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
    TestEqual(TEXT("Failed compilation preserves runtime front Cl"), Runtime.FrontLiftCoefficient, OriginalRuntime.FrontLiftCoefficient);
    TestEqual(TEXT("Failed compilation preserves runtime rear Cl"), Runtime.RearLiftCoefficient, OriginalRuntime.RearLiftCoefficient);
    TestTrue(
        TEXT("Failed compilation preserves runtime application point"),
        Runtime.DragApplicationPointBodyM.Equals(
            OriginalRuntime.DragApplicationPointBodyM,
            0.0));
    return true;
}

#endif
