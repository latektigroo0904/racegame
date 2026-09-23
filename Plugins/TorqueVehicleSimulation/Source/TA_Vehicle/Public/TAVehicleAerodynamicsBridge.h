#pragma once

#include "CoreMinimal.h"
#include "TAVehicleSimulation.h"

namespace TAVehicleAerodynamicsBridge
{
    // Single ownership point between vehicle-step data and the low-level aero solver.
    // This keeps environment data out of authored vehicle content and guarantees
    // aero uses the same chassis force accumulator as tires and suspension.
    TA_VEHICLE_API bool AddToChassis(
        const FTAVehicleRuntimeConfig& Config,
        const FTAVehicleStepInput& Input,
        const FTAChassisState& Chassis,
        FTAChassisForceAccumulator& InOutAccumulator,
        FTAVehicleStepOutput& InOutOutput);
}
