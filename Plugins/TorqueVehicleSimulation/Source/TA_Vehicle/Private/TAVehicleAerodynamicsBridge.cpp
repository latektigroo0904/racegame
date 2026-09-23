#include "TAVehicleAerodynamicsBridge.h"

bool TAVehicleAerodynamicsBridge::AddToChassis(
    const FTAVehicleRuntimeConfig& Config,
    const FTAVehicleStepInput& Input,
    const FTAChassisState& Chassis,
    FTAChassisForceAccumulator& InOutAccumulator,
    FTAVehicleStepOutput& InOutOutput)
{
    return TAAerodynamics::AddToChassis(
        Config.Aerodynamics,
        Input.AerodynamicsEnvironment,
        Chassis,
        InOutAccumulator,
        &InOutOutput.Aerodynamics);
}
