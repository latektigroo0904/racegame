#include "TAStartingElectrical.h"

bool TAStartingElectrical::ValidateConfig(
    const FTAStartingElectricalConfig& Config)
{
    return
        TABatteryElectrical::ValidateConfig(
            Config.Battery)
        && TAStarterMotor::ValidateConfig(
            Config.Starter)
        && Config.MaxCouplingIterations >= 1
        && Config.MaxCouplingIterations <= 64
        && FMath::IsFinite(
            Config.VoltageConvergenceToleranceV)
        && Config.VoltageConvergenceToleranceV
            > UE_DOUBLE_SMALL_NUMBER;
}

void TAStartingElectrical::InitializeState(
    FTAStartingElectricalState& OutState)
{
    OutState =
        FTAStartingElectricalState{};
}

bool TAStartingElectrical::Step(
    const FTAStartingElectricalConfig& Config,
    const FTAStartingElectricalInput& Input,
    FTAStartingElectricalState& InOutState,
    FTAStartingElectricalOutput& OutOutput)
{
    OutOutput =
        FTAStartingElectricalOutput{};

    if (!ValidateConfig(Config)
        || !FMath::IsFinite(
            Input.EngineAngularSpeedRadPerSec)
        || Input.EngineAngularSpeedRadPerSec < 0.0
        || !FMath::IsFinite(Input.StarterHealth01)
        || !FMath::IsFinite(Input.OtherLoadCurrentA)
        || Input.OtherLoadCurrentA < 0.0
        || !FMath::IsFinite(Input.AlternatorCurrentA)
        || Input.AlternatorCurrentA < 0.0
        || !FMath::IsFinite(Input.DeltaTimeSeconds)
        || Input.DeltaTimeSeconds <= 0.0
        || !FMath::IsFinite(
            InOutState.Battery.StateOfCharge01))
    {
        return false;
    }

    const double OpenCircuitVoltageV =
        TABatteryElectrical::CalculateOpenCircuitVoltageV(
            Config.Battery,
            InOutState.Battery.StateOfCharge01);

    double EstimatedBusVoltageV =
        OpenCircuitVoltageV;

    FTAStarterMotorOutput StarterEstimate;

    for (int32 Iteration = 0;
         Iteration < Config.MaxCouplingIterations;
         ++Iteration)
    {
        FTAStarterMotorInput StarterInput;
        StarterInput.BusVoltageV =
            EstimatedBusVoltageV;

        StarterInput.EngineAngularSpeedRadPerSec =
            Input.EngineAngularSpeedRadPerSec;

        StarterInput.bEngaged =
            Input.bStarterEngaged;

        StarterInput.Health01 =
            Input.StarterHealth01;

        if (!TAStarterMotor::Calculate(
                Config.Starter,
                StarterInput,
                StarterEstimate))
        {
            return false;
        }

        const double RequestedLoadCurrentA =
            Input.OtherLoadCurrentA
            + StarterEstimate.CurrentA;

        const double NetRequestedBatteryCurrentA =
            RequestedLoadCurrentA
            - Input.AlternatorCurrentA;

        double BatteryCurrentA = 0.0;

        if (NetRequestedBatteryCurrentA >= 0.0)
        {
            BatteryCurrentA =
                FMath::Min(
                    NetRequestedBatteryCurrentA,
                    Config.Battery.MaxDischargeCurrentA);
        }
        else
        {
            BatteryCurrentA =
                -FMath::Min(
                    -NetRequestedBatteryCurrentA,
                    Config.Battery.MaxChargeCurrentA);
        }

        const double NewBusVoltageV =
            FMath::Max(
                0.0,
                OpenCircuitVoltageV
                - BatteryCurrentA
                    * Config.Battery.InternalResistanceOhm);

        OutOutput.CouplingIterations =
            Iteration + 1;

        if (FMath::Abs(
                NewBusVoltageV
                - EstimatedBusVoltageV)
            <= Config.VoltageConvergenceToleranceV)
        {
            EstimatedBusVoltageV =
                NewBusVoltageV;

            OutOutput.bConverged =
                true;

            break;
        }

        EstimatedBusVoltageV =
            NewBusVoltageV;
    }

    FTAStarterMotorInput FinalStarterInput;
    FinalStarterInput.BusVoltageV =
        EstimatedBusVoltageV;

    FinalStarterInput.EngineAngularSpeedRadPerSec =
        Input.EngineAngularSpeedRadPerSec;

    FinalStarterInput.bEngaged =
        Input.bStarterEngaged;

    FinalStarterInput.Health01 =
        Input.StarterHealth01;

    if (!TAStarterMotor::Calculate(
            Config.Starter,
            FinalStarterInput,
            OutOutput.Starter))
    {
        return false;
    }

    FTABatteryInput BatteryInput;
    BatteryInput.RequestedLoadCurrentA =
        Input.OtherLoadCurrentA
        + OutOutput.Starter.CurrentA;

    BatteryInput.AlternatorCurrentA =
        Input.AlternatorCurrentA;

    BatteryInput.DeltaTimeSeconds =
        Input.DeltaTimeSeconds;

    if (!TABatteryElectrical::Step(
            Config.Battery,
            BatteryInput,
            InOutState.Battery,
            OutOutput.Battery))
    {
        return false;
    }

    OutOutput.CoupledBusVoltageV =
        OutOutput.Battery.TerminalVoltageV;

    // Final check reports whether the state-integrating battery result remains
    // within tolerance of the fixed-point estimate.
    OutOutput.bConverged =
        OutOutput.bConverged
        && FMath::Abs(
            OutOutput.CoupledBusVoltageV
            - EstimatedBusVoltageV)
            <= Config.VoltageConvergenceToleranceV
                * 2.0;

    return true;
}
