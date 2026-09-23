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

    if (!TAMultiLinkContactResolver::ResolveCompliantRoadContact(
            Chassis,
            LeftGeometry,
            Config.LeftSuspension,
            LeftTireConfig,
            Input.LeftDamage,
            Input.LeftRoad,
            DeltaTimeSeconds,
            InOutState.LeftGeometry,
            InOutState.LeftSuspension,
            InOutLeftTireState,
            OutOutput.LeftContact))
    {
        return false;
    }

    if (!TAMultiLinkContactResolver::ResolveCompliantRoadContact(
            Chassis,
            Config.RightGeometry,
            Config.RightSuspension,
            RightTireConfig,
            Input.RightDamage,
            Input.RightRoad,
            DeltaTimeSeconds,
            InOutState.RightGeometry,
            InOutState.RightSuspension,
            InOutRightTireState,
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
