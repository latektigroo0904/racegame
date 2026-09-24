#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TAStaticMassBalance.h"

namespace
{
    FTAStaticMassBalanceInput MakeBaseline()
    {
        FTAStaticMassBalanceInput Input;
        Input.MassKg = 1420.0;
        Input.GravityMagnitudeMps2 = 9.80665;
        Input.WheelbaseM = 2.62;
        Input.FrontTrackM = 1.55;
        Input.RearTrackM = 1.53;
        Input.ComFromRearAxleM = 0.55 * Input.WheelbaseM;
        Input.ComLateralM = 0.0;
        return Input;
    }

    constexpr double ForceToleranceN = 1.0e-6;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAStaticMassBalanceConservationTest,
    "TorqueAtlas.Vehicle.StaticMassBalance.ConservationAndSymmetry",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAStaticMassBalanceConservationTest::RunTest(const FString& Parameters)
{
    const FTAStaticMassBalanceInput Input = MakeBaseline();
    FTAStaticMassBalanceOutput Output;

    TestTrue(TEXT("Baseline calculates"), TAStaticMassBalance::Calculate(Input, Output));

    const double CornerSum =
        Output.FrontLeftLoadN + Output.FrontRightLoadN
        + Output.RearLeftLoadN + Output.RearRightLoadN;

    TestTrue(TEXT("Corner loads conserve total weight"),
        FMath::IsNearlyEqual(CornerSum, Output.TotalWeightN, ForceToleranceN));
    TestTrue(TEXT("Front corners conserve front axle"),
        FMath::IsNearlyEqual(Output.FrontLeftLoadN + Output.FrontRightLoadN,
            Output.FrontAxleLoadN, ForceToleranceN));
    TestTrue(TEXT("Rear corners conserve rear axle"),
        FMath::IsNearlyEqual(Output.RearLeftLoadN + Output.RearRightLoadN,
            Output.RearAxleLoadN, ForceToleranceN));
    TestTrue(TEXT("Centered front is symmetric"),
        FMath::IsNearlyEqual(Output.FrontLeftLoadN, Output.FrontRightLoadN, ForceToleranceN));
    TestTrue(TEXT("Centered rear is symmetric"),
        FMath::IsNearlyEqual(Output.RearLeftLoadN, Output.RearRightLoadN, ForceToleranceN));
    TestTrue(TEXT("Baseline front fraction is 55 percent"),
        FMath::IsNearlyEqual(Output.FrontAxleLoadN / Output.TotalWeightN, 0.55, 1.0e-12));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAStaticMassBalanceMonotonicityTest,
    "TorqueAtlas.Vehicle.StaticMassBalance.MonotonicityAndLateralSign",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAStaticMassBalanceMonotonicityTest::RunTest(const FString& Parameters)
{
    FTAStaticMassBalanceInput Rearward = MakeBaseline();
    Rearward.ComFromRearAxleM = 0.45 * Rearward.WheelbaseM;

    FTAStaticMassBalanceInput Forward = Rearward;
    Forward.ComFromRearAxleM = 0.65 * Forward.WheelbaseM;

    FTAStaticMassBalanceOutput RearwardOutput;
    FTAStaticMassBalanceOutput ForwardOutput;
    TestTrue(TEXT("Rearward case calculates"), TAStaticMassBalance::Calculate(Rearward, RearwardOutput));
    TestTrue(TEXT("Forward case calculates"), TAStaticMassBalance::Calculate(Forward, ForwardOutput));
    TestTrue(TEXT("Forward COM increases front axle load"),
        ForwardOutput.FrontAxleLoadN > RearwardOutput.FrontAxleLoadN);

    FTAStaticMassBalanceInput RightShift = MakeBaseline();
    RightShift.ComLateralM = 0.10;
    FTAStaticMassBalanceOutput RightOutput;
    TestTrue(TEXT("Right-shift case calculates"), TAStaticMassBalance::Calculate(RightShift, RightOutput));
    TestTrue(TEXT("Positive Y increases front-right load"),
        RightOutput.FrontRightLoadN > RightOutput.FrontLeftLoadN);
    TestTrue(TEXT("Positive Y increases rear-right load"),
        RightOutput.RearRightLoadN > RightOutput.RearLeftLoadN);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAStaticMassBalanceValidationTest,
    "TorqueAtlas.Vehicle.StaticMassBalance.Validation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAStaticMassBalanceValidationTest::RunTest(const FString& Parameters)
{
    FTAStaticMassBalanceOutput Output;

    FTAStaticMassBalanceInput Invalid = MakeBaseline();
    Invalid.MassKg = 0.0;
    TestFalse(TEXT("Zero mass rejected"), TAStaticMassBalance::Calculate(Invalid, Output));

    Invalid = MakeBaseline();
    Invalid.WheelbaseM = 0.0;
    TestFalse(TEXT("Zero wheelbase rejected"), TAStaticMassBalance::Calculate(Invalid, Output));

    Invalid = MakeBaseline();
    Invalid.ComFromRearAxleM = Invalid.WheelbaseM + 0.001;
    TestFalse(TEXT("COM beyond front axle rejected"), TAStaticMassBalance::Calculate(Invalid, Output));

    Invalid = MakeBaseline();
    Invalid.ComLateralM = 0.5 * Invalid.RearTrackM;
    TestFalse(TEXT("COM on lateral support boundary rejected"), TAStaticMassBalance::Calculate(Invalid, Output));

    Invalid = MakeBaseline();
    Invalid.GravityMagnitudeMps2 = TNumericLimits<double>::QuietNaN();
    TestFalse(TEXT("Non-finite gravity rejected"), TAStaticMassBalance::Calculate(Invalid, Output));

    TestTrue(TEXT("Failure leaves zero output"),
        Output.TotalWeightN == 0.0
        && Output.FrontLeftLoadN == 0.0
        && Output.FrontRightLoadN == 0.0
        && Output.RearLeftLoadN == 0.0
        && Output.RearRightLoadN == 0.0);

    return true;
}

#endif
