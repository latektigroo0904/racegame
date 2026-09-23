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

    FTAVehicleRuntimeState MakeVehicleState(
        const FTAVehicleRuntimeConfig& Config)
    {
        FTAVehicleRuntimeState State;
        TADamage::InitializeRadiatorState(
            Config.Radiator,
            State.Radiator);
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

#endif
