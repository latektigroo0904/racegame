#include "TAEngineSupportStack.h"

bool TAEngineSupportStack::ValidateConfig(
    const FTAEngineSupportStackConfig& Config)
{
    return
        TAPowerSupportStack::ValidateConfig(
            Config.PowerSupport)
        && TAFluidSubsystems::ValidateOilConfig(
            Config.Oil);
}

void TAEngineSupportStack::InitializeState(
    const FTAEngineSupportStackConfig& Config,
    FTAEngineSupportStackState& OutState)
{
    OutState =
        FTAEngineSupportStackState{};

    TAPowerSupportStack::InitializeState(
        Config.PowerSupport,
        OutState.PowerSupport);

    TAFluidSubsystems::InitializeOilState(
        Config.Oil,
        OutState.Oil);
}

bool TAEngineSupportStack::Step(
    const FTAEngineSupportStackConfig& Config,
    const FTAEngineSupportStackInput& Input,
    FTAEngineSupportStackState& InOutState,
    FTAEngineSupportStackOutput& OutOutput)
{
    OutOutput =
        FTAEngineSupportStackOutput{};

    if (!ValidateConfig(Config)
        || !FMath::IsFinite(Input.OilPumpHealth01)
        || !FMath::IsFinite(Input.OilLeakAreaM2)
        || Input.OilLeakAreaM2 < 0.0
        || !FMath::IsFinite(Input.OilGalleryPressureForLeakPa)
        || Input.OilGalleryPressureForLeakPa < 0.0)
    {
        return false;
    }

    if (!TAPowerSupportStack::Step(
            Config.PowerSupport,
            Input.PowerSupport,
            InOutState.PowerSupport,
            OutOutput.PowerSupport))
    {
        return false;
    }

    FTAOilSystemInput OilInput;
    OilInput.EngineRPM =
        Input.PowerSupport.EngineRPM;

    OilInput.PumpHealth01 =
        Input.OilPumpHealth01;

    OilInput.LeakAreaM2 =
        Input.OilLeakAreaM2;

    OilInput.GalleryPressureForLeakPa =
        Input.OilGalleryPressureForLeakPa;

    OilInput.AmbientPressurePa =
        Input.PowerSupport.AmbientPressurePa;

    OilInput.DeltaTimeSeconds =
        Input.PowerSupport.DeltaTimeSeconds;

    if (!TAFluidSubsystems::StepOil(
            Config.Oil,
            OilInput,
            InOutState.Oil,
            OutOutput.Oil))
    {
        return false;
    }

    OutOutput.CombustionAuthority01 =
        FMath::Clamp(
            OutOutput.PowerSupport.EngineControlAuthority01
            * OutOutput.PowerSupport.Fuel.DeliveryAuthority01,
            0.0,
            1.0);

    OutOutput.LubricationAuthority01 =
        OutOutput.Oil.LubricationAuthority01;

    OutOutput.CoolingAuthority01 =
        OutOutput.PowerSupport.Coolant.CoolingAuthority01;

    OutOutput.AlternatorMechanicalLoadTorqueNm =
        OutOutput.PowerSupport.Electrical
            .Alternator.MechanicalLoadTorqueNm;

    return true;
}
