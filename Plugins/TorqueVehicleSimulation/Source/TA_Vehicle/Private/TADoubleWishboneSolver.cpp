#include "TADoubleWishboneSolver.h"

namespace
{
    constexpr int32 UIndex = 0;
    constexpr int32 LIndex = 1;
    constexpr int32 TIndex = 2;
    constexpr int32 WIndex = 3;

    double Distance(
        const FVector3d& A,
        const FVector3d& B)
    {
        return (B - A).Length();
    }

    bool IsFiniteVector(const FVector3d& V)
    {
        return
            FMath::IsFinite(V.X)
            && FMath::IsFinite(V.Y)
            && FMath::IsFinite(V.Z);
    }

    void ProjectPointToFixedDistance(
        FVector3d& Point,
        const FVector3d& Fixed,
        const double RestLengthM)
    {
        const FVector3d Delta = Point - Fixed;
        const double LengthM = Delta.Length();

        if (LengthM <= UE_DOUBLE_SMALL_NUMBER)
        {
            return;
        }

        Point =
            Fixed
            + Delta * (RestLengthM / LengthM);
    }

    void ProjectPairDistance(
        FVector3d& A,
        FVector3d& B,
        const double RestLengthM)
    {
        const FVector3d Delta = B - A;
        const double LengthM = Delta.Length();

        if (LengthM <= UE_DOUBLE_SMALL_NUMBER)
        {
            return;
        }

        const FVector3d Direction = Delta / LengthM;
        const double ErrorM = LengthM - RestLengthM;
        const FVector3d HalfCorrection = 0.5 * ErrorM * Direction;

        A += HalfCorrection;
        B -= HalfCorrection;
    }

    bool BuildBasis(
        const FVector3d& Upper,
        const FVector3d& Lower,
        const FVector3d& Tie,
        FVector3d& OutPrimary,
        FVector3d& OutSecondary,
        FVector3d& OutTertiary)
    {
        OutPrimary =
            (Upper - Lower).GetSafeNormal();

        if (OutPrimary.IsNearlyZero())
        {
            return false;
        }

        const FVector3d TieVector = Tie - Lower;

        const FVector3d SecondaryRaw =
            TieVector
            - OutPrimary
            * FVector3d::DotProduct(TieVector, OutPrimary);

        OutSecondary =
            SecondaryRaw.GetSafeNormal();

        if (OutSecondary.IsNearlyZero())
        {
            return false;
        }

        OutTertiary =
            FVector3d::CrossProduct(
                OutPrimary,
                OutSecondary).GetSafeNormal();

        return !OutTertiary.IsNearlyZero();
    }

    FVector3d MapReferenceVector(
        const FVector3d& ReferenceVector,
        const FVector3d& RefPrimary,
        const FVector3d& RefSecondary,
        const FVector3d& RefTertiary,
        const FVector3d& CurPrimary,
        const FVector3d& CurSecondary,
        const FVector3d& CurTertiary)
    {
        return
            CurPrimary
                * FVector3d::DotProduct(ReferenceVector, RefPrimary)
            + CurSecondary
                * FVector3d::DotProduct(ReferenceVector, RefSecondary)
            + CurTertiary
                * FVector3d::DotProduct(ReferenceVector, RefTertiary);
    }

    double MaxResidual(
        const FVector3d Points[4],
        const FVector3d& UpperInnerA,
        const FVector3d& UpperInnerB,
        const FVector3d& LowerInnerA,
        const FVector3d& LowerInnerB,
        const FVector3d& TieRodInner,
        const double TargetWheelZ,
        const double RestUpperA,
        const double RestUpperB,
        const double RestLowerA,
        const double RestLowerB,
        const double RestTie,
        const double RestUL,
        const double RestUT,
        const double RestLT,
        const double RestUW,
        const double RestLW,
        const double RestTW)
    {
        double MaxError = 0.0;

        auto AccumulateDistanceResidual =
            [&MaxError](
                const FVector3d& A,
                const FVector3d& B,
                const double Rest)
            {
                MaxError =
                    FMath::Max(
                        MaxError,
                        FMath::Abs(Distance(A, B) - Rest));
            };

        AccumulateDistanceResidual(
            Points[UIndex],
            UpperInnerA,
            RestUpperA);

        AccumulateDistanceResidual(
            Points[UIndex],
            UpperInnerB,
            RestUpperB);

        AccumulateDistanceResidual(
            Points[LIndex],
            LowerInnerA,
            RestLowerA);

        AccumulateDistanceResidual(
            Points[LIndex],
            LowerInnerB,
            RestLowerB);

        AccumulateDistanceResidual(
            Points[TIndex],
            TieRodInner,
            RestTie);

        AccumulateDistanceResidual(
            Points[UIndex],
            Points[LIndex],
            RestUL);

        AccumulateDistanceResidual(
            Points[UIndex],
            Points[TIndex],
            RestUT);

        AccumulateDistanceResidual(
            Points[LIndex],
            Points[TIndex],
            RestLT);

        AccumulateDistanceResidual(
            Points[UIndex],
            Points[WIndex],
            RestUW);

        AccumulateDistanceResidual(
            Points[LIndex],
            Points[WIndex],
            RestLW);

        AccumulateDistanceResidual(
            Points[TIndex],
            Points[WIndex],
            RestTW);

        MaxError =
            FMath::Max(
                MaxError,
                FMath::Abs(Points[WIndex].Z - TargetWheelZ));

        return MaxError;
    }
}

bool TADoubleWishboneSolver::ValidateConfig(
    const FTADoubleWishboneSolverConfig& Config)
{
    const FTADoubleWishboneHardpoints& H = Config.Hardpoints;

    if (Config.MaxIterations <= 0 ||
        Config.PositionToleranceM <= 0.0 ||
        Config.MaxTravelM <= Config.MinTravelM)
    {
        return false;
    }

    const FVector3d AllPoints[] =
    {
        H.UpperInnerA,
        H.UpperInnerB,
        H.LowerInnerA,
        H.LowerInnerB,
        H.TieRodInner,
        H.DamperChassis,
        H.DamperLowerArmReference,
        H.UpperBallJointReference,
        H.LowerBallJointReference,
        H.TieRodOuterReference,
        H.WheelCenterReference,
        H.WheelForwardReference,
        H.WheelUpReference,
        H.SteeringRackAxisLocal
    };

    for (const FVector3d& Point : AllPoints)
    {
        if (!IsFiniteVector(Point))
        {
            return false;
        }
    }

    if (Distance(H.UpperInnerA, H.UpperInnerB) <= 0.01 ||
        Distance(H.LowerInnerA, H.LowerInnerB) <= 0.01 ||
        Distance(H.DamperChassis, H.DamperLowerArmReference) <= 0.01 ||
        Distance(H.UpperBallJointReference, H.LowerBallJointReference) <= 0.01 ||
        Distance(H.UpperBallJointReference, H.TieRodOuterReference) <= 0.01 ||
        Distance(H.LowerBallJointReference, H.TieRodOuterReference) <= 0.01 ||
        Distance(H.UpperBallJointReference, H.WheelCenterReference) <= 0.01 ||
        Distance(H.LowerBallJointReference, H.WheelCenterReference) <= 0.01 ||
        Distance(H.TieRodOuterReference, H.WheelCenterReference) <= 0.01)
    {
        return false;
    }

    if (H.WheelForwardReference.IsNearlyZero() ||
        H.WheelUpReference.IsNearlyZero() ||
        H.SteeringRackAxisLocal.IsNearlyZero() ||
        FMath::Abs(H.SideSign) < 0.5)
    {
        return false;
    }

    FVector3d P;
    FVector3d S;
    FVector3d T;

    return BuildBasis(
        H.UpperBallJointReference,
        H.LowerBallJointReference,
        H.TieRodOuterReference,
        P,
        S,
        T);
}

bool TADoubleWishboneSolver::Solve(
    const FTADoubleWishboneSolverConfig& Config,
    const FTADoubleWishboneSolveInput& Input,
    FTADoubleWishboneState& InOutState,
    FTADoubleWishboneSolveOutput& OutOutput)
{
    OutOutput = FTADoubleWishboneSolveOutput{};

    if (!ValidateConfig(Config))
    {
        InOutState.bHasValidPreviousSolution = false;
        return false;
    }

    const FTADoubleWishboneHardpoints& H = Config.Hardpoints;

    const double TravelM =
        FMath::Clamp(
            Input.TravelM,
            Config.MinTravelM,
            Config.MaxTravelM);

    const FVector3d RackAxis =
        H.SteeringRackAxisLocal.GetSafeNormal();

    const FVector3d UpperInnerA =
        H.UpperInnerA + Input.Damage.UpperInnerA;

    const FVector3d UpperInnerB =
        H.UpperInnerB + Input.Damage.UpperInnerB;

    const FVector3d LowerInnerA =
        H.LowerInnerA + Input.Damage.LowerInnerA;

    const FVector3d LowerInnerB =
        H.LowerInnerB + Input.Damage.LowerInnerB;

    const FVector3d TieRodInner =
        H.TieRodInner
        + Input.Damage.TieRodInner
        + RackAxis * Input.RackDisplacementM;

    const double RestUpperA =
        Distance(H.UpperBallJointReference, H.UpperInnerA);

    const double RestUpperB =
        Distance(H.UpperBallJointReference, H.UpperInnerB);

    const double RestLowerA =
        Distance(H.LowerBallJointReference, H.LowerInnerA);

    const double RestLowerB =
        Distance(H.LowerBallJointReference, H.LowerInnerB);

    const double RestTie =
        Distance(H.TieRodOuterReference, H.TieRodInner);

    const double RestUL =
        Distance(H.UpperBallJointReference, H.LowerBallJointReference);

    const double RestUT =
        Distance(H.UpperBallJointReference, H.TieRodOuterReference);

    const double RestLT =
        Distance(H.LowerBallJointReference, H.TieRodOuterReference);

    const double RestUW =
        Distance(H.UpperBallJointReference, H.WheelCenterReference);

    const double RestLW =
        Distance(H.LowerBallJointReference, H.WheelCenterReference);

    const double RestTW =
        Distance(H.TieRodOuterReference, H.WheelCenterReference);

    FVector3d Points[4];

    if (InOutState.bHasValidPreviousSolution)
    {
        const double TravelDelta =
            TravelM - InOutState.SolvedTravelM;

        const FVector3d InitialShift(0.0, 0.0, TravelDelta);

        Points[UIndex] =
            InOutState.UpperBallJoint + InitialShift;

        Points[LIndex] =
            InOutState.LowerBallJoint + InitialShift;

        Points[TIndex] =
            InOutState.TieRodOuter + InitialShift;

        Points[WIndex] =
            InOutState.WheelCenter + InitialShift;
    }
    else
    {
        const FVector3d InitialShift(0.0, 0.0, TravelM);

        Points[UIndex] =
            H.UpperBallJointReference + InitialShift;

        Points[LIndex] =
            H.LowerBallJointReference + InitialShift;

        Points[TIndex] =
            H.TieRodOuterReference + InitialShift;

        Points[WIndex] =
            H.WheelCenterReference + InitialShift;
    }

    const double TargetWheelZ =
        H.WheelCenterReference.Z + TravelM;

    double ResidualM = TNumericLimits<double>::Max();
    int32 IterationsUsed = 0;

    for (int32 Iteration = 0;
         Iteration < Config.MaxIterations;
         ++Iteration)
    {
        ProjectPointToFixedDistance(
            Points[UIndex],
            UpperInnerA,
            RestUpperA);

        ProjectPointToFixedDistance(
            Points[UIndex],
            UpperInnerB,
            RestUpperB);

        ProjectPointToFixedDistance(
            Points[LIndex],
            LowerInnerA,
            RestLowerA);

        ProjectPointToFixedDistance(
            Points[LIndex],
            LowerInnerB,
            RestLowerB);

        ProjectPointToFixedDistance(
            Points[TIndex],
            TieRodInner,
            RestTie);

        ProjectPairDistance(
            Points[UIndex],
            Points[LIndex],
            RestUL);

        ProjectPairDistance(
            Points[UIndex],
            Points[TIndex],
            RestUT);

        ProjectPairDistance(
            Points[LIndex],
            Points[TIndex],
            RestLT);

        ProjectPairDistance(
            Points[UIndex],
            Points[WIndex],
            RestUW);

        ProjectPairDistance(
            Points[LIndex],
            Points[WIndex],
            RestLW);

        ProjectPairDistance(
            Points[TIndex],
            Points[WIndex],
            RestTW);

        Points[WIndex].Z = TargetWheelZ;

        ResidualM =
            MaxResidual(
                Points,
                UpperInnerA,
                UpperInnerB,
                LowerInnerA,
                LowerInnerB,
                TieRodInner,
                TargetWheelZ,
                RestUpperA,
                RestUpperB,
                RestLowerA,
                RestLowerB,
                RestTie,
                RestUL,
                RestUT,
                RestLT,
                RestUW,
                RestLW,
                RestTW);

        IterationsUsed = Iteration + 1;

        if (ResidualM <= Config.PositionToleranceM)
        {
            break;
        }
    }

    FVector3d RefPrimary;
    FVector3d RefSecondary;
    FVector3d RefTertiary;

    FVector3d CurPrimary;
    FVector3d CurSecondary;
    FVector3d CurTertiary;

    if (!BuildBasis(
            H.UpperBallJointReference,
            H.LowerBallJointReference,
            H.TieRodOuterReference,
            RefPrimary,
            RefSecondary,
            RefTertiary) ||
        !BuildBasis(
            Points[UIndex],
            Points[LIndex],
            Points[TIndex],
            CurPrimary,
            CurSecondary,
            CurTertiary))
    {
        InOutState.bHasValidPreviousSolution = false;
        return false;
    }

    FVector3d WheelForward =
        MapReferenceVector(
            H.WheelForwardReference.GetSafeNormal(),
            RefPrimary,
            RefSecondary,
            RefTertiary,
            CurPrimary,
            CurSecondary,
            CurTertiary).GetSafeNormal();

    FVector3d WheelUpCandidate =
        MapReferenceVector(
            H.WheelUpReference.GetSafeNormal(),
            RefPrimary,
            RefSecondary,
            RefTertiary,
            CurPrimary,
            CurSecondary,
            CurTertiary);

    WheelUpCandidate -=
        WheelForward
        * FVector3d::DotProduct(
            WheelUpCandidate,
            WheelForward);

    FVector3d WheelUp =
        WheelUpCandidate.GetSafeNormal();

    FVector3d WheelRight =
        FVector3d::CrossProduct(
            WheelUp,
            WheelForward).GetSafeNormal();

    if (WheelForward.IsNearlyZero() ||
        WheelUp.IsNearlyZero() ||
        WheelRight.IsNearlyZero())
    {
        InOutState.bHasValidPreviousSolution = false;
        return false;
    }

    WheelUp =
        FVector3d::CrossProduct(
            WheelForward,
            WheelRight).GetSafeNormal();

    const double ToeRad =
        FMath::Atan2(
            WheelForward.Y,
            WheelForward.X);

    const double SideSign =
        H.SideSign >= 0.0 ? 1.0 : -1.0;

    const double CamberRad =
        FMath::Atan2(
            SideSign * WheelUp.Y,
            WheelUp.Z);

    FVector3d RefLowerPrimary;
    FVector3d RefLowerSecondary;
    FVector3d RefLowerTertiary;

    FVector3d CurLowerPrimary;
    FVector3d CurLowerSecondary;
    FVector3d CurLowerTertiary;

    if (!BuildBasis(
            H.LowerInnerB,
            H.LowerInnerA,
            H.LowerBallJointReference,
            RefLowerPrimary,
            RefLowerSecondary,
            RefLowerTertiary) ||
        !BuildBasis(
            LowerInnerB,
            LowerInnerA,
            Points[LIndex],
            CurLowerPrimary,
            CurLowerSecondary,
            CurLowerTertiary))
    {
        InOutState.bHasValidPreviousSolution = false;
        return false;
    }

    const FVector3d DamperReferenceOffset =
        H.DamperLowerArmReference - H.LowerInnerA;

    const FVector3d DamperLowerArmLocalM =
        LowerInnerA
        + MapReferenceVector(
            DamperReferenceOffset,
            RefLowerPrimary,
            RefLowerSecondary,
            RefLowerTertiary,
            CurLowerPrimary,
            CurLowerSecondary,
            CurLowerTertiary);

    const FVector3d DamperChassisRuntime =
        H.DamperChassis + Input.Damage.DamperChassis;

    const double DamperLengthM =
        Distance(
            DamperLowerArmLocalM,
            DamperChassisRuntime);

    InOutState.UpperBallJoint = Points[UIndex];
    InOutState.LowerBallJoint = Points[LIndex];
    InOutState.TieRodOuter = Points[TIndex];
    InOutState.WheelCenter = Points[WIndex];
    InOutState.SolvedTravelM = TravelM;

    OutOutput.bConverged =
        ResidualM <= Config.PositionToleranceM;

    OutOutput.IterationsUsed = IterationsUsed;
    OutOutput.MaxConstraintResidualM = ResidualM;

    OutOutput.WheelCenterLocalM = Points[WIndex];
    OutOutput.WheelForwardLocal = WheelForward;
    OutOutput.WheelRightLocal = WheelRight;
    OutOutput.WheelUpLocal = WheelUp;

    OutOutput.DamperLowerArmLocalM =
        DamperLowerArmLocalM;

    OutOutput.DamperLengthM =
        DamperLengthM;

    OutOutput.CamberRad = CamberRad;
    OutOutput.ToeRad = ToeRad;

    InOutState.bHasValidPreviousSolution =
        OutOutput.bConverged;

    return OutOutput.bConverged;
}
