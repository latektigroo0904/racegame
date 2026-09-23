#include "TAMultiLinkContactResolver.h"

namespace
{
    struct FTravelEvaluation
    {
        bool bValid = false;

        double TravelM = 0.0;
        double SignedContactDistanceM = 0.0;

        FTAMultiLinkSolveOutput Geometry;
    };

    struct FCompliantTravelEvaluation
    {
        bool bValid = false;

        double TravelM = 0.0;
        double SignedContactDistanceM = 0.0;
        double ForceResidualN = 0.0;

        FTAMultiLinkSolveOutput Geometry;

        FTASuspensionRuntimeState SuspensionState;
        FTASuspensionForceOutput SuspensionForce;

        FTATireVerticalForceOutput TireVertical;
    };

    bool HasMeaningfulDamage(
        const FTAMultiLinkDamageOffsets& Damage)
    {
        constexpr double ThresholdSquared = 1.0e-10;

        for (int32 Index = 0;
             Index < TARearMultiLinkCount;
             ++Index)
        {
            if (Damage.ChassisPickupOffsets[Index].SquaredLength()
                > ThresholdSquared)
            {
                return true;
            }
        }

        return
            Damage.DamperChassisOffset.SquaredLength()
            > ThresholdSquared;
    }

    bool EvaluateTravel(
        const FTAChassisState& Chassis,
        const FTAMultiLinkSolverConfig& GeometryConfig,
        const double WheelRadiusM,
        const FTAMultiLinkDamageOffsets& DamageOffsets,
        const FVector3d& RoadPointWorldM,
        const FVector3d& RoadNormalWorld,
        const double TravelM,
        FTravelEvaluation& OutEvaluation)
    {
        FTAMultiLinkRuntimeState ScratchState;
        FTAMultiLinkSolveOutput GeometryOutput;
        FTAMultiLinkSolveInput SolveInput;

        SolveInput.TravelM = TravelM;
        SolveInput.Damage = DamageOffsets;

        if (!TAMultiLinkSolver::Solve(
                GeometryConfig,
                SolveInput,
                ScratchState,
                GeometryOutput))
        {
            return false;
        }

        const FVector3d WheelCenterWorldM =
            Chassis.PositionWorldM
            + Chassis.OrientationWorld.RotateVector(
                GeometryOutput.WheelCenterLocalM);

        const FVector3d ContactPointWorldM =
            WheelCenterWorldM
            - RoadNormalWorld * WheelRadiusM;

        OutEvaluation.bValid = true;
        OutEvaluation.TravelM = TravelM;
        OutEvaluation.Geometry = GeometryOutput;

        OutEvaluation.SignedContactDistanceM =
            FVector3d::DotProduct(
                ContactPointWorldM
                    - RoadPointWorldM,
                RoadNormalWorld);

        return true;
    }

    double EstimateMotionRatio(
        const FTAMultiLinkSolverConfig& GeometryConfig,
        const FTAMultiLinkDamageOffsets& DamageOffsets,
        const double CurrentTravelM,
        const double CurrentDamperLengthM)
    {
        constexpr double ProbeDistanceM = 0.001;

        double ProbeTravelM =
            FMath::Min(
                GeometryConfig.MaxTravelM,
                CurrentTravelM + ProbeDistanceM);

        if (FMath::IsNearlyEqual(
                ProbeTravelM,
                CurrentTravelM,
                1.0e-9))
        {
            ProbeTravelM =
                FMath::Max(
                    GeometryConfig.MinTravelM,
                    CurrentTravelM - ProbeDistanceM);
        }

        const double TravelDeltaM =
            ProbeTravelM - CurrentTravelM;

        if (FMath::Abs(TravelDeltaM) <= 1.0e-9)
        {
            return 1.0;
        }

        FTAMultiLinkRuntimeState ProbeState;
        FTAMultiLinkSolveOutput ProbeOutput;
        FTAMultiLinkSolveInput ProbeInput;

        ProbeInput.TravelM = ProbeTravelM;
        ProbeInput.Damage = DamageOffsets;

        if (!TAMultiLinkSolver::Solve(
                GeometryConfig,
                ProbeInput,
                ProbeState,
                ProbeOutput))
        {
            return 1.0;
        }

        const double Ratio =
            FMath::Abs(
                (ProbeOutput.DamperLengthM
                    - CurrentDamperLengthM)
                / TravelDeltaM);

        return FMath::Clamp(
            Ratio,
            0.05,
            3.0);
    }

    bool EvaluateCompliantTravel(
        const FTAChassisState& Chassis,
        const FTAMultiLinkSolverConfig& GeometryConfig,
        const FTASuspensionRuntimeConfig& SuspensionConfig,
        const FTATireRuntimeConfig& TireConfig,
        const FTASuspensionRuntimeState& PreviousSuspensionState,
        const FTATireRuntimeState& TireState,
        const FTAMultiLinkDamageOffsets& DamageOffsets,
        const FVector3d& RoadPointWorldM,
        const FVector3d& RoadNormalWorld,
        const double TravelM,
        const double DeltaTimeSeconds,
        FCompliantTravelEvaluation& OutEvaluation)
    {
        FTravelEvaluation GeometryEvaluation;

        if (!EvaluateTravel(
                Chassis,
                GeometryConfig,
                TireConfig.UnloadedRadiusM,
                DamageOffsets,
                RoadPointWorldM,
                RoadNormalWorld,
                TravelM,
                GeometryEvaluation))
        {
            return false;
        }

        OutEvaluation = FCompliantTravelEvaluation{};

        OutEvaluation.bValid = true;
        OutEvaluation.TravelM = TravelM;

        OutEvaluation.SignedContactDistanceM =
            GeometryEvaluation.SignedContactDistanceM;

        OutEvaluation.Geometry =
            GeometryEvaluation.Geometry;

        OutEvaluation.SuspensionState =
            PreviousSuspensionState;

        OutEvaluation.SuspensionState.TravelM =
            TravelM;

        OutEvaluation.SuspensionState.TravelVelocityMps =
            PreviousSuspensionState.bTravelInitialized
            ? (TravelM - PreviousSuspensionState.TravelM)
                / DeltaTimeSeconds
            : 0.0;

        OutEvaluation.SuspensionState.MotionRatio =
            EstimateMotionRatio(
                GeometryConfig,
                DamageOffsets,
                TravelM,
                GeometryEvaluation.Geometry.DamperLengthM);

        OutEvaluation.SuspensionForce =
            TASuspensionRuntime::CalculateForce(
                SuspensionConfig,
                OutEvaluation.SuspensionState);

        const double RequestedDeflectionM =
            FMath::Max(
                0.0,
                -GeometryEvaluation.SignedContactDistanceM);

        const double DeflectionVelocityMps =
            TireState.bRadialStateInitialized
            ? (RequestedDeflectionM
                - TireState.RadialDeflectionM)
                / DeltaTimeSeconds
            : 0.0;

        OutEvaluation.TireVertical =
            TATireSolver::CalculateVerticalForce(
                TireConfig,
                TireState,
                RequestedDeflectionM,
                DeflectionVelocityMps);

        OutEvaluation.ForceResidualN =
            OutEvaluation.TireVertical.NormalForceN
            - FMath::Max(
                0.0,
                OutEvaluation.SuspensionForce.TotalForceN);

        return true;
    }

    void PopulateWorldKinematics(
        const FTAChassisState& Chassis,
        const FTAMultiLinkSolveOutput& Geometry,
        const FVector3d& RoadNormalWorld,
        const double RadiusM,
        FTAResolvedMultiLinkContact& OutContact)
    {
        OutContact.WheelCenterWorldM =
            Chassis.PositionWorldM
            + Chassis.OrientationWorld.RotateVector(
                Geometry.WheelCenterLocalM);

        OutContact.ContactPointWorldM =
            OutContact.WheelCenterWorldM
            - RoadNormalWorld * RadiusM;

        FVector3d ForwardWorld =
            Chassis.OrientationWorld.RotateVector(
                Geometry.WheelForwardLocal);

        ForwardWorld -=
            RoadNormalWorld
            * FVector3d::DotProduct(
                ForwardWorld,
                RoadNormalWorld);

        ForwardWorld =
            ForwardWorld.GetSafeNormal();

        if (ForwardWorld.IsNearlyZero())
        {
            return;
        }

        const FVector3d RightWorld =
            FVector3d::CrossProduct(
                RoadNormalWorld,
                ForwardWorld).GetSafeNormal();

        OutContact.ForwardTangentWorld =
            ForwardWorld;

        OutContact.RightTangentWorld =
            RightWorld;

        const FVector3d ContactVelocityWorldMps =
            TAWheelContactResolver::CalculatePointVelocityWorld(
                Chassis,
                OutContact.ContactPointWorldM);

        OutContact.LongitudinalVelocityMps =
            FVector3d::DotProduct(
                ContactVelocityWorldMps,
                ForwardWorld);

        OutContact.LateralVelocityMps =
            FVector3d::DotProduct(
                ContactVelocityWorldMps,
                RightWorld);
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

    if (DeltaTimeSeconds <= 0.0 ||
        WheelRadiusM <= UE_DOUBLE_SMALL_NUMBER ||
        !TAMultiLinkSolver::ValidateConfig(
            GeometryConfig))
    {
        return false;
    }

    const FVector3d RoadNormal =
        Road.NormalWorld.GetSafeNormal();

    if (RoadNormal.IsNearlyZero())
    {
        return false;
    }

    OutContact.RoadNormalWorld =
        RoadNormal;

    const FVector3d ChassisUp =
        Chassis.OrientationWorld.RotateVector(
            FVector3d(0.0, 0.0, 1.0)).GetSafeNormal();

    if (FVector3d::DotProduct(
            ChassisUp,
            RoadNormal) < 0.20)
    {
        return false;
    }

    FTravelEvaluation DroopEvaluation;
    FTravelEvaluation BumpEvaluation;

    if (!EvaluateTravel(
            Chassis,
            GeometryConfig,
            WheelRadiusM,
            DamageOffsets,
            Road.PointWorldM,
            RoadNormal,
            GeometryConfig.MinTravelM,
            DroopEvaluation) ||
        !EvaluateTravel(
            Chassis,
            GeometryConfig,
            WheelRadiusM,
            DamageOffsets,
            Road.PointWorldM,
            RoadNormal,
            GeometryConfig.MaxTravelM,
            BumpEvaluation))
    {
        return false;
    }

    double FinalTravelM = 0.0;

    if (DroopEvaluation.SignedContactDistanceM > 0.0)
    {
        FinalTravelM =
            GeometryConfig.MinTravelM;

        OutContact.bInContact = false;
        OutContact.bTravelClamped = true;
    }
    else if (BumpEvaluation.SignedContactDistanceM < 0.0)
    {
        FinalTravelM =
            GeometryConfig.MaxTravelM;

        OutContact.bInContact = true;
        OutContact.bTravelClamped = true;

        OutContact.PenetrationM =
            -BumpEvaluation.SignedContactDistanceM;
    }
    else
    {
        double LowTravelM =
            GeometryConfig.MinTravelM;

        double HighTravelM =
            GeometryConfig.MaxTravelM;

        for (int32 Iteration = 0;
             Iteration < 24;
             ++Iteration)
        {
            const double MidTravelM =
                0.5
                * (LowTravelM + HighTravelM);

            FTravelEvaluation MidEvaluation;

            if (!EvaluateTravel(
                    Chassis,
                    GeometryConfig,
                    WheelRadiusM,
                    DamageOffsets,
                    Road.PointWorldM,
                    RoadNormal,
                    MidTravelM,
                    MidEvaluation))
            {
                return false;
            }

            if (FMath::Abs(
                    MidEvaluation.SignedContactDistanceM)
                <= 1.0e-5)
            {
                LowTravelM = MidTravelM;
                HighTravelM = MidTravelM;
                break;
            }

            if (MidEvaluation.SignedContactDistanceM < 0.0)
            {
                LowTravelM = MidTravelM;
            }
            else
            {
                HighTravelM = MidTravelM;
            }
        }

        FinalTravelM =
            0.5
            * (LowTravelM + HighTravelM);

        OutContact.bInContact = true;
    }

    FTAMultiLinkSolveInput FinalInput;
    FinalInput.TravelM = FinalTravelM;
    FinalInput.Damage = DamageOffsets;

    if (!TAMultiLinkSolver::Solve(
            GeometryConfig,
            FinalInput,
            InOutGeometryState,
            OutContact.Geometry))
    {
        return false;
    }

    const double PreviousTravelM =
        InOutSuspensionState.TravelM;

    const bool bHadTravel =
        InOutSuspensionState.bTravelInitialized;

    InOutSuspensionState.TravelM =
        FinalTravelM;

    InOutSuspensionState.TravelVelocityMps =
        bHadTravel
        ? (FinalTravelM - PreviousTravelM)
            / DeltaTimeSeconds
        : 0.0;

    InOutSuspensionState.bTravelInitialized =
        true;

    InOutSuspensionState.WheelCenterOffsetM =
        OutContact.Geometry.WheelCenterLocalM
        - GeometryConfig.WheelCenterReference;

    InOutSuspensionState.CamberRad =
        OutContact.Geometry.CamberRad;

    InOutSuspensionState.ToeRad =
        OutContact.Geometry.ToeRad;

    InOutSuspensionState.MotionRatio =
        EstimateMotionRatio(
            GeometryConfig,
            DamageOffsets,
            FinalTravelM,
            OutContact.Geometry.DamperLengthM);

    InOutSuspensionState.bKinematicCacheValid =
        !HasMeaningfulDamage(
            DamageOffsets);

    OutContact.TravelM =
        FinalTravelM;

    PopulateWorldKinematics(
        Chassis,
        OutContact.Geometry,
        RoadNormal,
        WheelRadiusM,
        OutContact);

    OutContact.PenetrationM =
        FMath::Max(
            OutContact.PenetrationM,
            -FVector3d::DotProduct(
                OutContact.ContactPointWorldM
                    - Road.PointWorldM,
                RoadNormal));

    if (OutContact.bInContact)
    {
        OutContact.SuspensionForce =
            TASuspensionRuntime::CalculateForce(
                SuspensionConfig,
                InOutSuspensionState);

        OutContact.VerticalLoadN =
            FMath::Max(
                0.0,
                OutContact.SuspensionForce.TotalForceN);

        OutContact.SuspensionForceWorldN =
            RoadNormal
            * OutContact.VerticalLoadN;
    }

    return true;
}

bool TAMultiLinkContactResolver::ResolveCompliantRoadContact(
    const FTAChassisState& Chassis,
    const FTAMultiLinkSolverConfig& GeometryConfig,
    const FTASuspensionRuntimeConfig& SuspensionConfig,
    const FTATireRuntimeConfig& TireConfig,
    const FTAMultiLinkDamageOffsets& DamageOffsets,
    const FTARoadPlane& Road,
    const double DeltaTimeSeconds,
    FTAMultiLinkRuntimeState& InOutGeometryState,
    FTASuspensionRuntimeState& InOutSuspensionState,
    FTATireRuntimeState& InOutTireState,
    FTAResolvedMultiLinkContact& OutContact)
{
    OutContact = FTAResolvedMultiLinkContact{};
    OutContact.Surface = Road.Surface;

    if (DeltaTimeSeconds <= 0.0 ||
        TireConfig.UnloadedRadiusM <= UE_DOUBLE_SMALL_NUMBER ||
        !TAMultiLinkSolver::ValidateConfig(
            GeometryConfig))
    {
        return false;
    }

    const FVector3d RoadNormal =
        Road.NormalWorld.GetSafeNormal();

    if (RoadNormal.IsNearlyZero())
    {
        return false;
    }

    OutContact.RoadNormalWorld =
        RoadNormal;

    const FVector3d ChassisUp =
        Chassis.OrientationWorld.RotateVector(
            FVector3d(0.0, 0.0, 1.0)).GetSafeNormal();

    if (FVector3d::DotProduct(
            ChassisUp,
            RoadNormal) < 0.20)
    {
        return false;
    }

    FCompliantTravelEvaluation LowEvaluation;
    FCompliantTravelEvaluation HighEvaluation;

    if (!EvaluateCompliantTravel(
            Chassis,
            GeometryConfig,
            SuspensionConfig,
            TireConfig,
            InOutSuspensionState,
            InOutTireState,
            DamageOffsets,
            Road.PointWorldM,
            RoadNormal,
            GeometryConfig.MinTravelM,
            DeltaTimeSeconds,
            LowEvaluation) ||
        !EvaluateCompliantTravel(
            Chassis,
            GeometryConfig,
            SuspensionConfig,
            TireConfig,
            InOutSuspensionState,
            InOutTireState,
            DamageOffsets,
            Road.PointWorldM,
            RoadNormal,
            GeometryConfig.MaxTravelM,
            DeltaTimeSeconds,
            HighEvaluation))
    {
        return false;
    }

    FCompliantTravelEvaluation FinalEvaluation;
    bool bAirborne = false;
    bool bTravelClamped = false;

    if (LowEvaluation.SignedContactDistanceM > 0.0)
    {
        FinalEvaluation =
            LowEvaluation;

        bAirborne = true;
        bTravelClamped = true;
    }
    else if (
        LowEvaluation.ForceResidualN
        * HighEvaluation.ForceResidualN <= 0.0)
    {
        FCompliantTravelEvaluation Low =
            LowEvaluation;

        FCompliantTravelEvaluation High =
            HighEvaluation;

        for (int32 Iteration = 0;
             Iteration < 28;
             ++Iteration)
        {
            const double MidTravelM =
                0.5
                * (Low.TravelM + High.TravelM);

            FCompliantTravelEvaluation Mid;

            if (!EvaluateCompliantTravel(
                    Chassis,
                    GeometryConfig,
                    SuspensionConfig,
                    TireConfig,
                    InOutSuspensionState,
                    InOutTireState,
                    DamageOffsets,
                    Road.PointWorldM,
                    RoadNormal,
                    MidTravelM,
                    DeltaTimeSeconds,
                    Mid))
            {
                return false;
            }

            if (FMath::Abs(Mid.ForceResidualN) <= 2.0 ||
                FMath::Abs(
                    High.TravelM - Low.TravelM)
                    <= 1.0e-6)
            {
                FinalEvaluation = Mid;
                break;
            }

            if (Low.ForceResidualN
                * Mid.ForceResidualN <= 0.0)
            {
                High = Mid;
            }
            else
            {
                Low = Mid;
            }

            FinalEvaluation =
                FMath::Abs(Low.ForceResidualN)
                    < FMath::Abs(High.ForceResidualN)
                ? Low
                : High;
        }
    }
    else
    {
        bTravelClamped = true;

        FinalEvaluation =
            FMath::Abs(LowEvaluation.ForceResidualN)
                < FMath::Abs(HighEvaluation.ForceResidualN)
            ? LowEvaluation
            : HighEvaluation;
    }

    FTAMultiLinkSolveInput FinalInput;
    FinalInput.TravelM =
        FinalEvaluation.TravelM;
    FinalInput.Damage =
        DamageOffsets;

    if (!TAMultiLinkSolver::Solve(
            GeometryConfig,
            FinalInput,
            InOutGeometryState,
            OutContact.Geometry))
    {
        return false;
    }

    InOutSuspensionState =
        FinalEvaluation.SuspensionState;

    InOutSuspensionState.bTravelInitialized =
        true;

    InOutSuspensionState.WheelCenterOffsetM =
        OutContact.Geometry.WheelCenterLocalM
        - GeometryConfig.WheelCenterReference;

    InOutSuspensionState.CamberRad =
        OutContact.Geometry.CamberRad;

    InOutSuspensionState.ToeRad =
        OutContact.Geometry.ToeRad;

    InOutSuspensionState.bKinematicCacheValid =
        !HasMeaningfulDamage(
            DamageOffsets);

    TATireSolver::CommitVerticalState(
        FinalEvaluation.TireVertical,
        InOutTireState);

    OutContact.bInContact =
        !bAirborne
        && (FinalEvaluation.TireVertical.NormalForceN > 0.0
            || FinalEvaluation.TireVertical.EffectiveDeflectionM > 0.0);

    OutContact.bTravelClamped =
        bTravelClamped;

    OutContact.TravelM =
        FinalEvaluation.TravelM;

    OutContact.TireRadialDeflectionM =
        FinalEvaluation.TireVertical.EffectiveDeflectionM;

    OutContact.bTireBottomed =
        FinalEvaluation.TireVertical.bBottomed;

    OutContact.SuspensionForce =
        FinalEvaluation.SuspensionForce;

    PopulateWorldKinematics(
        Chassis,
        OutContact.Geometry,
        RoadNormal,
        TireConfig.UnloadedRadiusM,
        OutContact);

    const double LoadedRadiusM =
        FMath::Max(
            0.01,
            TireConfig.UnloadedRadiusM
            - OutContact.TireRadialDeflectionM);

    OutContact.ContactPointWorldM =
        OutContact.WheelCenterWorldM
        - RoadNormal * LoadedRadiusM;

    const FVector3d ContactVelocityWorldMps =
        TAWheelContactResolver::CalculatePointVelocityWorld(
            Chassis,
            OutContact.ContactPointWorldM);

    OutContact.LongitudinalVelocityMps =
        FVector3d::DotProduct(
            ContactVelocityWorldMps,
            OutContact.ForwardTangentWorld);

    OutContact.LateralVelocityMps =
        FVector3d::DotProduct(
            ContactVelocityWorldMps,
            OutContact.RightTangentWorld);

    OutContact.PenetrationM =
        FinalEvaluation.TireVertical.RequestedDeflectionM;

    if (OutContact.bInContact)
    {
        OutContact.VerticalLoadN =
            FinalEvaluation.TireVertical.NormalForceN;

        OutContact.SuspensionForceWorldN =
            RoadNormal
            * OutContact.VerticalLoadN;
    }

    return true;
}

void TAMultiLinkContactResolver::ApplyAntiRollBarToPair(
    const FTAAntiRollBarConfig& Config,
    FTAResolvedMultiLinkContact& InOutLeftContact,
    FTAResolvedMultiLinkContact& InOutRightContact)
{
    const FTAAntiRollBarOutput Adjustment =
        TASuspensionRuntime::CalculateAntiRollBar(
            Config,
            InOutLeftContact.TravelM,
            InOutRightContact.TravelM);

    if (InOutLeftContact.bInContact)
    {
        InOutLeftContact.VerticalLoadN =
            FMath::Max(
                0.0,
                InOutLeftContact.VerticalLoadN
                + Adjustment.LeftLoadAdjustmentN);

        InOutLeftContact.SuspensionForceWorldN =
            InOutLeftContact.RoadNormalWorld
            * InOutLeftContact.VerticalLoadN;
    }
    else
    {
        InOutLeftContact.VerticalLoadN = 0.0;
        InOutLeftContact.SuspensionForceWorldN =
            FVector3d::ZeroVector;
    }

    if (InOutRightContact.bInContact)
    {
        InOutRightContact.VerticalLoadN =
            FMath::Max(
                0.0,
                InOutRightContact.VerticalLoadN
                + Adjustment.RightLoadAdjustmentN);

        InOutRightContact.SuspensionForceWorldN =
            InOutRightContact.RoadNormalWorld
            * InOutRightContact.VerticalLoadN;
    }
    else
    {
        InOutRightContact.VerticalLoadN = 0.0;
        InOutRightContact.SuspensionForceWorldN =
            FVector3d::ZeroVector;
    }
}

FTAWheelContactInput TAMultiLinkContactResolver::BuildVehicleWheelContactInput(
    const FTAResolvedMultiLinkContact& Contact)
{
    FTAWheelContactInput Input;

    Input.VerticalLoadN =
        Contact.VerticalLoadN;

    Input.LongitudinalVelocityMps =
        Contact.LongitudinalVelocityMps;

    Input.LateralVelocityMps =
        Contact.LateralVelocityMps;

    Input.CamberRad =
        Contact.Geometry.CamberRad;

    Input.ContactPointWorldM =
        Contact.ContactPointWorldM;

    Input.ForwardDirectionWorld =
        Contact.ForwardTangentWorld;

    Input.RightDirectionWorld =
        Contact.RightTangentWorld;

    Input.SuspensionForceWorldN =
        Contact.SuspensionForceWorldN;

    Input.Surface =
        Contact.Surface;

    return Input;
}
