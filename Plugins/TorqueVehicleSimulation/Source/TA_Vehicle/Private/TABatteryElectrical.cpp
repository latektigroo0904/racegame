#include "TABatteryElectrical.h"

bool TABatteryElectrical::ValidateConfig(
    const FTABatteryConfig& Config)
{
    return
        FMath::IsFinite(Config.CapacityAh)
        && Config.CapacityAh > UE_DOUBLE_SMALL_NUMBER
        && FMath::IsFinite(Config.EmptyOpenCircuitVoltageV)
        && Config.EmptyOpenCircuitVoltageV > 0.0
        && FMath::IsFinite(Config.FullOpenCircuitVoltageV)
        && Config.FullOpenCircuitVoltageV
            >= Config.EmptyOpenCircuitVoltageV
        && FMath::IsFinite(Config.InternalResistanceOhm)
        && Config.InternalResistanceOhm >= 0.0
        && FMath::IsFinite(Config.MaxDischargeCurrentA)
        && Config.MaxDischargeCurrentA >= 0.0
        && FMath::IsFinite(Config.MaxChargeCurrentA)
        && Config.MaxChargeCurrentA >= 0.0;
}

double TABatteryElectrical::CalculateOpenCircuitVoltageV(
    const FTABatteryConfig& Config,
    const double StateOfCharge01)
{
    return FMath::Lerp(
        Config.EmptyOpenCircuitVoltageV,
        Config.FullOpenCircuitVoltageV,
        FMath::Clamp(
            StateOfCharge01,
            0.0,
            1.0));
}

bool TABatteryElectrical::Step(
    const FTABatteryConfig& Config,
    const FTABatteryInput& Input,
    FTABatteryState& InOutState,
    FTABatteryOutput& OutOutput)
{
    OutOutput =
        FTABatteryOutput{};

    if (!ValidateConfig(Config)
        || !FMath::IsFinite(Input.RequestedLoadCurrentA)
        || Input.RequestedLoadCurrentA < 0.0
        || !FMath::IsFinite(Input.AlternatorCurrentA)
        || Input.AlternatorCurrentA < 0.0
        || !FMath::IsFinite(Input.DeltaTimeSeconds)
        || Input.DeltaTimeSeconds <= 0.0
        || !FMath::IsFinite(InOutState.StateOfCharge01))
    {
        return false;
    }

    InOutState.StateOfCharge01 =
        FMath::Clamp(
            InOutState.StateOfCharge01,
            0.0,
            1.0);

    OutOutput.OpenCircuitVoltageV =
        CalculateOpenCircuitVoltageV(
            Config,
            InOutState.StateOfCharge01);

    const double NetRequestedBatteryCurrentA =
        Input.RequestedLoadCurrentA
        - Input.AlternatorCurrentA;

    if (NetRequestedBatteryCurrentA >= 0.0)
    {
        OutOutput.BatteryCurrentA =
            FMath::Min(
                NetRequestedBatteryCurrentA,
                Config.MaxDischargeCurrentA);

        OutOutput.DeliveredLoadCurrentA =
            Input.AlternatorCurrentA
            + OutOutput.BatteryCurrentA;

        OutOutput.AcceptedChargeCurrentA =
            0.0;
    }
    else
    {
        const double ChargeCurrentA =
            FMath::Min(
                -NetRequestedBatteryCurrentA,
                Config.MaxChargeCurrentA);

        OutOutput.BatteryCurrentA =
            -ChargeCurrentA;

        OutOutput.DeliveredLoadCurrentA =
            Input.RequestedLoadCurrentA;

        OutOutput.AcceptedChargeCurrentA =
            ChargeCurrentA;
    }

    OutOutput.TerminalVoltageV =
        FMath::Max(
            0.0,
            OutOutput.OpenCircuitVoltageV
            - OutOutput.BatteryCurrentA
                * Config.InternalResistanceOhm);

    const double CapacityAs =
        Config.CapacityAh
        * 3600.0;

    InOutState.StateOfCharge01 =
        FMath::Clamp(
            InOutState.StateOfCharge01
            - OutOutput.BatteryCurrentA
                * Input.DeltaTimeSeconds
                / CapacityAs,
            0.0,
            1.0);

    OutOutput.StateOfCharge01 =
        InOutState.StateOfCharge01;

    return true;
}
