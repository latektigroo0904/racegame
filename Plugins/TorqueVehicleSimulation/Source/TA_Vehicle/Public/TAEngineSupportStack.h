#pragma once

#include "CoreMinimal.h"
#include "TAPowerSupportStack.h"

struct TA_VEHICLE_API FTAEngineSupportStackConfig
{
    FTAPowerSupportStackConfig PowerSupport;
    FTAOilSystemConfig Oil;
};

struct TA_VEHICLE_API FTAEngineSupportStackState
{
    FTAPowerSupportStackState PowerSupport;
    FTAOilSystemState Oil;
};

struct TA_VEHICLE_API FTAEngineSupportStackInput
{
    FTAPowerSupportStackInput PowerSupport;

    double OilPumpHealth01 = 1.0;
    double OilLeakAreaM2 = 0.0;
    double OilGalleryPressureForLeakPa = 0.0;
};

struct TA_VEHICLE_API FTAEngineSupportStackOutput
{
    FTAPowerSupportStackOutput PowerSupport;
    FTAOilSystemOutput Oil;

    double CombustionAuthority01 = 0.0;
    double LubricationAuthority01 = 0.0;
    double CoolingAuthority01 = 0.0;

    double AlternatorMechanicalLoadTorqueNm = 0.0;
};

namespace TAEngineSupportStack
{
    TA_VEHICLE_API bool ValidateConfig(
        const FTAEngineSupportStackConfig& Config);

    TA_VEHICLE_API void InitializeState(
        const FTAEngineSupportStackConfig& Config,
        FTAEngineSupportStackState& OutState);

    TA_VEHICLE_API bool Step(
        const FTAEngineSupportStackConfig& Config,
        const FTAEngineSupportStackInput& Input,
        FTAEngineSupportStackState& InOutState,
        FTAEngineSupportStackOutput& OutOutput);
}
