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

    if (DeltaTimeSeconds <= 0.0 ||
        WheelRadiusM <= UE_DOUBLE_SMALL_NUMBER ||
        !TADoubleWishboneSolver::ValidateConfig(GeometryConfig))
    {
        return false;
    }

    const FVector3d RoadNormal =
        Road.NormalWorld.GetSafeNormal();

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
        !HasMeaningfulDamage(DamageOffsets);

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

    return Input;
}
