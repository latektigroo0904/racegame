#include "TARearAxleRuntime.h"

bool TARearAxleRuntime::Resolve(
    const FTAChassisState& Chassis,
    const FTARearAxleRuntimeConfig& Config,
    const FTARearAxleSolveInput& Input,
    const double LeftWheelRadiusM,
    const double RightWheelRadiusM,
    const double DeltaTimeSeconds,
    FTARearAxleRuntimeState& InOutState,
    FTARearAxleSolveOutput& OutOutput)
{
    OutOutput = FTARearAxleSolveOutput{};

    if (DeltaTimeSeconds <= 0.0 ||
        LeftWheelRadiusM <= UE_DOUBLE_SMALL_NUMBER ||
        RightWheelRadiusM <= UE_DOUBLE_SMALL_NUMBER ||
        !TAMultiLinkSolver::ValidateConfig(Config.RightGeometry))
    {
        return false;
    }

    const FTAMultiLinkSolverConfig LeftGeometry =
        TAMultiLinkSolver::MirrorAcrossCenterline(
            Config.RightGeometry);

    if (!TAMultiLinkSolver::ValidateConfig(LeftGeometry))
    {
        return false;
    }

    if (!TAMultiLinkContactResolver::ResolveRoadContact(
            Chassis,
            LeftGeometry,
            Config.LeftSuspension,
            LeftWheelRadiusM,
            Input.LeftDamage,
            Input.LeftRoad,
            DeltaTimeSeconds,
            InOutState.LeftGeometry,
            InOutState.LeftSuspension,
            OutOutput.LeftContact))
    {
        return false;
    }

    if (!TAMultiLinkContactResolver::ResolveRoadContact(
            Chassis,
            Config.RightGeometry,
            Config.RightSuspension,
            RightWheelRadiusM,
            Input.RightDamage,
            Input.RightRoad,
            DeltaTimeSeconds,
            InOutState.RightGeometry,
            InOutState.RightSuspension,
            OutOutput.RightContact))
    {
        return false;
    }

    TAMultiLinkContactResolver::ApplyAntiRollBarToPair(
        Config.AntiRollBar,
        OutOutput.LeftContact,
        OutOutput.RightContact);

    OutOutput.LeftVehicleContact =
        TAMultiLinkContactResolver::BuildVehicleWheelContactInput(
            OutOutput.LeftContact);

    OutOutput.RightVehicleContact =
        TAMultiLinkContactResolver::BuildVehicleWheelContactInput(
            OutOutput.RightContact);

    OutOutput.LeftToeRad =
        OutOutput.LeftContact.Geometry.ToeRad;

    OutOutput.RightToeRad =
        OutOutput.RightContact.Geometry.ToeRad;

    OutOutput.LeftCamberRad =
        OutOutput.LeftContact.Geometry.CamberRad;

    OutOutput.RightCamberRad =
        OutOutput.RightContact.Geometry.CamberRad;

    OutOutput.bSolved = true;
    return true;
}


bool TARearAxleRuntime::ResolveWithTireCompliance(
    const FTAChassisState& Chassis,
    const FTARearAxleRuntimeConfig& Config,
    const FTARearAxleSolveInput& Input,
    const FTATireRuntimeConfig& LeftTireConfig,
    const FTATireRuntimeConfig& RightTireConfig,
    const double DeltaTimeSeconds,
    FTARearAxleRuntimeState& InOutState,
    FTATireRuntimeState& InOutLeftTireState,
    FTATireRuntimeState& InOutRightTireState,
    FTARearAxleSolveOutput& OutOutput)
{
    OutOutput = FTARearAxleSolveOutput{};

    if (DeltaTimeSeconds <= 0.0 ||
        !TAMultiLinkSolver::ValidateConfig(
            Config.RightGeometry))
    {
        return false;
    }

    const FTAMultiLinkSolverConfig LeftGeometry =
        TAMultiLinkSolver::MirrorAcrossCenterline(
            Config.RightGeometry);

    if (!TAMultiLinkSolver::ValidateConfig(
            LeftGeometry))
    {
        return false;
    }

    const FTARearAxleRuntimeState BaseAxleState =
        InOutState;

    const FTATireRuntimeState BaseLeftTireState =
        InOutLeftTireState;

    const FTATireRuntimeState BaseRightTireState =
        InOutRightTireState;

    double LeftAntiRollReactionN = 0.0;
    double RightAntiRollReactionN = 0.0;

    constexpr int32 CouplingIterations = 6;
    constexpr double ReactionToleranceN = 2.0;

    for (int32 Iteration = 0;
         Iteration < CouplingIterations;
         ++Iteration)
    {
        FTARearAxleRuntimeState ProbeState =
            BaseAxleState;

        FTATireRuntimeState ProbeLeftTire =
            BaseLeftTireState;

        FTATireRuntimeState ProbeRightTire =
            BaseRightTireState;

        FTAResolvedMultiLinkContact ProbeLeftContact;
        FTAResolvedMultiLinkContact ProbeRightContact;

        if (!TAMultiLinkContactResolver::ResolveCompliantRoadContact(
                Chassis,
                LeftGeometry,
                Config.LeftSuspension,
                LeftTireConfig,
                LeftAntiRollReactionN,
                Input.LeftDamage,
                Input.LeftRoad,
                DeltaTimeSeconds,
                ProbeState.LeftGeometry,
                ProbeState.LeftSuspension,
                ProbeLeftTire,
                ProbeLeftContact) ||
            !TAMultiLinkContactResolver::ResolveCompliantRoadContact(
                Chassis,
                Config.RightGeometry,
                Config.RightSuspension,
                RightTireConfig,
                RightAntiRollReactionN,
                Input.RightDamage,
                Input.RightRoad,
                DeltaTimeSeconds,
                ProbeState.RightGeometry,
                ProbeState.RightSuspension,
                ProbeRightTire,
                ProbeRightContact))
        {
            return false;
        }

        const FTAAntiRollBarOutput AntiRoll =
            TASuspensionRuntime::CalculateAntiRollBar(
                Config.AntiRollBar,
                ProbeLeftContact.TravelM,
                ProbeRightContact.TravelM);

        const double MaxReactionDeltaN =
            FMath::Max(
                FMath::Abs(
                    AntiRoll.LeftLoadAdjustmentN
                    - LeftAntiRollReactionN),
                FMath::Abs(
                    AntiRoll.RightLoadAdjustmentN
                    - RightAntiRollReactionN));

        LeftAntiRollReactionN =
            FMath::Lerp(
                LeftAntiRollReactionN,
                AntiRoll.LeftLoadAdjustmentN,
                0.75);

        RightAntiRollReactionN =
            FMath::Lerp(
                RightAntiRollReactionN,
                AntiRoll.RightLoadAdjustmentN,
                0.75);

        if (MaxReactionDeltaN <= ReactionToleranceN)
        {
            LeftAntiRollReactionN =
                AntiRoll.LeftLoadAdjustmentN;

            RightAntiRollReactionN =
                AntiRoll.RightLoadAdjustmentN;

            break;
        }
    }

    FTARearAxleRuntimeState FinalState =
        BaseAxleState;

    FTATireRuntimeState FinalLeftTire =
        BaseLeftTireState;

    FTATireRuntimeState FinalRightTire =
        BaseRightTireState;

    if (!TAMultiLinkContactResolver::ResolveCompliantRoadContact(
            Chassis,
            LeftGeometry,
            Config.LeftSuspension,
            LeftTireConfig,
            LeftAntiRollReactionN,
            Input.LeftDamage,
            Input.LeftRoad,
            DeltaTimeSeconds,
            FinalState.LeftGeometry,
            FinalState.LeftSuspension,
            FinalLeftTire,
            OutOutput.LeftContact) ||
        !TAMultiLinkContactResolver::ResolveCompliantRoadContact(
            Chassis,
            Config.RightGeometry,
            Config.RightSuspension,
            RightTireConfig,
            RightAntiRollReactionN,
            Input.RightDamage,
            Input.RightRoad,
            DeltaTimeSeconds,
            FinalState.RightGeometry,
            FinalState.RightSuspension,
            FinalRightTire,
            OutOutput.RightContact))
    {
        return false;
    }

    InOutState =
        FinalState;

    InOutLeftTireState =
        FinalLeftTire;

    InOutRightTireState =
        FinalRightTire;

    OutOutput.LeftVehicleContact =
        TAMultiLinkContactResolver::BuildVehicleWheelContactInput(
            OutOutput.LeftContact);

    OutOutput.RightVehicleContact =
        TAMultiLinkContactResolver::BuildVehicleWheelContactInput(
            OutOutput.RightContact);

    OutOutput.LeftToeRad =
        OutOutput.LeftContact.Geometry.ToeRad;

    OutOutput.RightToeRad =
        OutOutput.RightContact.Geometry.ToeRad;

    OutOutput.LeftCamberRad =
        OutOutput.LeftContact.Geometry.CamberRad;

    OutOutput.RightCamberRad =
        OutOutput.RightContact.Geometry.CamberRad;

    OutOutput.bSolved = true;
    return true;
}
