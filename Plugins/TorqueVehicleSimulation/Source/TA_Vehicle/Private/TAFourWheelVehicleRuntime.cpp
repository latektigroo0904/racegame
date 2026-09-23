#include "TAFourWheelVehicleRuntime.h"

namespace
{
    int32 ToIndex(const ETAPrototypeWheelIndex Wheel)
    {
        return static_cast<int32>(Wheel);
    }
}

bool TAFourWheelVehicleRuntime::Initialize(
    const FTAVehicleRuntimeConfig& VehicleConfig,
    FTAFourWheelRuntimeState& OutState)
{
    OutState = FTAFourWheelRuntimeState{};

    if (VehicleConfig.Wheels.Num() != 4 ||
        VehicleConfig.Tires.Num() != 4)
    {
        return false;
    }

    return TAVehicleSimulation::Initialize(
        VehicleConfig,
        OutState.Vehicle);
}

bool TAFourWheelVehicleRuntime::Step(
    const FTAVehicleRuntimeConfig& VehicleConfig,
    const FTAFourWheelRuntimeConfig& RuntimeConfig,
    const FTAFourWheelStepInput& Input,
    const double DeltaTimeSeconds,
    FTAFourWheelRuntimeState& InOutState,
    FTAFourWheelStepOutput& OutOutput)
{
    OutOutput = FTAFourWheelStepOutput{};

    if (DeltaTimeSeconds <= 0.0 ||
        VehicleConfig.Wheels.Num() != 4 ||
        VehicleConfig.Tires.Num() != 4 ||
        InOutState.Vehicle.Wheels.Num() != 4)
    {
        return false;
    }

    FTAFrontAxleSolveInput FrontInput;
    FrontInput.Steering01 =
        Input.Controls.Steering01;

    FrontInput.SteeringCommandAuthority01 =
        InOutState.Vehicle.SteeringRackDamage
            .CommandAuthority01;

    FrontInput.SteeringRackFreePlayM =
        InOutState.Vehicle.SteeringRackDamage
            .FreePlayM;

    FrontInput.LeftRoad =
        Input.FrontLeftRoad;

    FrontInput.RightRoad =
        Input.FrontRightRoad;

    FrontInput.LeftDamage =
        Input.FrontLeftDamage;

    FrontInput.RightDamage =
        Input.FrontRightDamage;

    const int32 FrontLeftIndex =
        ToIndex(ETAPrototypeWheelIndex::FrontLeft);

    const int32 FrontRightIndex =
        ToIndex(ETAPrototypeWheelIndex::FrontRight);

    if (!TAFrontAxleRuntime::ResolveWithTireCompliance(
            InOutState.Vehicle.Chassis,
            RuntimeConfig.FrontAxle,
            FrontInput,
            VehicleConfig.Tires[FrontLeftIndex],
            VehicleConfig.Tires[FrontRightIndex],
            DeltaTimeSeconds,
            InOutState.FrontAxle,
            InOutState.Vehicle.Wheels[FrontLeftIndex].TireState,
            InOutState.Vehicle.Wheels[FrontRightIndex].TireState,
            OutOutput.FrontAxle))
    {
        return false;
    }

    FTARearAxleSolveInput RearInput;

    RearInput.LeftRoad =
        Input.RearLeftRoad;

    RearInput.RightRoad =
        Input.RearRightRoad;

    RearInput.LeftDamage =
        Input.RearLeftDamage;

    RearInput.RightDamage =
        Input.RearRightDamage;

    const int32 RearLeftIndex =
        ToIndex(ETAPrototypeWheelIndex::RearLeft);

    const int32 RearRightIndex =
        ToIndex(ETAPrototypeWheelIndex::RearRight);

    if (!TARearAxleRuntime::ResolveWithTireCompliance(
            InOutState.Vehicle.Chassis,
            RuntimeConfig.RearAxle,
            RearInput,
            VehicleConfig.Tires[RearLeftIndex],
            VehicleConfig.Tires[RearRightIndex],
            DeltaTimeSeconds,
            InOutState.RearAxle,
            InOutState.Vehicle.Wheels[RearLeftIndex].TireState,
            InOutState.Vehicle.Wheels[RearRightIndex].TireState,
            OutOutput.RearAxle))
    {
        return false;
    }

    FTAVehicleStepInput VehicleInput;
    VehicleInput.Controls =
        Input.Controls;

    VehicleInput.WheelContacts.SetNum(4);

    VehicleInput.WheelContacts[
        ToIndex(ETAPrototypeWheelIndex::FrontLeft)] =
        OutOutput.FrontAxle.LeftVehicleContact;

    VehicleInput.WheelContacts[
        ToIndex(ETAPrototypeWheelIndex::FrontRight)] =
        OutOutput.FrontAxle.RightVehicleContact;

    VehicleInput.WheelContacts[
        ToIndex(ETAPrototypeWheelIndex::RearLeft)] =
        OutOutput.RearAxle.LeftVehicleContact;

    VehicleInput.WheelContacts[
        ToIndex(ETAPrototypeWheelIndex::RearRight)] =
        OutOutput.RearAxle.RightVehicleContact;

    OutOutput.bContactsSolved = true;

    return TAVehicleSimulation::Step(
        VehicleConfig,
        VehicleInput,
        DeltaTimeSeconds,
        InOutState.Vehicle,
        OutOutput.Vehicle);
}
