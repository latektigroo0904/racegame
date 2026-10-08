#include "TABrakeControlStack.h"

bool TABrakeControlStack::ValidateConfig(
    const FTABrakeControlStackConfig& Config)
{
    return
        TABrakeHydraulics::ValidateConfig(
            Config.Hydraulics)
        && TAAbsController::ValidateConfig(
            Config.Abs);
}

void TABrakeControlStack::InitializeState(
    const FTABrakeControlStackConfig& Config,
    FTABrakeControlStackState& OutState)
{
    OutState =
        FTABrakeControlStackState{};

    TABrakeHydraulics::InitializeState(
        Config.Hydraulics,
        OutState.Hydraulics);

    for (int32 CornerIndex = 0;
         CornerIndex < TABrakeCornerCount;
         ++CornerIndex)
    {
        TAAbsController::InitializeState(
            OutState.Abs[CornerIndex]);
    }
}

bool TABrakeControlStack::Step(
    const FTABrakeControlStackConfig& Config,
    const FTABrakeControlStackInput& Input,
    FTABrakeControlStackState& InOutState,
    FTABrakeControlStackOutput& OutOutput)
{
    OutOutput =
        FTABrakeControlStackOutput{};

    if (!ValidateConfig(Config)
        || !FMath::IsFinite(Input.BrakePedal01)
        || !FMath::IsFinite(Input.VehicleSpeedMps)
        || !FMath::IsFinite(Input.DeltaTimeSeconds)
        || Input.DeltaTimeSeconds <= 0.0)
    {
        return false;
    }

    FTABrakeHydraulicInput HydraulicInput;
    HydraulicInput.Pedal01 =
        FMath::Clamp(
            Input.BrakePedal01,
            0.0,
            1.0);

    HydraulicInput.DeltaTimeSeconds =
        Input.DeltaTimeSeconds;

    for (int32 CornerIndex = 0;
         CornerIndex < TABrakeCornerCount;
         ++CornerIndex)
    {
        if (!FMath::IsFinite(Input.SlipRatio[CornerIndex])
            || !FMath::IsFinite(
                Input.WheelAngularDecelerationRadPerSec2[CornerIndex]))
        {
            return false;
        }

        FTAAbsInput AbsInput;
        AbsInput.BrakeRequest01 =
            HydraulicInput.Pedal01;

        AbsInput.VehicleSpeedMps =
            Input.VehicleSpeedMps;

        AbsInput.SlipRatio =
            Input.SlipRatio[CornerIndex];

        AbsInput.WheelAngularDecelerationRadPerSec2 =
            Input.WheelAngularDecelerationRadPerSec2[CornerIndex];

        AbsInput.bContactValid =
            Input.bContactValid[CornerIndex];

        AbsInput.DeltaTimeSeconds =
            Input.DeltaTimeSeconds;

        if (!TAAbsController::Step(
                Config.Abs,
                AbsInput,
                InOutState.Abs[CornerIndex],
                OutOutput.Abs[CornerIndex]))
        {
            return false;
        }

        HydraulicInput.CornerPressureModulation01[CornerIndex] =
            OutOutput.Abs[CornerIndex].PressureModulation01;
    }

    return TABrakeHydraulics::Step(
        Config.Hydraulics,
        HydraulicInput,
        InOutState.Hydraulics,
        OutOutput.Hydraulics);
}
