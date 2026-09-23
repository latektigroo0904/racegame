#include "TAVehicleSimulation.h"

namespace
{
    bool FindTwoDrivenWheels(
        const FTAVehicleRuntimeConfig& Config,
        int32& OutLeftIndex,
        int32& OutRightIndex)
    {
        OutLeftIndex = INDEX_NONE;
        OutRightIndex = INDEX_NONE;

        for (int32 Index = 0; Index < Config.Wheels.Num(); ++Index)
        {
            if (!Config.Wheels[Index].bDriven)
            {
                continue;
            }

            if (OutLeftIndex == INDEX_NONE)
            {
                OutLeftIndex = Index;
            }
            else if (OutRightIndex == INDEX_NONE)
            {
                OutRightIndex = Index;
            }
            else
            {
                return false;
            }
        }

        return OutLeftIndex != INDEX_NONE && OutRightIndex != INDEX_NONE;
    }

    FTATireSolveInput BuildTireInput(
        const FTAWheelContactInput& Contact,
        const double WheelAngularSpeedRadPerSec)
    {
        FTATireSolveInput Input;
        Input.VerticalLoadN = Contact.VerticalLoadN;
        Input.LongitudinalVelocityMps = Contact.LongitudinalVelocityMps;
        Input.LateralVelocityMps = Contact.LateralVelocityMps;
        Input.WheelAngularSpeedRadPerSec = WheelAngularSpeedRadPerSec;
        Input.CamberRad = Contact.CamberRad;
        Input.Surface = Contact.Surface;
        return Input;
    }
}

bool TAVehicleSimulation::Initialize(
    const FTAVehicleRuntimeConfig& Config,
    FTAVehicleRuntimeState& OutState)
{
    if (Config.Wheels.Num() < 2 || Config.Wheels.Num() != Config.Tires.Num())
    {
        return false;
    }

    int32 DrivenA = INDEX_NONE;
    int32 DrivenB = INDEX_NONE;
    if (!FindTwoDrivenWheels(Config, DrivenA, DrivenB))
    {
        return false;
    }

    OutState = FTAVehicleRuntimeState{};
    OutState.Wheels.SetNum(Config.Wheels.Num());
    OutState.WheelHubDamage.SetNum(Config.Wheels.Num());
    OutState.SuspensionDamage.SetNum(Config.Wheels.Num());

    const double IdleRadPerSec =
        FMath::Max(0.0, Config.Engine.IdleRPM)
        * (2.0 * UE_DOUBLE_PI)
        / 60.0;

    OutState.Engine.AngularSpeedRadPerSec = IdleRadPerSec;
    OutState.Engine.RunState = ETAEngineRunState::Running;

    TAPowertrainSolver::InitializeEngineThermalState(
        Config.EngineThermal,
        OutState.EngineThermal);

    TADamage::InitializeRadiatorState(
        Config.Radiator,
        OutState.Radiator);

    OutState.Clutch.TemperatureC = Config.Clutch.AmbientTemperatureC;
    OutState.Clutch.ThermalCapacityFactor = 1.0;
    OutState.Clutch.WearCapacityFactor = 1.0;

    for (int32 Index = 0; Index < OutState.Wheels.Num(); ++Index)
    {
        OutState.Wheels[Index].TireState.PressureKPa =
            Config.Tires[Index].ReferencePressureKPa;

        OutState.Wheels[Index].TireState.TreadDepthMm =
            Config.Tires[Index].NewTreadDepthMm;
    }

    return true;
}

double TAVehicleSimulation::IntegrateWheelAngularSpeed(
    const FTAWheelRuntimeConfig& Config,
    const double CurrentAngularSpeedRadPerSec,
    const double DriveTorqueNm,
    const double TireReactionTorqueNm,
    const double BrakeInput01,
    const double DeltaTimeSeconds)
{
    if (Config.InertiaKgm2 <= UE_DOUBLE_SMALL_NUMBER || DeltaTimeSeconds <= 0.0)
    {
        return CurrentAngularSpeedRadPerSec;
    }

    const double NonBrakeTorqueNm =
        DriveTorqueNm + TireReactionTorqueNm;

    double PredictedAngularSpeed =
        CurrentAngularSpeedRadPerSec
        + (NonBrakeTorqueNm / Config.InertiaKgm2) * DeltaTimeSeconds;

    const double BrakeTorqueNm =
        FMath::Clamp(BrakeInput01, 0.0, 1.0)
        * FMath::Max(0.0, Config.MaxBrakeTorqueNm);

    const double BrakeDeltaSpeed =
        (BrakeTorqueNm / Config.InertiaKgm2) * DeltaTimeSeconds;

    if (PredictedAngularSpeed > 0.0)
    {
        PredictedAngularSpeed =
            FMath::Max(0.0, PredictedAngularSpeed - BrakeDeltaSpeed);
    }
    else if (PredictedAngularSpeed < 0.0)
    {
        PredictedAngularSpeed =
            FMath::Min(0.0, PredictedAngularSpeed + BrakeDeltaSpeed);
    }

    return PredictedAngularSpeed;
}

bool TAVehicleSimulation::Step(
    const FTAVehicleRuntimeConfig& Config,
    const FTAVehicleStepInput& Input,
    const double DeltaTimeSeconds,
    FTAVehicleRuntimeState& InOutState,
    FTAVehicleStepOutput& OutOutput)
{
    OutOutput = FTAVehicleStepOutput{};

    if (DeltaTimeSeconds <= 0.0 ||
        Config.Wheels.Num() != Config.Tires.Num() ||
        InOutState.Wheels.Num() != Config.Wheels.Num() ||
        InOutState.WheelHubDamage.Num() != Config.Wheels.Num() ||
        InOutState.SuspensionDamage.Num() != Config.Wheels.Num() ||
        Input.WheelContacts.Num() != Config.Wheels.Num())
    {
        return false;
    }

    int32 DrivenLeftIndex = INDEX_NONE;
    int32 DrivenRightIndex = INDEX_NONE;
    if (!FindTwoDrivenWheels(Config, DrivenLeftIndex, DrivenRightIndex))
    {
        return false;
    }

    FTAChassisForceAccumulator ChassisForces;

    const double Throttle01 =
        FMath::Clamp(Input.Controls.Throttle01, 0.0, 1.0);

    const double Brake01 =
        FMath::Clamp(Input.Controls.Brake01, 0.0, 1.0);

    InOutState.Clutch.Engagement =
        FMath::Clamp(Input.Controls.ClutchEngagement01, 0.0, 1.0);

    InOutState.SelectedGear = Input.Controls.SelectedGear;

    const double GearRatio =
        TAPowertrainSolver::GetSelectedGearRatio(
            Config.Gearbox,
            InOutState.SelectedGear);

    const double LeftWheelSpeed =
        InOutState.Wheels[DrivenLeftIndex].AngularSpeedRadPerSec;

    const double RightWheelSpeed =
        InOutState.Wheels[DrivenRightIndex].AngularSpeedRadPerSec;

    const double DifferentialCarrierSpeed =
        0.5 * (LeftWheelSpeed + RightWheelSpeed);

    double GearboxInputTargetSpeed = 0.0;

    if (FMath::Abs(GearRatio) > UE_DOUBLE_SMALL_NUMBER &&
        FMath::Abs(Config.Gearbox.FinalDriveRatio) > UE_DOUBLE_SMALL_NUMBER)
    {
        const double GearboxOutputTargetSpeed =
            DifferentialCarrierSpeed
            * Config.Gearbox.FinalDriveRatio;

        GearboxInputTargetSpeed =
            GearboxOutputTargetSpeed * GearRatio;
    }

    double ClutchTorqueNm = 0.0;

    if (FMath::Abs(GearRatio) > UE_DOUBLE_SMALL_NUMBER)
    {
        ClutchTorqueNm =
            TAPowertrainSolver::CalculateClutchTorqueNm(
                Config.Clutch,
                InOutState.Clutch,
                InOutState.Engine.AngularSpeedRadPerSec,
                GearboxInputTargetSpeed);
    }

    TADamage::UpdateRadiatorFluidLoss(
        Config.Radiator,
        DeltaTimeSeconds,
        InOutState.Radiator);

    const double CombustionTorqueNm =
        TAPowertrainSolver::CalculateCombustionTorqueNm(
            Config.Engine,
            InOutState.Engine,
            Throttle01);

    const double StarterTorqueNm =
        TAPowertrainSolver::CalculateStarterTorqueNm(
            Config.Engine,
            InOutState.Engine,
            Input.Controls.bStarterEngaged);

    InOutState.Engine.AngularSpeedRadPerSec =
        TAPowertrainSolver::IntegrateEngineAngularSpeed(
            Config.Engine,
            InOutState.Engine.AngularSpeedRadPerSec,
            CombustionTorqueNm + StarterTorqueNm,
            ClutchTorqueNm,
            DeltaTimeSeconds);

    TAPowertrainSolver::UpdateEngineRunState(
        Config.Engine,
        Input.Controls.bStarterEngaged,
        InOutState.Engine);

    double DifferentialInputTorqueNm = 0.0;

    if (FMath::Abs(GearRatio) > UE_DOUBLE_SMALL_NUMBER)
    {
        const double GearboxOutputTorqueNm =
            TAPowertrainSolver::CalculateGearboxOutputTorqueNm(
                Config.Gearbox,
                ClutchTorqueNm,
                GearRatio);

        DifferentialInputTorqueNm =
            TAPowertrainSolver::CalculateFinalDriveOutputTorqueNm(
                Config.Gearbox,
                GearboxOutputTorqueNm);
    }

    const FTATireSolveInput LeftCapacityInput =
        BuildTireInput(
            Input.WheelContacts[DrivenLeftIndex],
            LeftWheelSpeed);

    const FTATireSolveInput RightCapacityInput =
        BuildTireInput(
            Input.WheelContacts[DrivenRightIndex],
            RightWheelSpeed);

    const double LeftReactionCapacityNm =
        TATireSolver::EstimateLongitudinalForceCapacityN(
            Config.Tires[DrivenLeftIndex],
            InOutState.Wheels[DrivenLeftIndex].TireState,
            LeftCapacityInput)
        * FMath::Max(0.01, Config.Wheels[DrivenLeftIndex].RadiusM);

    const double RightReactionCapacityNm =
        TATireSolver::EstimateLongitudinalForceCapacityN(
            Config.Tires[DrivenRightIndex],
            InOutState.Wheels[DrivenRightIndex].TireState,
            RightCapacityInput)
        * FMath::Max(0.01, Config.Wheels[DrivenRightIndex].RadiusM);

    const FTAOpenDifferentialTorqueOutput DifferentialOutput =
        TAPowertrainSolver::CalculateOpenDifferentialTorque(
            DifferentialInputTorqueNm,
            LeftReactionCapacityNm,
            RightReactionCapacityNm);

    OutOutput.LeftDrivenWheelTorqueNm =
        DifferentialOutput.LeftWheelTorqueNm
        * FMath::Clamp(
            InOutState.WheelHubDamage[DrivenLeftIndex]
                .DriveEfficiency01,
            0.0,
            1.0);

    OutOutput.RightDrivenWheelTorqueNm =
        DifferentialOutput.RightWheelTorqueNm
        * FMath::Clamp(
            InOutState.WheelHubDamage[DrivenRightIndex]
                .DriveEfficiency01,
            0.0,
            1.0);

    for (int32 Index = 0; Index < Config.Wheels.Num(); ++Index)
    {
        FTAWheelRuntimeState& WheelState = InOutState.Wheels[Index];

        FTATireSolveInput TireInput =
            BuildTireInput(
                Input.WheelContacts[Index],
                WheelState.AngularSpeedRadPerSec);

        const FTATireSolveOutput TireOutput =
            TATireSolver::Solve(
                Config.Tires[Index],
                WheelState.TireState,
                TireInput);

        double DriveTorqueNm = 0.0;

        if (Index == DrivenLeftIndex)
        {
            DriveTorqueNm = DifferentialOutput.LeftWheelTorqueNm;
        }
        else if (Index == DrivenRightIndex)
        {
            DriveTorqueNm = DifferentialOutput.RightWheelTorqueNm;
        }

        const FTAWheelHubFunctionalDamageState& HubDamage =
            InOutState.WheelHubDamage[Index];

        DriveTorqueNm *=
            FMath::Clamp(
                HubDamage.DriveEfficiency01,
                0.0,
                1.0);

        const double RadiusM =
            FMath::Max(0.01, Config.Wheels[Index].RadiusM);

        double TireReactionTorqueNm =
            -TireOutput.LongitudinalForceN * RadiusM;

        const double BearingDragTorqueNm =
            FMath::Max(
                0.0,
                HubDamage.BearingDragTorqueNm);

        if (WheelState.AngularSpeedRadPerSec > UE_DOUBLE_SMALL_NUMBER)
        {
            TireReactionTorqueNm -=
                BearingDragTorqueNm;
        }
        else if (WheelState.AngularSpeedRadPerSec < -UE_DOUBLE_SMALL_NUMBER)
        {
            TireReactionTorqueNm +=
                BearingDragTorqueNm;
        }

        FTAWheelRuntimeConfig EffectiveWheelConfig =
            Config.Wheels[Index];

        EffectiveWheelConfig.MaxBrakeTorqueNm *=
            FMath::Clamp(
                HubDamage.BrakeEfficiency01,
                0.0,
                1.0);

        WheelState.AngularSpeedRadPerSec =
            IntegrateWheelAngularSpeed(
                EffectiveWheelConfig,
                WheelState.AngularSpeedRadPerSec,
                DriveTorqueNm,
                TireReactionTorqueNm,
                Brake01,
                DeltaTimeSeconds);

        WheelState.LastTireOutput = TireOutput;

        TATireSolver::UpdateThermalPressureAndWear(
            Config.Tires[Index],
            TireInput,
            TireOutput,
            DeltaTimeSeconds,
            WheelState.TireState);

        const FTAWheelContactInput& Contact =
            Input.WheelContacts[Index];

        FVector3d ForwardWorld =
            Contact.ForwardDirectionWorld.GetSafeNormal();

        FVector3d RightWorld =
            Contact.RightDirectionWorld.GetSafeNormal();

        if (ForwardWorld.IsNearlyZero())
        {
            ForwardWorld = FVector3d(1.0, 0.0, 0.0);
        }

        if (RightWorld.IsNearlyZero())
        {
            RightWorld = FVector3d(0.0, 1.0, 0.0);
        }

        const double LongitudinalForceN =
            TireOutput.LongitudinalForceN
            + TireOutput.RollingResistanceForceN;

        const FVector3d TireForceWorldN =
            ForwardWorld * LongitudinalForceN
            + RightWorld * TireOutput.LateralForceN;

        TAChassisDynamics::AddForceAtWorldPoint(
            InOutState.Chassis,
            TireForceWorldN,
            Contact.ContactPointWorldM,
            ChassisForces);

        const FVector3d SuspensionApplicationPointWorldM =
            Contact.bHasSuspensionForceApplicationPoint
            ? Contact.SuspensionForceApplicationPointWorldM
            : Contact.ContactPointWorldM;

        TAChassisDynamics::AddForceAtWorldPoint(
            InOutState.Chassis,
            Contact.SuspensionForceWorldN,
            SuspensionApplicationPointWorldM,
            ChassisForces);

        const FVector3d ContactNormalWorld =
            FVector3d::CrossProduct(
                ForwardWorld,
                RightWorld).GetSafeNormal();

        if (!ContactNormalWorld.IsNearlyZero())
        {
            TAChassisDynamics::AddTorqueWorld(
                ContactNormalWorld
                    * TireOutput.AligningMomentNm,
                ChassisForces);
        }

        OutOutput.TotalLongitudinalForceN +=
            LongitudinalForceN;

        OutOutput.TotalLateralForceN +=
            TireOutput.LateralForceN;

        OutOutput.TotalAligningMomentNm +=
            TireOutput.AligningMomentNm;
    }

    if (!TAChassisDynamics::Integrate(
            Config.Chassis,
            ChassisForces,
            DeltaTimeSeconds,
            InOutState.Chassis))
    {
        return false;
    }

    OutOutput.ClutchSlipRadPerSec =
        InOutState.Engine.AngularSpeedRadPerSec
        - GearboxInputTargetSpeed;

    TAPowertrainSolver::UpdateClutchThermalAndWear(
        Config.Clutch,
        ClutchTorqueNm,
        OutOutput.ClutchSlipRadPerSec,
        DeltaTimeSeconds,
        InOutState.Clutch);

    OutOutput.EngineRPM =
        InOutState.Engine.AngularSpeedRadPerSec
        * 60.0
        / (2.0 * UE_DOUBLE_PI);

    TAPowertrainSolver::UpdateEngineThermalState(
        Config.EngineThermal,
        Throttle01,
        OutOutput.EngineRPM,
        InOutState.Radiator.CoolingEfficiency01,
        DeltaTimeSeconds,
        InOutState.EngineThermal,
        InOutState.Engine);

    OutOutput.EngineCoolantTemperatureC =
        InOutState.EngineThermal.CoolantTemperatureC;

    OutOutput.EngineThermalTorqueFactor =
        InOutState.Engine.ThermalTorqueFactor;

    OutOutput.CoolingEfficiency01 =
        InOutState.Radiator.CoolingEfficiency01;

    OutOutput.CoolantMassKg =
        InOutState.Radiator.CoolantMassKg;

    ++InOutState.SimulationTick;
    return true;
}
