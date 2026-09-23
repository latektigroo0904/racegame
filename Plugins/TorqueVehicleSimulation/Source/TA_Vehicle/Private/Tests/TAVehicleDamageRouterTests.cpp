#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "TAVehicleDamageRouter.h"

namespace
{
    FTAVehicleDamageRoutingConfig MakeRadiatorRoutingConfig()
    {
        FTAVehicleDamageRoutingConfig Config;

        FTAVehicleDamageRoute Route;
        Route.TargetComponentIndex = 7;
        Route.Consumer = ETAVehicleDamageConsumerType::Radiator;
        Route.ImpactEnergyScale = 1.0;
        Route.FullCrushDisplacementM = 0.10;

        Config.Routes.Add(Route);

        return Config;
    }

    FTAVehicleRuntimeConfig MakeFunctionalDamageVehicleConfig()
    {
        FTAVehicleRuntimeConfig Config;
        Config.Wheels.SetNum(4);
        Config.Tires.SetNum(4);

        for (int32 Index = 0; Index < 4; ++Index)
        {
            Config.Wheels[Index].RadiusM =
                Config.Tires[Index].UnloadedRadiusM;
        }

        Config.Wheels[2].bDriven = true;
        Config.Wheels[3].bDriven = true;
        return Config;
    }

    FTAVehicleRuntimeState MakeVehicleState(
        const FTAVehicleRuntimeConfig& Config)
    {
        FTAVehicleRuntimeState State;

        if (!TAVehicleSimulation::Initialize(
                Config,
                State))
        {
            TADamage::InitializeRadiatorState(
                Config.Radiator,
                State.Radiator);

            State.WheelHubDamage.SetNum(
                Config.Wheels.Num());
        }

        return State;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDamageRouterRadiatorImpactTest,
    "TorqueAtlas.Damage.VehicleRouter.RadiatorImpact",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDamageRouterRadiatorImpactTest::RunTest(
    const FString& Parameters)
{
    FTAVehicleRuntimeConfig VehicleConfig;
    FTAVehicleRuntimeState State =
        MakeVehicleState(VehicleConfig);

    FTADamageSignal Signal;
    Signal.TargetComponentIndex = 7;
    Signal.Type = ETADamageSignalType::ImpactEnergy;
    Signal.ScalarValue = 12000.0;

    FTAVehicleDamageRoutingOutput Output;

    TestTrue(
        TEXT("Impact route succeeds"),
        TAVehicleDamageRouter::RouteSignals(
            MakeRadiatorRoutingConfig(),
            VehicleConfig,
            MakeArrayView(&Signal, 1),
            State,
            Output));

    TestTrue(
        TEXT("Routed impact punctures radiator"),
        State.Radiator.bPunctured);

    TestTrue(
        TEXT("Routed impact creates leak area"),
        State.Radiator.LeakAreaMm2 > 0.0);

    TestEqual(
        TEXT("One radiator signal is applied"),
        Output.RadiatorSignalsApplied,
        1);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDamageRouterRadiatorDisplacementTest,
    "TorqueAtlas.Damage.VehicleRouter.RadiatorSupportDisplacement",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDamageRouterRadiatorDisplacementTest::RunTest(
    const FString& Parameters)
{
    FTAVehicleRuntimeConfig VehicleConfig;
    FTAVehicleRuntimeState State =
        MakeVehicleState(VehicleConfig);

    FTADamageSignal Signal;
    Signal.TargetComponentIndex = 7;
    Signal.Type = ETADamageSignalType::StructuralDisplacement;
    Signal.ScalarValue = 0.05;
    Signal.VectorValue = FVector3d(0.05, 0.0, 0.0);

    FTAVehicleDamageRoutingOutput Output;

    TestTrue(
        TEXT("Displacement route succeeds"),
        TAVehicleDamageRouter::RouteSignals(
            MakeRadiatorRoutingConfig(),
            VehicleConfig,
            MakeArrayView(&Signal, 1),
            State,
            Output));

    TestTrue(
        TEXT("Support displacement reduces radiator airflow"),
        State.Radiator.AirflowEfficiency01 < 1.0);

    TestFalse(
        TEXT("Support displacement without impact energy need not puncture"),
        State.Radiator.bPunctured);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDamageRouterUnknownTargetTest,
    "TorqueAtlas.Damage.VehicleRouter.UnknownTargetIgnored",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDamageRouterUnknownTargetTest::RunTest(
    const FString& Parameters)
{
    FTAVehicleRuntimeConfig VehicleConfig;
    FTAVehicleRuntimeState State =
        MakeVehicleState(VehicleConfig);

    FTADamageSignal Signal;
    Signal.TargetComponentIndex = 999;
    Signal.Type = ETADamageSignalType::ImpactEnergy;
    Signal.ScalarValue = 50000.0;

    FTAVehicleDamageRoutingOutput Output;

    TestTrue(
        TEXT("Unknown target does not make routing fail"),
        TAVehicleDamageRouter::RouteSignals(
            MakeRadiatorRoutingConfig(),
            VehicleConfig,
            MakeArrayView(&Signal, 1),
            State,
            Output));

    TestEqual(
        TEXT("Unknown target is counted as ignored"),
        Output.SignalsIgnored,
        1);

    TestFalse(
        TEXT("Unknown target does not damage radiator"),
        State.Radiator.bPunctured);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDamageRouterDuplicateTargetValidationTest,
    "TorqueAtlas.Damage.VehicleRouter.RejectsDuplicateTarget",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDamageRouterDuplicateTargetValidationTest::RunTest(
    const FString& Parameters)
{
    FTAVehicleDamageRoutingConfig Config =
        MakeRadiatorRoutingConfig();

    Config.Routes.Add(
        Config.Routes[0]);

    TestFalse(
        TEXT("Duplicate target routes are rejected"),
        TAVehicleDamageRouter::ValidateConfig(
            Config));

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDamageRouterSteeringRackTest,
    "TorqueAtlas.Damage.VehicleRouter.SteeringRackFunctionalDamage",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDamageRouterSteeringRackTest::RunTest(
    const FString& Parameters)
{
    const FTAVehicleRuntimeConfig VehicleConfig =
        MakeFunctionalDamageVehicleConfig();

    FTAVehicleRuntimeState State =
        MakeVehicleState(VehicleConfig);

    FTAVehicleDamageRoutingConfig Routing;

    FTAVehicleDamageRoute Route;
    Route.TargetComponentIndex = 8;
    Route.Consumer =
        ETAVehicleDamageConsumerType::SteeringRack;
    Route.FullDamageEnergyJ = 10000.0;
    Route.MinimumSteeringAuthority01 = 0.25;
    Route.MaximumSteeringFreePlayM = 0.012;

    Routing.Routes.Add(Route);

    FTADamageSignal Impact;
    Impact.TargetComponentIndex = 8;
    Impact.Type = ETADamageSignalType::ImpactEnergy;
    Impact.ScalarValue = 5000.0;

    FTAVehicleDamageRoutingOutput Output;

    TestTrue(
        TEXT("Steering-rack damage route succeeds"),
        TAVehicleDamageRouter::RouteSignals(
            Routing,
            VehicleConfig,
            MakeArrayView(&Impact, 1),
            State,
            Output));

    TestTrue(
        TEXT("Half-energy impact produces half severity"),
        FMath::IsNearlyEqual(
            State.SteeringRackDamage.Damage01,
            0.5,
            1.0e-9));

    TestTrue(
        TEXT("Steering authority degrades monotonically"),
        FMath::IsNearlyEqual(
            State.SteeringRackDamage.CommandAuthority01,
            0.625,
            1.0e-9));

    TestTrue(
        TEXT("Rack free-play grows with severity"),
        FMath::IsNearlyEqual(
            State.SteeringRackDamage.FreePlayM,
            0.006,
            1.0e-9));

    TestEqual(
        TEXT("Steering route count increments"),
        Output.SteeringRackSignalsApplied,
        1);

    FTADamageSignal SmallerImpact =
        Impact;

    SmallerImpact.ScalarValue =
        1000.0;

    TestTrue(
        TEXT("Smaller later hit routes"),
        TAVehicleDamageRouter::RouteSignals(
            Routing,
            VehicleConfig,
            MakeArrayView(&SmallerImpact, 1),
            State,
            Output));

    TestTrue(
        TEXT("Smaller hit does not heal steering authority"),
        FMath::IsNearlyEqual(
            State.SteeringRackDamage.CommandAuthority01,
            0.625,
            1.0e-9));

    TestTrue(
        TEXT("Smaller hit does not reduce accumulated free-play"),
        FMath::IsNearlyEqual(
            State.SteeringRackDamage.FreePlayM,
            0.006,
            1.0e-9));

    FTADamageSignal Fracture =
        Impact;

    Fracture.Type =
        ETADamageSignalType::StructuralFracture;

    Route.bAcceptStructuralFracture =
        true;

    Routing.Routes[0] =
        Route;

    TestTrue(
        TEXT("Steering-support fracture routes"),
        TAVehicleDamageRouter::RouteSignals(
            Routing,
            VehicleConfig,
            MakeArrayView(&Fracture, 1),
            State,
            Output));

    TestTrue(
        TEXT("Fracture reaches full steering damage"),
        FMath::IsNearlyEqual(
            State.SteeringRackDamage.Damage01,
            1.0,
            1.0e-9));

    TestTrue(
        TEXT("Fracture reaches configured minimum authority"),
        FMath::IsNearlyEqual(
            State.SteeringRackDamage.CommandAuthority01,
            0.25,
            1.0e-9));

    TestTrue(
        TEXT("Fracture reaches configured maximum free-play"),
        FMath::IsNearlyEqual(
            State.SteeringRackDamage.FreePlayM,
            0.012,
            1.0e-9));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDamageRouterWheelHubTest,
    "TorqueAtlas.Damage.VehicleRouter.WheelHubFunctionalDamage",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDamageRouterWheelHubTest::RunTest(
    const FString& Parameters)
{
    const FTAVehicleRuntimeConfig VehicleConfig =
        MakeFunctionalDamageVehicleConfig();

    FTAVehicleRuntimeState State =
        MakeVehicleState(VehicleConfig);

    FTAVehicleDamageRoutingConfig Routing;

    FTAVehicleDamageRoute Route;
    Route.TargetComponentIndex = 9;
    Route.Consumer =
        ETAVehicleDamageConsumerType::WheelHub;
    Route.WheelIndex = 3;
    Route.FullDamageEnergyJ = 10000.0;
    Route.MinimumBrakeEfficiency01 = 0.20;
    Route.MinimumDriveEfficiency01 = 0.0;
    Route.MaximumBearingDragTorqueNm = 80.0;

    Routing.Routes.Add(Route);

    FTADamageSignal Impact;
    Impact.TargetComponentIndex = 9;
    Impact.Type = ETADamageSignalType::ImpactEnergy;
    Impact.ScalarValue = 5000.0;

    FTAVehicleDamageRoutingOutput Output;

    TestTrue(
        TEXT("Wheel-hub damage route succeeds"),
        TAVehicleDamageRouter::RouteSignals(
            Routing,
            VehicleConfig,
            MakeArrayView(&Impact, 1),
            State,
            Output));

    const FTAWheelHubFunctionalDamageState& Hub =
        State.WheelHubDamage[3];

    TestTrue(
        TEXT("Half-energy impact produces half hub severity"),
        FMath::IsNearlyEqual(
            Hub.Damage01,
            0.5,
            1.0e-9));

    TestTrue(
        TEXT("Hub brake efficiency degrades"),
        FMath::IsNearlyEqual(
            Hub.BrakeEfficiency01,
            0.6,
            1.0e-9));

    TestTrue(
        TEXT("Hub drive efficiency degrades"),
        FMath::IsNearlyEqual(
            Hub.DriveEfficiency01,
            0.5,
            1.0e-9));

    TestTrue(
        TEXT("Hub bearing drag grows"),
        FMath::IsNearlyEqual(
            Hub.BearingDragTorqueNm,
            40.0,
            1.0e-9));

    TestEqual(
        TEXT("Hub route count increments"),
        Output.WheelHubSignalsApplied,
        1);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTAVehicleDamageRouterInvalidHubRouteTest,
    "TorqueAtlas.Damage.VehicleRouter.RejectsInvalidHubRoute",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTAVehicleDamageRouterInvalidHubRouteTest::RunTest(
    const FString& Parameters)
{
    FTAVehicleDamageRoutingConfig Routing;

    FTAVehicleDamageRoute Route;
    Route.TargetComponentIndex = 9;
    Route.Consumer =
        ETAVehicleDamageConsumerType::WheelHub;
    Route.WheelIndex = INDEX_NONE;

    Routing.Routes.Add(Route);

    TestFalse(
        TEXT("Wheel-hub route requires a wheel index"),
        TAVehicleDamageRouter::ValidateConfig(
            Routing));

    return true;
}

#endif
