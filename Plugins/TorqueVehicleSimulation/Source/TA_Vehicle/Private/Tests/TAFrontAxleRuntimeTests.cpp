#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TAFrontAxleRuntime.h"

namespace
{
    FTAFrontAxleRuntimeConfig MakeFrontAxleConfig()
    {
        FTAFrontAxleRuntimeConfig Config;
        FTADoubleWishboneHardpoints& H =
            Config.RightGeometry.Hardpoints;

        H.UpperInnerA = FVector3d(1.48, 0.38, -0.22);
        H.UpperInnerB = FVector3d(1.14, 0.38, -0.22);

        H.LowerInnerA = FVector3d(1.50, 0.35, -0.48);
        H.LowerInnerB = FVector3d(1.12, 0.35, -0.48);

        H.TieRodInner = FVector3d(1.12, 0.35, -0.38);

        H.DamperChassis = FVector3d(1.31, 0.40, -0.05);
        H.DamperLowerArmReference = FVector3d(1.31, 0.52, -0.43);

        H.UpperBallJointReference = FVector3d(1.31, 0.70, -0.25);
        H.LowerBallJointReference = FVector3d(1.31, 0.73, -0.50);
        H.TieRodOuterReference = FVector3d(1.12, 0.71, -0.39);
        H.WheelCenterReference = FVector3d(1.31, 0.775, -0.45);

        H.WheelForwardReference = FVector3d(1.0, 0.0, 0.0);
        H.WheelUpReference = FVector3d(0.0, 0.0, 1.0);
        H.SteeringRackAxisLocal = FVector3d(0.0, 1.0, 0.0);
        H.SideSign = 1.0;

        Config.RightGeometry.MaxIterations = 80;
        Config.RightGeometry.PositionToleranceM = 0.0005;

        Config.LeftSuspension.SpringRateNPerM = 95000.0;
        Config.LeftSuspension.StaticSpringCompressionM = 0.08;
        Config.LeftSuspension.BumpDampingNsPerM = 4500.0;
        Config.LeftSuspension.ReboundDampingNsPerM = 6500.0;

        Config.RightSuspension =
            Config.LeftSuspension;

        Config.AntiRollBar.CouplingRateNPerM = 12000.0;
        Config.AntiRollBar.MaxTransferForceN = 3000.0;

        Config.SteeringRack.MaxRackDisplacementM = 0.035;
        Config.SteeringRack.InputExponent = 1.0;
        Config.SteeringRack.SteeringSign = -1.0;

        return Config;
    }

    FTARoadPlane MakeRoad()
    {
        FTARoadPlane Road;
        Road.PointWorldM = FVector3d::ZeroVector;
        Road.NormalWorld = FVector3d(0.0, 0.0, 1.0);
        Road.Surface.Material = ETASurfaceMaterial::FreshAsphalt;
        return Road;
    }

    FTAChassisState MakeChassis()
    {
        FTAChassisState Chassis;
        Chassis.PositionWorldM = FVector3d(0.0, 0.0, 0.777);
        return Chassis;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAFrontAxleMirrorTest,
    "TorqueAtlas.Suspension.FrontAxle.MirrorGeometry",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAFrontAxleMirrorTest::RunTest(const FString& Parameters)
{
    const FTAFrontAxleRuntimeConfig Axle =
        MakeFrontAxleConfig();

    const FTADoubleWishboneSolverConfig Left =
        TADoubleWishboneSolver::MirrorAcrossCenterline(
            Axle.RightGeometry);

    TestTrue(
        TEXT("Mirrored geometry validates"),
        TADoubleWishboneSolver::ValidateConfig(Left));

    TestTrue(
        TEXT("Wheel center Y mirrors"),
        FMath::IsNearlyEqual(
            Left.Hardpoints.WheelCenterReference.Y,
            -Axle.RightGeometry.Hardpoints.WheelCenterReference.Y,
            1.0e-9));

    TestTrue(
        TEXT("Rack translation axis remains shared rather than mirrored"),
        Left.Hardpoints.SteeringRackAxisLocal.Equals(
            Axle.RightGeometry.Hardpoints.SteeringRackAxisLocal,
            1.0e-9));

    TestTrue(
        TEXT("Side sign flips"),
        Left.Hardpoints.SideSign
            == -Axle.RightGeometry.Hardpoints.SideSign);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAFrontAxleRackClampTest,
    "TorqueAtlas.Suspension.FrontAxle.RackMapping",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAFrontAxleRackClampTest::RunTest(const FString& Parameters)
{
    FTASteeringRackRuntimeConfig Config;
    Config.MaxRackDisplacementM = 0.04;
    Config.InputExponent = 1.0;
    Config.SteeringSign = -1.0;

    TestTrue(
        TEXT("Positive full steering maps to configured signed rack travel"),
        FMath::IsNearlyEqual(
            TAFrontAxleRuntime::CalculateRackDisplacementM(
                Config,
                1.0),
            -0.04,
            1.0e-9));

    TestTrue(
        TEXT("Steering command clamps above one"),
        FMath::IsNearlyEqual(
            TAFrontAxleRuntime::CalculateRackDisplacementM(
                Config,
                2.0),
            -0.04,
            1.0e-9));

    TestTrue(
        TEXT("Negative full steering maps opposite"),
        FMath::IsNearlyEqual(
            TAFrontAxleRuntime::CalculateRackDisplacementM(
                Config,
                -1.0),
            0.04,
            1.0e-9));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAFrontAxleSteeringTest,
    "TorqueAtlas.Suspension.FrontAxle.SharedRackSteering",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAFrontAxleSteeringTest::RunTest(const FString& Parameters)
{
    const FTAFrontAxleRuntimeConfig Config =
        MakeFrontAxleConfig();

    FTAFrontAxleSolveInput Input;
    Input.Steering01 = 0.30;
    Input.LeftRoad = MakeRoad();
    Input.RightRoad = MakeRoad();

    FTAFrontAxleRuntimeState State;
    FTAFrontAxleSolveOutput Output;

    TestTrue(
        TEXT("Front axle solves"),
        TAFrontAxleRuntime::Resolve(
            MakeChassis(),
            Config,
            Input,
            0.327,
            0.327,
            1.0 / 240.0,
            State,
            Output));

    TestTrue(
        TEXT("Positive steering command produces right-turn wheel angles"),
        Output.LeftSteeringAngleRad > 0.0
        && Output.RightSteeringAngleRad > 0.0);

    TestTrue(
        TEXT("Inside right wheel steers more than outside left wheel"),
        Output.AckermannDeltaRad > 0.0);

    TestTrue(
        TEXT("Both front wheels remain in road contact"),
        Output.LeftContact.bInContact
        && Output.RightContact.bInContact);

    TestTrue(
        TEXT("Symmetric ride height has near-equal front loads"),
        FMath::Abs(
            Output.LeftContact.VerticalLoadN
            - Output.RightContact.VerticalLoadN) < 50.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAFrontAxleAsymmetricRoadTest,
    "TorqueAtlas.Suspension.FrontAxle.AsymmetricTravelAntiRoll",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAFrontAxleAsymmetricRoadTest::RunTest(const FString& Parameters)
{
    const FTAFrontAxleRuntimeConfig Config =
        MakeFrontAxleConfig();

    FTAFrontAxleSolveInput Input;
    Input.LeftRoad = MakeRoad();
    Input.RightRoad = MakeRoad();

    // 20 mm road rise under the right wheel.
    Input.RightRoad.PointWorldM.Z = 0.020;

    FTAFrontAxleRuntimeState State;
    FTAFrontAxleSolveOutput Output;

    TestTrue(
        TEXT("Asymmetric front axle solves"),
        TAFrontAxleRuntime::Resolve(
            MakeChassis(),
            Config,
            Input,
            0.327,
            0.327,
            1.0 / 240.0,
            State,
            Output));

    TestTrue(
        TEXT("Road-height split creates different suspension travel"),
        FMath::Abs(
            Output.LeftContact.TravelM
            - Output.RightContact.TravelM) > 0.005);

    TestTrue(
        TEXT("Anti-roll/contact solution creates different wheel loads"),
        FMath::Abs(
            Output.LeftContact.VerticalLoadN
            - Output.RightContact.VerticalLoadN) > 100.0);

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAFrontAxleCompliantAntiRollEquilibriumTest,
    "TorqueAtlas.Suspension.FrontAxle.CompliantAntiRollEquilibrium",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAFrontAxleCompliantAntiRollEquilibriumTest::RunTest(const FString& Parameters)
{
    FTAFrontAxleRuntimeConfig NoBarConfig =
        MakeFrontAxleConfig();

    NoBarConfig.AntiRollBar.CouplingRateNPerM = 0.0;
    NoBarConfig.AntiRollBar.MaxTransferForceN = 0.0;

    FTAFrontAxleRuntimeConfig BarConfig =
        MakeFrontAxleConfig();

    FTAFrontAxleSolveInput Input;
    Input.LeftRoad = MakeRoad();
    Input.RightRoad = MakeRoad();
    Input.RightRoad.PointWorldM.Z = 0.020;

    FTATireRuntimeConfig LeftTireConfig;
    FTATireRuntimeConfig RightTireConfig;

    FTAFrontAxleRuntimeState NoBarState;
    FTATireRuntimeState NoBarLeftTire;
    FTATireRuntimeState NoBarRightTire;
    FTAFrontAxleSolveOutput NoBarOutput;

    TestTrue(
        TEXT("Compliant front axle resolves without anti-roll"),
        TAFrontAxleRuntime::ResolveWithTireCompliance(
            MakeChassis(),
            NoBarConfig,
            Input,
            LeftTireConfig,
            RightTireConfig,
            1.0 / 240.0,
            NoBarState,
            NoBarLeftTire,
            NoBarRightTire,
            NoBarOutput));

    FTAFrontAxleRuntimeState BarState;
    FTATireRuntimeState BarLeftTire;
    FTATireRuntimeState BarRightTire;
    FTAFrontAxleSolveOutput BarOutput;

    TestTrue(
        TEXT("Compliant front axle resolves with coupled anti-roll"),
        TAFrontAxleRuntime::ResolveWithTireCompliance(
            MakeChassis(),
            BarConfig,
            Input,
            LeftTireConfig,
            RightTireConfig,
            1.0 / 240.0,
            BarState,
            BarLeftTire,
            BarRightTire,
            BarOutput));

    TestTrue(
        TEXT("Raised right road compresses right suspension more"),
        BarOutput.RightContact.TravelM
            > BarOutput.LeftContact.TravelM);

    TestTrue(
        TEXT("Coupled anti-roll raises inside/right normal load"),
        BarOutput.RightContact.VerticalLoadN
            > NoBarOutput.RightContact.VerticalLoadN);

    TestTrue(
        TEXT("Coupled anti-roll lowers opposite/left normal load"),
        BarOutput.LeftContact.VerticalLoadN
            < NoBarOutput.LeftContact.VerticalLoadN);

    TestTrue(
        TEXT("Higher anti-roll-supported load is represented by tire deflection"),
        BarOutput.RightContact.TireRadialDeflectionM
            > NoBarOutput.RightContact.TireRadialDeflectionM);

    TestTrue(
        TEXT("Lower opposite load is represented by reduced tire deflection"),
        BarOutput.LeftContact.TireRadialDeflectionM
            < NoBarOutput.LeftContact.TireRadialDeflectionM);

    return true;
}

#endif
