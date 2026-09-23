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

    const double IdleRadPerSec =
        FMath::Max(0.0, Config.Engine.IdleRPM)
        * (2.0 * UE_DOUBLE_PI)
        / 60.0;

    OutState.Engine.AngularSpeedRadPerSec = IdleRadPerSec;
    OutState.Engine.RunState = ETAEngineRunState::Running;

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

    const double CombustionTorqueNm =
        TAPowertrainSolver::CalculateCombustionTorqueNm(
            Config.Engine,
            InOutState.Engine,
            Throttle01);

    InOutState.Engine.AngularSpeedRadPerSec =
        TAPowertrainSolver::IntegrateEngineAngularSpeed(
            Config.Engine,
            InOutState.Engine.AngularSpeedRadPerSec,
            CombustionTorqueNm,
            ClutchTorqueNm,
            DeltaTimeSeconds);

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
        DifferentialOutput.LeftWheelTorqueNm;

    OutOutput.RightDrivenWheelTorqueNm =
        DifferentialOutput.RightWheelTorqueNm;

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

        const double RadiusM =
            FMath::Max(0.01, Config.Wheels[Index].RadiusM);

        const double TireReactionTorqueNm =
            -TireOutput.LongitudinalForceN * RadiusM;

        WheelState.AngularSpeedRadPerSec =
            IntegrateWheelAngularSpeed(
                Config.Wheels[Index],
                WheelState.AngularSpeedRadPerSec,
                DriveTorqueNm,
                TireReactionTorqueNm,
                Brake01,
                DeltaTimeSeconds);

        WheelState.LastTireOutput = TireOutput;

        OutOutput.TotalLongitudinalForceN +=
            TireOutput.LongitudinalForceN
            + TireOutput.RollingResistanceForceN;

        OutOutput.TotalLateralForceN +=
            TireOutput.LateralForceN;

        OutOutput.TotalAligningMomentNm +=
            TireOutput.AligningMomentNm;
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

    ++InOutState.SimulationTick;
    return true;
}
