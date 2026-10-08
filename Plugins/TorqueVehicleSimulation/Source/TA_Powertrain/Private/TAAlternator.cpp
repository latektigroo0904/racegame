#include "TAAlternator.h"

bool TAAlternator::ValidateConfig(
    const FTAAlternatorConfig& Config)
{
    return
        FMath::IsFinite(Config.CutInEngineRPM)
        && Config.CutInEngineRPM >= 0.0
        && FMath::IsFinite(Config.FullOutputEngineRPM)
        && Config.FullOutputEngineRPM > Config.CutInEngineRPM
        && FMath::IsFinite(Config.RegulatedVoltageV)
        && Config.RegulatedVoltageV > 0.0
        && FMath::IsFinite(Config.VoltageRegulationBandV)
        && Config.VoltageRegulationBandV > UE_DOUBLE_SMALL_NUMBER
        && FMath::IsFinite(Config.MaxCurrentA)
        && Config.MaxCurrentA >= 0.0
        && FMath::IsFinite(Config.ElectricalEfficiency01)
        && Config.ElectricalEfficiency01 > UE_DOUBLE_SMALL_NUMBER
        && Config.ElectricalEfficiency01 <= 1.0;
}

bool TAAlternator::Calculate(
    const FTAAlternatorConfig& Config,
    const FTAAlternatorInput& Input,
    FTAAlternatorOutput& OutOutput)
{
    OutOutput =
        FTAAlternatorOutput{};

    if (!ValidateConfig(Config)
        || !FMath::IsFinite(Input.EngineRPM)
        || !FMath::IsFinite(Input.BusVoltageV)
        || Input.BusVoltageV < 0.0
        || !FMath::IsFinite(Input.Health01))
    {
        return false;
    }

    const double EngineRPM =
        FMath::Max(0.0, Input.EngineRPM);

    OutOutput.SpeedAvailability01 =
        FMath::Clamp(
            (EngineRPM - Config.CutInEngineRPM)
            / (Config.FullOutputEngineRPM - Config.CutInEngineRPM),
            0.0,
            1.0);

    OutOutput.VoltageDemand01 =
        FMath::Clamp(
            (Config.RegulatedVoltageV - Input.BusVoltageV)
            / Config.VoltageRegulationBandV,
            0.0,
            1.0);

    OutOutput.OutputCurrentA =
        Config.MaxCurrentA
        * OutOutput.SpeedAvailability01
        * OutOutput.VoltageDemand01
        * FMath::Clamp(
            Input.Health01,
            0.0,
            1.0);

    OutOutput.ElectricalPowerW =
        Input.BusVoltageV
        * OutOutput.OutputCurrentA;

    const double EngineAngularSpeedRadPerSec =
        EngineRPM
        * (2.0 * UE_DOUBLE_PI)
        / 60.0;

    if (EngineAngularSpeedRadPerSec
        > UE_DOUBLE_SMALL_NUMBER)
    {
        const double RequiredMechanicalPowerW =
            OutOutput.ElectricalPowerW
            / Config.ElectricalEfficiency01;

        OutOutput.MechanicalLoadTorqueNm =
            RequiredMechanicalPowerW
            / EngineAngularSpeedRadPerSec;
    }

    return true;
}
