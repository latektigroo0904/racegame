#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TARearAxleRuntime.h"

namespace
{
    FTARearAxleRuntimeConfig MakeRearAxleConfig()
    {
        FTARearAxleRuntimeConfig Config;

        const FVector3d Chassis[TARearMultiLinkCount] =
        {
            FVector3d(-1.15, 0.35, -0.18),
            FVector3d(-1.50, 0.36, -0.20),
            FVector3d(-1.10, 0.34, -0.46),
            FVector3d(-1.53, 0.33, -0.47),
            FVector3d(-1.50, 0.38, -0.36)
        };

        const FVector3d Upright[TARearMultiLinkCount] =
        {
            FVector3d(-1.20, 0.68, -0.24),
            FVector3d(-1.42, 0.69, -0.25),
            FVector3d(-1.18, 0.72, -0.49),
            FVector3d(-1.45, 0.73, -0.50),
            FVector3d(-1.48, 0.71, -0.38)
        };

        for (int32 Index = 0;
             Index < TARearMultiLinkCount;
             ++Index)
        {
            Config.RightGeometry.Links[Index].ChassisPickupReference =
                Chassis[Index];

            Config.RightGeometry.Links[Index].UprightPickupReference =
                Upright[Index];
        }

        Config.RightGeometry.WheelCenterReference =
            FVector3d(-1.31, 0.765, -0.45);

        Config.RightGeometry.DamperChassisReference =
            FVector3d(-1.31, 0.40, -0.05);

        Config.RightGeometry.DamperUprightReference =
            FVector3d(-1.31, 0.64, -0.40);

        Config.RightGeometry.WheelForwardReference =
            FVector3d(1.0, 0.0, 0.0);

        Config.RightGeometry.WheelUpReference =
            FVector3d(0.0, 0.0, 1.0);

        Config.RightGeometry.MaxIterations = 180;
        Config.RightGeometry.PositionToleranceM = 0.001;

        Config.LeftSuspension.SpringRateNPerM = 50000.0;
        Config.LeftSuspension.StaticSpringCompressionM = 0.08;
        Config.LeftSuspension.BumpDampingNsPerM = 3800.0;
        Config.LeftSuspension.ReboundDampingNsPerM = 5200.0;

        Config.RightSuspension =
            Config.LeftSuspension;

        Config.AntiRollBar.CouplingRateNPerM = 9000.0;
        Config.AntiRollBar.MaxTransferForceN = 2500.0;

        return Config;
    }

    FTARoadPlane MakeRearRoad()
    {
        FTARoadPlane Road;
        Road.NormalWorld = FVector3d(0.0, 0.0, 1.0);
        Road.Surface.Material = ETASurfaceMaterial::FreshAsphalt;
        return Road;
    }

    FTAChassisState MakeRearChassis()
    {
        FTAChassisState Chassis;
        Chassis.PositionWorldM =
            FVector3d(0.0, 0.0, 0.777);

        return Chassis;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTARearAxleSymmetryTest,
    "TorqueAtlas.Suspension.RearAxle.SymmetricContact",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTARearAxleSymmetryTest::RunTest(const FString& Parameters)
{
    const FTARearAxleRuntimeConfig Config =
        MakeRearAxleConfig();

    FTARearAxleSolveInput Input;
    Input.LeftRoad = MakeRearRoad();
    Input.RightRoad = MakeRearRoad();

    FTARearAxleRuntimeState State;
    FTARearAxleSolveOutput Output;

    TestTrue(
        TEXT("Rear axle resolves"),
        TARearAxleRuntime::Resolve(
            MakeRearChassis(),
            Config,
            Input,
            0.327,
            0.327,
            1.0 / 240.0,
            State,
            Output));

    TestTrue(
        TEXT("Both rear wheels contact road"),
        Output.LeftContact.bInContact
        && Output.RightContact.bInContact);

    TestTrue(
        TEXT("Rear loads mirror closely"),
        FMath::Abs(
            Output.LeftContact.VerticalLoadN
            - Output.RightContact.VerticalLoadN)
            < 50.0);

    TestTrue(
        TEXT("Rear toe mirrors at symmetric reference"),
        FMath::Abs(
            Output.LeftToeRad
            + Output.RightToeRad)
            < FMath::DegreesToRadians(0.1));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTARearAxleRoadSplitTest,
    "TorqueAtlas.Suspension.RearAxle.AsymmetricRoadAntiRoll",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTARearAxleRoadSplitTest::RunTest(const FString& Parameters)
{
    const FTARearAxleRuntimeConfig Config =
        MakeRearAxleConfig();

    FTARearAxleSolveInput Input;
    Input.LeftRoad = MakeRearRoad();
    Input.RightRoad = MakeRearRoad();
    Input.RightRoad.PointWorldM.Z = 0.020;

    FTARearAxleRuntimeState State;
    FTARearAxleSolveOutput Output;

    TestTrue(
        TEXT("Rear split-height axle resolves"),
        TARearAxleRuntime::Resolve(
            MakeRearChassis(),
            Config,
            Input,
            0.327,
            0.327,
            1.0 / 240.0,
            State,
            Output));

    TestTrue(
        TEXT("Road split creates rear travel difference"),
        FMath::Abs(
            Output.LeftContact.TravelM
            - Output.RightContact.TravelM)
            > 0.005);

    TestTrue(
        TEXT("Rear anti-roll/contact creates load difference"),
        FMath::Abs(
            Output.LeftContact.VerticalLoadN
            - Output.RightContact.VerticalLoadN)
            > 100.0);

    return true;
}

#endif
