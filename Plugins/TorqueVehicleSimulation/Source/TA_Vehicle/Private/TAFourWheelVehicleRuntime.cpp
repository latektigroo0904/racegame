#include "TAFourWheelVehicleRuntime.h"

namespace
{
    int32 ToIndex(const ETAPrototypeWheelIndex Wheel)
    {
        return static_cast<int32>(Wheel);
    }

    FTASuspensionRuntimeConfig ApplySuspensionFunctionalDamage(
        const FTASuspensionRuntimeConfig& Base,
        const FTASuspensionFunctionalDamageState& Damage)
    {
        FTASuspensionRuntimeConfig Effective =
            Base;

        const double SpringEfficiency01 =
            FMath::Clamp(
                Damage.SpringEfficiency01,
                0.0,
                1.0);

        const double DampingEfficiency01 =
            FMath::Clamp(
                Damage.DampingEfficiency01,
                0.0,
                1.0);

        const double StopEfficiency01 =
            FMath::Clamp(
                Damage.StopEfficiency01,
                0.0,
                1.0);

        Effective.SpringRateNPerM *=
            SpringEfficiency01;

        Effective.BumpDampingNsPerM *=
            DampingEfficiency01;

        Effective.ReboundDampingNsPerM *=
            DampingEfficiency01;

        Effective.BumpStopRateNPerM *=
            StopEfficiency01;

        Effective.DroopStopRateNPerM *=
            StopEfficiency01;

        return Effective;
    }

    double CalculateAxleAntiRollEfficiency01(
        const FTASuspensionFunctionalDamageState& LeftDamage,
        const FTASuspensionFunctionalDamageState& RightDamage)
    {
        return FMath::Clamp(
            FMath::Min(
                LeftDamage.AntiRollLinkEfficiency01,
                RightDamage.AntiRollLinkEfficiency01),
            0.0,
            1.0);
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
        InOutState.Vehicle.Wheels.Num() != 4 ||
        InOutState.Vehicle.SuspensionDamage.Num() != 4)
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

    FTAFrontAxleRuntimeConfig EffectiveFrontConfig =
        RuntimeConfig.FrontAxle;

    EffectiveFrontConfig.LeftSuspension =
        ApplySuspensionFunctionalDamage(
            RuntimeConfig.FrontAxle.LeftSuspension,
            InOutState.Vehicle.SuspensionDamage[
                FrontLeftIndex]);

    EffectiveFrontConfig.RightSuspension =
        ApplySuspensionFunctionalDamage(
            RuntimeConfig.FrontAxle.RightSuspension,
            InOutState.Vehicle.SuspensionDamage[
                FrontRightIndex]);

    const double FrontAntiRollEfficiency01 =
        CalculateAxleAntiRollEfficiency01(
            InOutState.Vehicle.SuspensionDamage[
                FrontLeftIndex],
            InOutState.Vehicle.SuspensionDamage[
                FrontRightIndex]);

    EffectiveFrontConfig.AntiRollBar.CouplingRateNPerM *=
        FrontAntiRollEfficiency01;

    EffectiveFrontConfig.AntiRollBar.MaxTransferForceN *=
        FrontAntiRollEfficiency01;

    if (!TAFrontAxleRuntime::ResolveWithTireCompliance(
            InOutState.Vehicle.Chassis,
            EffectiveFrontConfig,
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

    FTARearAxleRuntimeConfig EffectiveRearConfig =
        RuntimeConfig.RearAxle;

    EffectiveRearConfig.LeftSuspension =
        ApplySuspensionFunctionalDamage(
            RuntimeConfig.RearAxle.LeftSuspension,
            InOutState.Vehicle.SuspensionDamage[
                RearLeftIndex]);

    EffectiveRearConfig.RightSuspension =
        ApplySuspensionFunctionalDamage(
            RuntimeConfig.RearAxle.RightSuspension,
            InOutState.Vehicle.SuspensionDamage[
                RearRightIndex]);

    const double RearAntiRollEfficiency01 =
        CalculateAxleAntiRollEfficiency01(
            InOutState.Vehicle.SuspensionDamage[
                RearLeftIndex],
            InOutState.Vehicle.SuspensionDamage[
                RearRightIndex]);

    EffectiveRearConfig.AntiRollBar.CouplingRateNPerM *=
        RearAntiRollEfficiency01;

    EffectiveRearConfig.AntiRollBar.MaxTransferForceN *=
        RearAntiRollEfficiency01;

    if (!TARearAxleRuntime::ResolveWithTireCompliance(
            InOutState.Vehicle.Chassis,
            EffectiveRearConfig,
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
