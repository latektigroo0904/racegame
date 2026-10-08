#pragma once

#include "CoreMinimal.h"

struct TA_POWERTRAIN_API FTAAlternatorConfig
{
    double CutInEngineRPM = 800.0;
    double FullOutputEngineRPM = 2500.0;

    double RegulatedVoltageV = 14.2;
    double VoltageRegulationBandV = 1.0;

    double MaxCurrentA = 120.0;
    double ElectricalEfficiency01 = 0.70;
};

struct TA_POWERTRAIN_API FTAAlternatorInput
{
    double EngineRPM = 0.0;
    double BusVoltageV = 12.0;
    double Health01 = 1.0;
};

struct TA_POWERTRAIN_API FTAAlternatorOutput
{
    double SpeedAvailability01 = 0.0;
    double VoltageDemand01 = 0.0;

    double OutputCurrentA = 0.0;
    double ElectricalPowerW = 0.0;

    // Mechanical crankshaft/accessory load required to create output power.
    double MechanicalLoadTorqueNm = 0.0;
};

namespace TAAlternator
{
    TA_POWERTRAIN_API bool ValidateConfig(
        const FTAAlternatorConfig& Config);

    TA_POWERTRAIN_API bool Calculate(
        const FTAAlternatorConfig& Config,
        const FTAAlternatorInput& Input,
        FTAAlternatorOutput& OutOutput);
}
