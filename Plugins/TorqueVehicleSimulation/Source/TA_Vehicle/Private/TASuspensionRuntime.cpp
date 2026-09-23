#include "TASuspensionRuntime.h"

bool TASuspensionRuntime::EvaluateKinematicCache(
    const FTASuspensionRuntimeConfig& Config,
    const double TravelM,
    FTASuspensionRuntimeState& InOutState)
{
    if (!InOutState.bKinematicCacheValid || Config.KinematicSamples.Num() == 0)
    {
        return false;
    }

    const TArray<FTASuspensionKinematicSample>& Samples = Config.KinematicSamples;

    const FTASuspensionKinematicSample* Lower = &Samples[0];
    const FTASuspensionKinematicSample* Upper = &Samples.Last();

    if (TravelM <= Samples[0].TravelM)
    {
        Lower = Upper = &Samples[0];
    }
    else if (TravelM >= Samples.Last().TravelM)
    {
        Lower = Upper = &Samples.Last();
    }
    else
    {
        for (int32 Index = 1; Index < Samples.Num(); ++Index)
        {
            if (TravelM <= Samples[Index].TravelM)
            {
                Lower = &Samples[Index - 1];
                Upper = &Samples[Index];
                break;
            }
        }
    }

    double Alpha = 0.0;

    if (Upper != Lower)
    {
        const double Range =
            FMath::Max(UE_DOUBLE_SMALL_NUMBER, Upper->TravelM - Lower->TravelM);

        Alpha =
            FMath::Clamp((TravelM - Lower->TravelM) / Range, 0.0, 1.0);
    }

    InOutState.TravelM = TravelM;

    InOutState.WheelCenterOffsetM =
        FMath::Lerp(
            Lower->WheelCenterOffsetM,
            Upper->WheelCenterOffsetM,
            Alpha);

    InOutState.CamberRad =
        FMath::Lerp(Lower->CamberRad, Upper->CamberRad, Alpha);

    InOutState.ToeRad =
        FMath::Lerp(Lower->ToeRad, Upper->ToeRad, Alpha);

    InOutState.MotionRatio =
        FMath::Max(
            0.01,
            FMath::Lerp(Lower->MotionRatio, Upper->MotionRatio, Alpha));

    return true;
}

FTASuspensionForceOutput TASuspensionRuntime::CalculateForce(
    const FTASuspensionRuntimeConfig& Config,
    const FTASuspensionRuntimeState& State)
{
    FTASuspensionForceOutput Output;

    const double MotionRatio =
        FMath::Max(0.01, State.MotionRatio);

    const double SpringCompressionM =
        FMath::Max(0.0, State.TravelM * MotionRatio);

    Output.SpringForceN =
        FMath::Max(0.0, Config.SpringRateNPerM)
        * SpringCompressionM
        * MotionRatio;

    const double DamperRate =
        State.TravelVelocityMps >= 0.0
        ? FMath::Max(0.0, Config.BumpDampingNsPerM)
        : FMath::Max(0.0, Config.ReboundDampingNsPerM);

    Output.DamperForceN =
        DamperRate
        * State.TravelVelocityMps
        * MotionRatio
        * MotionRatio;

    if (State.TravelM > Config.BumpStopTravelM)
    {
        Output.StopForceN =
            FMath::Max(0.0, Config.BumpStopRateNPerM)
            * (State.TravelM - Config.BumpStopTravelM);
    }
    else if (State.TravelM < Config.DroopStopTravelM)
    {
        Output.StopForceN =
            FMath::Min(
                0.0,
                FMath::Max(0.0, Config.DroopStopRateNPerM)
                * (State.TravelM - Config.DroopStopTravelM));
    }

    Output.TotalForceN =
        Output.SpringForceN
        + Output.DamperForceN
        + Output.StopForceN;

    return Output;
}
