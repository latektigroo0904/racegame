#include "TAWheelContactResolver.h"

namespace
{
    struct FTravelEvaluation
    {
        bool bValid = false;

        double TravelM = 0.0;
        double SignedContactDistanceM = 0.0;

        FTADoubleWishboneSolveOutput Geometry;
    };

    bool HasMeaningfulDamage(
        const FTADoubleWishboneDamageOffsets& Damage)
    {
        constexpr double ThresholdSquared = 1.0e-10;

        return
            Damage.UpperInnerA.SquaredLength() > ThresholdSquared
            || Damage.UpperInnerB.SquaredLength() > ThresholdSquared
            || Damage.LowerInnerA.SquaredLength() > ThresholdSquared
            || Damage.LowerInnerB.SquaredLength() > ThresholdSquared
            || Damage.TieRodInner.SquaredLength() > ThresholdSquared
            || Damage.DamperChassis.SquaredLength() > ThresholdSquared;
    }

    bool EvaluateTravel(
        const FTAChassisState& Chassis,
        const FTADoubleWishboneSolverConfig& GeometryConfig,
        const double WheelRadiusM,
        const double RackDisplacementM,
        const FTADoubleWishboneDamageOffsets& DamageOffsets,
        const FVector3d& RoadPointWorldM,
        const FVector3d& RoadNormalWorld,
        const double TravelM,
        FTravelEvaluation& OutEvaluation)
    {
        FTADoubleWishboneState ScratchState;
        FTADoubleWishboneSolveOutput GeometryOutput;
        FTADoubleWishboneSolveInput SolveInput;

        SolveInput.TravelM = TravelM;
        SolveInput.RackDisplacementM = RackDisplacementM;
        SolveInput.Damage = DamageOffsets;

        if (!TADoubleWishboneSolver::Solve(
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
                ContactPointWorldM - RoadPointWorldM,
                RoadNormalWorld);

        return true;
    }

    double EstimateMotionRatio(
        const FTADoubleWishboneSolverConfig& GeometryConfig,
        const double RackDisplacementM,
        const FTADoubleWishboneDamageOffsets& DamageOffsets,
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

        FTADoubleWishboneState ProbeState;
        FTADoubleWishboneSolveOutput ProbeOutput;
        FTADoubleWishboneSolveInput ProbeInput;

        ProbeInput.TravelM = ProbeTravelM;
        ProbeInput.RackDisplacementM = RackDisplacementM;
        ProbeInput.Damage = DamageOffsets;

        if (!TADoubleWishboneSolver::Solve(
                GeometryConfig,
                ProbeInput,
                ProbeState,
                ProbeOutput))
        {
            return 1.0;
        }

        const double Ratio =
            FMath::Abs(
                (ProbeOutput.DamperLengthM - CurrentDamperLengthM)
                / TravelDeltaM);

        return FMath::Clamp(Ratio, 0.05, 3.0);
    }

    struct FCompliantTravelEvaluation
    {
        bool bValid = false;

        double TravelM = 0.0;
        double SignedContactDistanceM = 0.0;
        double ForceResidualN = 0.0;

        FTADoubleWishboneSolveOutput Geometry;

        FTASuspensionRuntimeState SuspensionState;
        FTASuspensionForceOutput SuspensionForce;

        FTATireVerticalForceOutput TireVertical;
    };

    bool EvaluateCompliantTravel(
        const FTAChassisState& Chassis,
        const FTADoubleWishboneSolverConfig& GeometryConfig,
        const FTASuspensionRuntimeConfig& SuspensionConfig,
        const FTATireRuntimeConfig& TireConfig,
        const FTASuspensionRuntimeState& PreviousSuspensionState,
        const FTATireRuntimeState& TireState,
        const double RackDisplacementM,
        const FTADoubleWishboneDamageOffsets& DamageOffsets,
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
                RackDisplacementM,
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
                RackDisplacementM,
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
        const FTADoubleWishboneSolveOutput& Geometry,
        const FVector3d& RoadNormalWorld,
        const double WheelRadiusM,
        FTAResolvedWheelContact& OutContact)
    {
        OutContact.WheelCenterWorldM =
            Chassis.PositionWorldM
            + Chassis.OrientationWorld.RotateVector(
                Geometry.WheelCenterLocalM);

        OutContact.ContactPointWorldM =
            OutContact.WheelCenterWorldM
            - RoadNormalWorld * WheelRadiusM;

        FVector3d ForwardWorld =
            Chassis.OrientationWorld.RotateVector(
                Geometry.WheelForwardLocal);

        ForwardWorld -=
            RoadNormalWorld
            * FVector3d::DotProduct(
                ForwardWorld,
                RoadNormalWorld);

        ForwardWorld = ForwardWorld.GetSafeNormal();

        if (ForwardWorld.IsNearlyZero())
        {
            ForwardWorld =
                Chassis.OrientationWorld.RotateVector(
                    FVector3d(1.0, 0.0, 0.0));

            ForwardWorld -=
                RoadNormalWorld
                * FVector3d::DotProduct(
                    ForwardWorld,
                    RoadNormalWorld);

            ForwardWorld = ForwardWorld.GetSafeNormal();
        }

        const FVector3d RightWorld =
            FVector3d::CrossProduct(
                RoadNormalWorld,
                ForwardWorld).GetSafeNormal();

        OutContact.ForwardTangentWorld = ForwardWorld;
        OutContact.RightTangentWorld = RightWorld;

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

FVector3d TAWheelContactResolver::CalculatePointVelocityWorld(
    const FTAChassisState& Chassis,
    const FVector3d& PointWorldM)
{
    const FVector3d LeverArmM =
        PointWorldM - Chassis.PositionWorldM;

    return
        Chassis.LinearVelocityWorldMps
        + FVector3d::CrossProduct(
            Chassis.AngularVelocityWorldRadPerSec,
            LeverArmM);
}

bool TAWheelContactResolver::ResolveDoubleWishboneRoadContact(
    const FTAChassisState& Chassis,
    const FTADoubleWishboneSolverConfig& GeometryConfig,
    const FTASuspensionRuntimeConfig& SuspensionConfig,
    const double WheelRadiusM,
    const double RackDisplacementM,
    const FTADoubleWishboneDamageOffsets& DamageOffsets,
    const FTARoadPlane& Road,
    const double DeltaTimeSeconds,
    FTADoubleWishboneState& InOutGeometryState,
    FTASuspensionRuntimeState& InOutSuspensionState,
    FTAResolvedWheelContact& OutContact)
{
    OutContact = FTAResolvedWheelContact{};
    OutContact.Surface = Road.Surface;

    if (DeltaTimeSeconds <= 0.0 ||
        WheelRadiusM <= UE_DOUBLE_SMALL_NUMBER ||
        !TADoubleWishboneSolver::ValidateConfig(GeometryConfig))
    {
        return false;
    }

    const FVector3d RoadNormal =
        Road.NormalWorld.GetSafeNormal();

    OutContact.RoadNormalWorld = RoadNormal;

    if (RoadNormal.IsNearlyZero())
    {
        return false;
    }

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
            RackDisplacementM,
            DamageOffsets,
            Road.PointWorldM,
            RoadNormal,
            GeometryConfig.MinTravelM,
            DroopEvaluation) ||
        !EvaluateTravel(
            Chassis,
            GeometryConfig,
            WheelRadiusM,
            RackDisplacementM,
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
        FinalTravelM = GeometryConfig.MinTravelM;
        OutContact.bInContact = false;
        OutContact.bTravelClamped = true;
    }
    else if (BumpEvaluation.SignedContactDistanceM < 0.0)
    {
        FinalTravelM = GeometryConfig.MaxTravelM;
        OutContact.bInContact = true;
        OutContact.bTravelClamped = true;
        OutContact.PenetrationM =
            -BumpEvaluation.SignedContactDistanceM;
    }
    else
    {
        double LowTravelM = GeometryConfig.MinTravelM;
        double HighTravelM = GeometryConfig.MaxTravelM;

        FTravelEvaluation MidEvaluation;

        for (int32 Iteration = 0; Iteration < 24; ++Iteration)
        {
            const double MidTravelM =
                0.5 * (LowTravelM + HighTravelM);

            if (!EvaluateTravel(
                    Chassis,
                    GeometryConfig,
                    WheelRadiusM,
                    RackDisplacementM,
                    DamageOffsets,
                    Road.PointWorldM,
                    RoadNormal,
                    MidTravelM,
                    MidEvaluation))
            {
                return false;
            }

            if (FMath::Abs(
                    MidEvaluation.SignedContactDistanceM) <= 1.0e-5)
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
            0.5 * (LowTravelM + HighTravelM);

        OutContact.bInContact = true;
    }

    FTADoubleWishboneSolveInput FinalSolveInput;
    FinalSolveInput.TravelM = FinalTravelM;
    FinalSolveInput.RackDisplacementM = RackDisplacementM;
    FinalSolveInput.Damage = DamageOffsets;

    FTADoubleWishboneSolveOutput FinalGeometry;

    if (!TADoubleWishboneSolver::Solve(
            GeometryConfig,
            FinalSolveInput,
            InOutGeometryState,
            FinalGeometry))
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

    InOutSuspensionState.bTravelInitialized = true;

    InOutSuspensionState.WheelCenterOffsetM =
        FinalGeometry.WheelCenterLocalM
        - GeometryConfig.Hardpoints.WheelCenterReference;

    InOutSuspensionState.CamberRad =
        FinalGeometry.CamberRad;

    InOutSuspensionState.ToeRad =
        FinalGeometry.ToeRad;

    InOutSuspensionState.MotionRatio =
        EstimateMotionRatio(
            GeometryConfig,
            RackDisplacementM,
            DamageOffsets,
            FinalTravelM,
            FinalGeometry.DamperLengthM);

    InOutSuspensionState.bKinematicCacheValid =
        !HasMeaningfulDamage(DamageOffsets)
        && FMath::Abs(RackDisplacementM) <= 1.0e-8;

    OutContact.TravelM = FinalTravelM;
    OutContact.Geometry = FinalGeometry;

    PopulateWorldKinematics(
        Chassis,
        FinalGeometry,
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

bool TAWheelContactResolver::ResolveDoubleWishboneCompliantRoadContact(
    const FTAChassisState& Chassis,
    const FTADoubleWishboneSolverConfig& GeometryConfig,
    const FTASuspensionRuntimeConfig& SuspensionConfig,
    const FTATireRuntimeConfig& TireConfig,
    const double RackDisplacementM,
    const FTADoubleWishboneDamageOffsets& DamageOffsets,
    const FTARoadPlane& Road,
    const double DeltaTimeSeconds,
    FTADoubleWishboneState& InOutGeometryState,
    FTASuspensionRuntimeState& InOutSuspensionState,
    FTATireRuntimeState& InOutTireState,
    FTAResolvedWheelContact& OutContact)
{
    OutContact = FTAResolvedWheelContact{};
    OutContact.Surface = Road.Surface;

    if (DeltaTimeSeconds <= 0.0 ||
        TireConfig.UnloadedRadiusM <= UE_DOUBLE_SMALL_NUMBER ||
        !TADoubleWishboneSolver::ValidateConfig(
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
            RackDisplacementM,
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
            RackDisplacementM,
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
        FinalEvaluation = LowEvaluation;
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
                    RackDisplacementM,
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

    FTADoubleWishboneSolveInput FinalSolveInput;
    FinalSolveInput.TravelM =
        FinalEvaluation.TravelM;
    FinalSolveInput.RackDisplacementM =
        RackDisplacementM;
    FinalSolveInput.Damage =
        DamageOffsets;

    FTADoubleWishboneSolveOutput FinalGeometry;

    if (!TADoubleWishboneSolver::Solve(
            GeometryConfig,
            FinalSolveInput,
            InOutGeometryState,
            FinalGeometry))
    {
        return false;
    }

    InOutSuspensionState =
        FinalEvaluation.SuspensionState;

    InOutSuspensionState.bTravelInitialized =
        true;

    InOutSuspensionState.WheelCenterOffsetM =
        FinalGeometry.WheelCenterLocalM
        - GeometryConfig.Hardpoints.WheelCenterReference;

    InOutSuspensionState.CamberRad =
        FinalGeometry.CamberRad;

    InOutSuspensionState.ToeRad =
        FinalGeometry.ToeRad;

    InOutSuspensionState.bKinematicCacheValid =
        !HasMeaningfulDamage(DamageOffsets)
        && FMath::Abs(RackDisplacementM)
            <= 1.0e-8;

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

    OutContact.Geometry =
        FinalGeometry;

    OutContact.SuspensionForce =
        FinalEvaluation.SuspensionForce;

    PopulateWorldKinematics(
        Chassis,
        FinalGeometry,
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
        CalculatePointVelocityWorld(
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

void TAWheelContactResolver::ApplyAntiRollBarToPair(
    const FTAAntiRollBarConfig& Config,
    FTAResolvedWheelContact& InOutLeftContact,
    FTAResolvedWheelContact& InOutRightContact)
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

FTAWheelContactInput TAWheelContactResolver::BuildVehicleWheelContactInput(
    const FTAResolvedWheelContact& Contact)
{
    FTAWheelContactInput Input;

    Input.VerticalLoadN = Contact.VerticalLoadN;
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

    Input.Surface = Contact.Surface;

    return Input;
}
