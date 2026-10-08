#include "TAHvBattery.h"

bool TAHvBattery::ValidateConfig(const FTAHvBatteryConfig& Config)
{
    return
        FMath::IsFinite(Config.CapacityKWh)
        && Config.CapacityKWh > UE_DOUBLE_SMALL_NUMBER
        && FMath::IsFinite(Config.EmptyOpenCircuitVoltageV)
        && Config.EmptyOpenCircuitVoltageV > 0.0
        && FMath::IsFinite(Config.FullOpenCircuitVoltageV)
        && Config.FullOpenCircuitVoltageV >= Config.EmptyOpenCircuitVoltageV
        && FMath::IsFinite(Config.InternalResistanceOhm)
        && Config.InternalResistanceOhm >= 0.0
        && FMath::IsFinite(Config.MaxDischargeCurrentA)
        && Config.MaxDischargeCurrentA >= 0.0
        && FMath::IsFinite(Config.MaxChargeCurrentA)
        && Config.MaxChargeCurrentA >= 0.0;
}

double TAHvBattery::CalculateOpenCircuitVoltageV(
    const FTAHvBatteryConfig& Config,
    const double StateOfCharge01)
{
    return FMath::Lerp(
        Config.EmptyOpenCircuitVoltageV,
        Config.FullOpenCircuitVoltageV,
        FMath::Clamp(StateOfCharge01, 0.0, 1.0));
}

bool TAHvBattery::Step(
    const FTAHvBatteryConfig& Config,
    const FTAHvBatteryInput& Input,
    FTAHvBatteryState& InOutState,
    FTAHvBatteryOutput& OutOutput)
{
    OutOutput = FTAHvBatteryOutput{};

    if (!ValidateConfig(Config)
        || !FMath::IsFinite(Input.RequestedTerminalPowerW)
        || !FMath::IsFinite(Input.DeltaTimeSeconds)
        || Input.DeltaTimeSeconds <= 0.0
        || !FMath::IsFinite(InOutState.StateOfCharge01))
    {
        return false;
    }

    InOutState.StateOfCharge01 =
        FMath::Clamp(InOutState.StateOfCharge01, 0.0, 1.0);

    OutOutput.OpenCircuitVoltageV =
        CalculateOpenCircuitVoltageV(Config, InOutState.StateOfCharge01);

    double RequestedPowerW = Input.RequestedTerminalPowerW;

    if (InOutState.StateOfCharge01 <= 0.0 && RequestedPowerW > 0.0)
    {
        RequestedPowerW = 0.0;
        OutOutput.bPowerLimited = true;
    }

    if (InOutState.StateOfCharge01 >= 1.0 && RequestedPowerW < 0.0)
    {
        RequestedPowerW = 0.0;
        OutOutput.bPowerLimited = true;
    }

    double RequestedCurrentA = 0.0;

    if (FMath::Abs(RequestedPowerW) > UE_DOUBLE_SMALL_NUMBER)
    {
        if (Config.InternalResistanceOhm <= UE_DOUBLE_SMALL_NUMBER)
        {
            RequestedCurrentA =
                RequestedPowerW
                / FMath::Max(
                    OutOutput.OpenCircuitVoltageV,
                    UE_DOUBLE_SMALL_NUMBER);
        }
        else
        {
            const double Discriminant =
                OutOutput.OpenCircuitVoltageV
                    * OutOutput.OpenCircuitVoltageV
                - 4.0
                    * Config.InternalResistanceOhm
                    * RequestedPowerW;

            if (Discriminant >= 0.0)
            {
                RequestedCurrentA =
                    (OutOutput.OpenCircuitVoltageV
                        - FMath::Sqrt(Discriminant))
                    / (2.0 * Config.InternalResistanceOhm);
            }
            else
            {
                RequestedCurrentA =
                    OutOutput.OpenCircuitVoltageV
                    / (2.0 * Config.InternalResistanceOhm);

                OutOutput.bPowerLimited = true;
            }
        }
    }

    const double CurrentA =
        FMath::Clamp(
            RequestedCurrentA,
            -Config.MaxChargeCurrentA,
            Config.MaxDischargeCurrentA);

    if (!FMath::IsNearlyEqual(CurrentA, RequestedCurrentA, 1.0e-9))
    {
        OutOutput.bPowerLimited = true;
    }

    OutOutput.CurrentA = CurrentA;

    OutOutput.TerminalVoltageV =
        FMath::Max(
            0.0,
            OutOutput.OpenCircuitVoltageV
            - CurrentA * Config.InternalResistanceOhm);

    OutOutput.ActualTerminalPowerW =
        OutOutput.TerminalVoltageV
        * CurrentA;

    OutOutput.ChemicalPowerW =
        OutOutput.OpenCircuitVoltageV
        * CurrentA;

    OutOutput.InternalHeatPowerW =
        CurrentA * CurrentA
        * Config.InternalResistanceOhm;

    const double CapacityJ =
        Config.CapacityKWh
        * 3.6e6;

    InOutState.StateOfCharge01 =
        FMath::Clamp(
            InOutState.StateOfCharge01
            - OutOutput.ChemicalPowerW
                * Input.DeltaTimeSeconds
                / CapacityJ,
            0.0,
            1.0);

    OutOutput.StateOfCharge01 =
        InOutState.StateOfCharge01;

    if (FMath::Abs(
            OutOutput.ActualTerminalPowerW
            - RequestedPowerW)
        > FMath::Max(
            1.0,
            1.0e-6 * FMath::Abs(RequestedPowerW)))
    {
        OutOutput.bPowerLimited = true;
    }

    return true;
}
