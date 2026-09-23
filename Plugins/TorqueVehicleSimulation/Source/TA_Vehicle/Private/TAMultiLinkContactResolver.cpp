#include "TAMultiLinkContactResolver.h"

namespace
{
    struct FTravelEvaluation
    {
        double SignedContactDistanceM = 0.0;
    };

    bool EvaluateTravel(
        const FTAChassisState& Chassis,
        const FTAMultiLinkSolverConfig& Config,
        const double WheelRadiusM,
        const FTAMultiLinkDamageOffsets& Damage,
        const FVector3d& RoadPoint,
        const FVector3d& RoadNormal,
        const double TravelM,
        FTravelEvaluation& Out)
    {
        FTAMultiLinkRuntimeState State;
        FTAMultiLinkSolveOutput Geometry;
        FTAMultiLinkSolveInput Input;
        Input.TravelM = TravelM;
        Input.Damage = Damage;

        if (!TAMultiLinkSolver::Solve(Config, Input, State, Geometry))
        {
            return false;
        }

        const FVector3d CenterWorld =
            Chassis.PositionWorldM
            + Chassis.OrientationWorld.RotateVector(Geometry.WheelCenterLocalM);

        const FVector3d ContactWorld = CenterWorld - RoadNormal * WheelRadiusM;
        Out.SignedContactDistanceM =
            FVector3d::DotProduct(ContactWorld - RoadPoint, RoadNormal);
        return true;
    }

    double EstimateMotionRatio(
        const FTAMultiLinkSolverConfig& Config,
        const FTAMultiLinkDamageOffsets& Damage,
        const double TravelM,
        const double DamperLengthM)
    {
        constexpr double ProbeM = 0.001;
        double ProbeTravel = FMath::Min(Config.MaxTravelM, TravelM + ProbeM);
        if (FMath::IsNearlyEqual(ProbeTravel, TravelM, 1.0e-9))
        {
            ProbeTravel = FMath::Max(Config.MinTravelM, TravelM - ProbeM);
        }
        const double Delta = ProbeTravel - TravelM;
        if (FMath::Abs(Delta) <= 1.0e-9) return 1.0;

        FTAMultiLinkRuntimeState State;
        FTAMultiLinkSolveOutput Output;
        FTAMultiLinkSolveInput Input;
        Input.TravelM = ProbeTravel;
        Input.Damage = Damage;
        if (!TAMultiLinkSolver::Solve(Config, Input, State, Output)) return 1.0;

        return FMath::Clamp(
            FMath::Abs((Output.DamperLengthM - DamperLengthM) / Delta),
            0.05, 3.0);
    }
}

bool TAMultiLinkContactResolver::ResolveRoadContact(
    const FTAChassisState& Chassis,
    const FTAMultiLinkSolverConfig& GeometryConfig,
    const FTASuspensionRuntimeConfig& SuspensionConfig,
    const double WheelRadiusM,
    const FTAMultiLinkDamageOffsets& DamageOffsets,
    const FTARoadPlane& Road,
    const double DeltaTimeSeconds,
    FTAMultiLinkRuntimeState& InOutGeometryState,
    FTASuspensionRuntimeState& InOutSuspensionState,
    FTAResolvedMultiLinkContact& OutContact)
{
    OutContact = FTAResolvedMultiLinkContact{};
    OutContact.Surface = Road.Surface;

    if (DeltaTimeSeconds <= 0.0 || WheelRadiusM <= UE_DOUBLE_SMALL_NUMBER ||
        !TAMultiLinkSolver::ValidateConfig(GeometryConfig)) return false;

    const FVector3d RoadNormal = Road.NormalWorld.GetSafeNormal();
    if (RoadNormal.IsNearlyZero()) return false;
    OutContact.RoadNormalWorld = RoadNormal;

    const FVector3d ChassisUp = Chassis.OrientationWorld.RotateVector(FVector3d(0,0,1)).GetSafeNormal();
    if (FVector3d::DotProduct(ChassisUp, RoadNormal) < 0.20) return false;

    FTravelEvaluation Droop, Bump;
    if (!EvaluateTravel(Chassis, GeometryConfig, WheelRadiusM, DamageOffsets,
            Road.PointWorldM, RoadNormal, GeometryConfig.MinTravelM, Droop) ||
        !EvaluateTravel(Chassis, GeometryConfig, WheelRadiusM, DamageOffsets,
            Road.PointWorldM, RoadNormal, GeometryConfig.MaxTravelM, Bump)) return false;

    double FinalTravel = 0.0;
    if (Droop.SignedContactDistanceM > 0.0)
    {
        FinalTravel = GeometryConfig.MinTravelM;
        OutContact.bTravelClamped = true;
    }
    else if (Bump.SignedContactDistanceM < 0.0)
    {
        FinalTravel = GeometryConfig.MaxTravelM;
        OutContact.bInContact = true;
        OutContact.bTravelClamped = true;
        OutContact.PenetrationM = -Bump.SignedContactDistanceM;
    }
    else
    {
        double Low = GeometryConfig.MinTravelM;
        double High = GeometryConfig.MaxTravelM;
        for (int32 I = 0; I < 24; ++I)
        {
            const double Mid = 0.5 * (Low + High);
            FTravelEvaluation Eval;
            if (!EvaluateTravel(Chassis, GeometryConfig, WheelRadiusM, DamageOffsets,
                    Road.PointWorldM, RoadNormal, Mid, Eval)) return false;
            if (FMath::Abs(Eval.SignedContactDistanceM) <= 1.0e-5) { Low = High = Mid; break; }
            if (Eval.SignedContactDistanceM < 0.0) Low = Mid; else High = Mid;
        }
        FinalTravel = 0.5 * (Low + High);
        OutContact.bInContact = true;
    }

    FTAMultiLinkSolveInput SolveInput;
    SolveInput.TravelM = FinalTravel;
    SolveInput.Damage = DamageOffsets;
    if (!TAMultiLinkSolver::Solve(GeometryConfig, SolveInput, InOutGeometryState, OutContact.Geometry)) return false;

    const double PreviousTravel = InOutSuspensionState.TravelM;
    const bool bHadTravel = InOutSuspensionState.bTravelInitialized;
    InOutSuspensionState.TravelM = FinalTravel;
    InOutSuspensionState.TravelVelocityMps = bHadTravel ? (FinalTravel - PreviousTravel) / DeltaTimeSeconds : 0.0;
    InOutSuspensionState.bTravelInitialized = true;
    InOutSuspensionState.WheelCenterOffsetM = OutContact.Geometry.WheelCenterLocalM - GeometryConfig.WheelCenterReference;
    InOutSuspensionState.CamberRad = OutContact.Geometry.CamberRad;
    InOutSuspensionState.ToeRad = OutContact.Geometry.ToeRad;
    InOutSuspensionState.MotionRatio = EstimateMotionRatio(GeometryConfig, DamageOffsets, FinalTravel, OutContact.Geometry.DamperLengthM);
    InOutSuspensionState.bKinematicCacheValid = false;

    OutContact.TravelM = FinalTravel;
    OutContact.WheelCenterWorldM = Chassis.PositionWorldM + Chassis.OrientationWorld.RotateVector(OutContact.Geometry.WheelCenterLocalM);
    OutContact.ContactPointWorldM = OutContact.WheelCenterWorldM - RoadNormal * WheelRadiusM;

    FVector3d Forward = Chassis.OrientationWorld.RotateVector(OutContact.Geometry.WheelForwardLocal);
    Forward -= RoadNormal * FVector3d::DotProduct(Forward, RoadNormal);
    Forward = Forward.GetSafeNormal();
    if (Forward.IsNearlyZero()) return false;
    const FVector3d Right = FVector3d::CrossProduct(RoadNormal, Forward).GetSafeNormal();
    OutContact.ForwardTangentWorld = Forward;
    OutContact.RightTangentWorld = Right;

    const FVector3d PatchVelocity = TAWheelContactResolver::CalculatePointVelocityWorld(Chassis, OutContact.ContactPointWorldM);
    OutContact.LongitudinalVelocityMps = FVector3d::DotProduct(PatchVelocity, Forward);
    OutContact.LateralVelocityMps = FVector3d::DotProduct(PatchVelocity, Right);

    OutContact.PenetrationM = FMath::Max(OutContact.PenetrationM,
        -FVector3d::DotProduct(OutContact.ContactPointWorldM - Road.PointWorldM, RoadNormal));

    if (OutContact.bInContact)
    {
        OutContact.SuspensionForce = TASuspensionRuntime::CalculateForce(SuspensionConfig, InOutSuspensionState);
        OutContact.VerticalLoadN = FMath::Max(0.0, OutContact.SuspensionForce.TotalForceN);
        OutContact.SuspensionForceWorldN = RoadNormal * OutContact.VerticalLoadN;
    }
    return true;
}

void TAMultiLinkContactResolver::ApplyAntiRollBarToPair(
    const FTAAntiRollBarConfig& Config,
    FTAResolvedMultiLinkContact& Left,
    FTAResolvedMultiLinkContact& Right)
{
    const FTAAntiRollBarOutput Adjustment = TASuspensionRuntime::CalculateAntiRollBar(Config, Left.TravelM, Right.TravelM);
    if (Left.bInContact)
    {
        Left.VerticalLoadN = FMath::Max(0.0, Left.VerticalLoadN + Adjustment.LeftLoadAdjustmentN);
        Left.SuspensionForceWorldN = Left.RoadNormalWorld * Left.VerticalLoadN;
    }
    else { Left.VerticalLoadN = 0.0; Left.SuspensionForceWorldN = FVector3d::ZeroVector; }

    if (Right.bInContact)
    {
        Right.VerticalLoadN = FMath::Max(0.0, Right.VerticalLoadN + Adjustment.RightLoadAdjustmentN);
        Right.SuspensionForceWorldN = Right.RoadNormalWorld * Right.VerticalLoadN;
    }
    else { Right.VerticalLoadN = 0.0; Right.SuspensionForceWorldN = FVector3d::ZeroVector; }
}

FTAWheelContactInput TAMultiLinkContactResolver::BuildVehicleWheelContactInput(const FTAResolvedMultiLinkContact& Contact)
{
    FTAWheelContactInput Input;
    Input.VerticalLoadN = Contact.VerticalLoadN;
    Input.LongitudinalVelocityMps = Contact.LongitudinalVelocityMps;
    Input.LateralVelocityMps = Contact.LateralVelocityMps;
    Input.CamberRad = Contact.Geometry.CamberRad;
    Input.ContactPointWorldM = Contact.ContactPointWorldM;
    Input.ForwardDirectionWorld = Contact.ForwardTangentWorld;
    Input.RightDirectionWorld = Contact.RightTangentWorld;
    Input.SuspensionForceWorldN = Contact.SuspensionForceWorldN;
    Input.Surface = Contact.Surface;
    return Input;
}
