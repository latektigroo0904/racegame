#include "TAMultiLinkSolver.h"

namespace
{
    constexpr int32 WheelPointIndex = TARearMultiLinkCount;
    constexpr int32 DamperPointIndex = TARearMultiLinkCount + 1;
    constexpr int32 TotalRigidPointCount = TARearMultiLinkCount + 2;

    double Distance(
        const FVector3d& A,
        const FVector3d& B)
    {
        return (B - A).Length();
    }

    bool IsFiniteVector(
        const FVector3d& V)
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
        const FVector3d Delta =
            Point - Fixed;

        const double LengthM =
            Delta.Length();

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
        const FVector3d Delta =
            B - A;

        const double LengthM =
            Delta.Length();

        if (LengthM <= UE_DOUBLE_SMALL_NUMBER)
        {
            return;
        }

        const FVector3d Direction =
            Delta / LengthM;

        const double ErrorM =
            LengthM - RestLengthM;

        const FVector3d HalfCorrection =
            0.5 * ErrorM * Direction;

        A += HalfCorrection;
        B -= HalfCorrection;
    }

    bool BuildBasis(
        const FVector3d& A,
        const FVector3d& B,
        const FVector3d& C,
        FVector3d& OutPrimary,
        FVector3d& OutSecondary,
        FVector3d& OutTertiary)
    {
        OutPrimary =
            (B - A).GetSafeNormal();

        if (OutPrimary.IsNearlyZero())
        {
            return false;
        }

        const FVector3d ThirdVector =
            C - A;

        const FVector3d SecondaryRaw =
            ThirdVector
            - OutPrimary
            * FVector3d::DotProduct(
                ThirdVector,
                OutPrimary);

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
                * FVector3d::DotProduct(
                    ReferenceVector,
                    RefPrimary)
            + CurSecondary
                * FVector3d::DotProduct(
                    ReferenceVector,
                    RefSecondary)
            + CurTertiary
                * FVector3d::DotProduct(
                    ReferenceVector,
                    RefTertiary);
    }
}

FTAMultiLinkSolverConfig TAMultiLinkSolver::MirrorAcrossCenterline(
    const FTAMultiLinkSolverConfig& RightSideConfig)
{
    FTAMultiLinkSolverConfig Mirrored =
        RightSideConfig;

    auto MirrorPoint =
        [](FVector3d& Point)
        {
            Point.Y = -Point.Y;
        };

    for (int32 Index = 0;
         Index < TARearMultiLinkCount;
         ++Index)
    {
        MirrorPoint(
            Mirrored.Links[Index]
                .ChassisPickupReference);

        MirrorPoint(
            Mirrored.Links[Index]
                .UprightPickupReference);
    }

    MirrorPoint(
        Mirrored.WheelCenterReference);

    MirrorPoint(
        Mirrored.DamperChassisReference);

    MirrorPoint(
        Mirrored.DamperUprightReference);

    Mirrored.SideSign =
        Mirrored.SideSign >= 0.0
        ? -1.0
        : 1.0;

    return Mirrored;
}

bool TAMultiLinkSolver::ValidateConfig(
    const FTAMultiLinkSolverConfig& Config)
{
    if (Config.MaxIterations <= 0 ||
        Config.PositionToleranceM <= 0.0 ||
        Config.MaxTravelM <= Config.MinTravelM ||
        FMath::Abs(Config.SideSign) < 0.5)
    {
        return false;
    }

    for (int32 Index = 0;
         Index < TARearMultiLinkCount;
         ++Index)
    {
        const FTAMultiLinkLinkConfig& Link =
            Config.Links[Index];

        if (!IsFiniteVector(
                Link.ChassisPickupReference) ||
            !IsFiniteVector(
                Link.UprightPickupReference) ||
            Distance(
                Link.ChassisPickupReference,
                Link.UprightPickupReference) <= 0.01)
        {
            return false;
        }
    }

    if (!IsFiniteVector(
            Config.WheelCenterReference) ||
        !IsFiniteVector(
            Config.DamperChassisReference) ||
        !IsFiniteVector(
            Config.DamperUprightReference) ||
        Config.WheelForwardReference.IsNearlyZero() ||
        Config.WheelUpReference.IsNearlyZero() ||
        Distance(
            Config.DamperChassisReference,
            Config.DamperUprightReference) <= 0.01)
    {
        return false;
    }

    FVector3d Primary;
    FVector3d Secondary;
    FVector3d Tertiary;

    if (!BuildBasis(
            Config.Links[0].UprightPickupReference,
            Config.Links[1].UprightPickupReference,
            Config.Links[2].UprightPickupReference,
            Primary,
            Secondary,
            Tertiary))
    {
        return false;
    }

    const FVector3d RigidPoints[TotalRigidPointCount] =
    {
        Config.Links[0].UprightPickupReference,
        Config.Links[1].UprightPickupReference,
        Config.Links[2].UprightPickupReference,
        Config.Links[3].UprightPickupReference,
        Config.Links[4].UprightPickupReference,
        Config.WheelCenterReference,
        Config.DamperUprightReference
    };

    for (int32 A = 0;
         A < TotalRigidPointCount;
         ++A)
    {
        for (int32 B = A + 1;
             B < TotalRigidPointCount;
             ++B)
        {
            if (Distance(
                    RigidPoints[A],
                    RigidPoints[B]) <= 0.005)
            {
                return false;
            }
        }
    }

    return true;
}

bool TAMultiLinkSolver::Solve(
    const FTAMultiLinkSolverConfig& Config,
    const FTAMultiLinkSolveInput& Input,
    FTAMultiLinkRuntimeState& InOutState,
    FTAMultiLinkSolveOutput& OutOutput)
{
    OutOutput =
        FTAMultiLinkSolveOutput{};

    if (!ValidateConfig(Config))
    {
        InOutState.bHasValidPreviousSolution =
            false;

        return false;
    }

    const double TravelM =
        FMath::Clamp(
            Input.TravelM,
            Config.MinTravelM,
            Config.MaxTravelM);

    FVector3d ChassisPickups[TARearMultiLinkCount];
    double LinkRestLengths[TARearMultiLinkCount];

    FVector3d ReferenceRigidPoints[TotalRigidPointCount];

    for (int32 Index = 0;
         Index < TARearMultiLinkCount;
         ++Index)
    {
        ChassisPickups[Index] =
            Config.Links[Index].ChassisPickupReference
            + Input.Damage.ChassisPickupOffsets[Index];

        ReferenceRigidPoints[Index] =
            Config.Links[Index].UprightPickupReference;

        LinkRestLengths[Index] =
            Distance(
                Config.Links[Index].ChassisPickupReference,
                Config.Links[Index].UprightPickupReference);
    }

    ReferenceRigidPoints[WheelPointIndex] =
        Config.WheelCenterReference;

    ReferenceRigidPoints[DamperPointIndex] =
        Config.DamperUprightReference;

    double RigidRestLengths
        [TotalRigidPointCount]
        [TotalRigidPointCount] = {};

    for (int32 A = 0;
         A < TotalRigidPointCount;
         ++A)
    {
        for (int32 B = A + 1;
             B < TotalRigidPointCount;
             ++B)
        {
            RigidRestLengths[A][B] =
                Distance(
                    ReferenceRigidPoints[A],
                    ReferenceRigidPoints[B]);
        }
    }

    FVector3d Points[TotalRigidPointCount];

    if (InOutState.bHasValidPreviousSolution)
    {
        const double TravelDeltaM =
            TravelM
            - InOutState.SolvedTravelM;

        const FVector3d Shift(
            0.0,
            0.0,
            TravelDeltaM);

        for (int32 Index = 0;
             Index < TARearMultiLinkCount;
             ++Index)
        {
            Points[Index] =
                InOutState.UprightLinkPoints[Index]
                + Shift;
        }

        Points[WheelPointIndex] =
            InOutState.WheelCenter + Shift;

        Points[DamperPointIndex] =
            InOutState.DamperUpright + Shift;
    }
    else
    {
        const FVector3d Shift(
            0.0,
            0.0,
            TravelM);

        for (int32 Index = 0;
             Index < TotalRigidPointCount;
             ++Index)
        {
            Points[Index] =
                ReferenceRigidPoints[Index]
                + Shift;
        }
    }

    const double TargetWheelZ =
        Config.WheelCenterReference.Z
        + TravelM;

    double ResidualM =
        TNumericLimits<double>::Max();

    int32 IterationsUsed = 0;

    for (int32 Iteration = 0;
         Iteration < Config.MaxIterations;
         ++Iteration)
    {
        for (int32 LinkIndex = 0;
             LinkIndex < TARearMultiLinkCount;
             ++LinkIndex)
        {
            ProjectPointToFixedDistance(
                Points[LinkIndex],
                ChassisPickups[LinkIndex],
                LinkRestLengths[LinkIndex]);
        }

        for (int32 A = 0;
             A < TotalRigidPointCount;
             ++A)
        {
            for (int32 B = A + 1;
                 B < TotalRigidPointCount;
                 ++B)
            {
                ProjectPairDistance(
                    Points[A],
                    Points[B],
                    RigidRestLengths[A][B]);
            }
        }

        Points[WheelPointIndex].Z =
            TargetWheelZ;

        ResidualM = 0.0;

        for (int32 LinkIndex = 0;
             LinkIndex < TARearMultiLinkCount;
             ++LinkIndex)
        {
            ResidualM =
                FMath::Max(
                    ResidualM,
                    FMath::Abs(
                        Distance(
                            Points[LinkIndex],
                            ChassisPickups[LinkIndex])
                        - LinkRestLengths[LinkIndex]));
        }

        for (int32 A = 0;
             A < TotalRigidPointCount;
             ++A)
        {
            for (int32 B = A + 1;
                 B < TotalRigidPointCount;
                 ++B)
            {
                ResidualM =
                    FMath::Max(
                        ResidualM,
                        FMath::Abs(
                            Distance(
                                Points[A],
                                Points[B])
                            - RigidRestLengths[A][B]));
            }
        }

        ResidualM =
            FMath::Max(
                ResidualM,
                FMath::Abs(
                    Points[WheelPointIndex].Z
                    - TargetWheelZ));

        IterationsUsed =
            Iteration + 1;

        if (ResidualM <=
            Config.PositionToleranceM)
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
            ReferenceRigidPoints[0],
            ReferenceRigidPoints[1],
            ReferenceRigidPoints[2],
            RefPrimary,
            RefSecondary,
            RefTertiary) ||
        !BuildBasis(
            Points[0],
            Points[1],
            Points[2],
            CurPrimary,
            CurSecondary,
            CurTertiary))
    {
        InOutState.bHasValidPreviousSolution =
            false;

        return false;
    }

    FVector3d WheelForward =
        MapReferenceVector(
            Config.WheelForwardReference.GetSafeNormal(),
            RefPrimary,
            RefSecondary,
            RefTertiary,
            CurPrimary,
            CurSecondary,
            CurTertiary).GetSafeNormal();

    FVector3d WheelUpCandidate =
        MapReferenceVector(
            Config.WheelUpReference.GetSafeNormal(),
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
        InOutState.bHasValidPreviousSolution =
            false;

        return false;
    }

    WheelUp =
        FVector3d::CrossProduct(
            WheelForward,
            WheelRight).GetSafeNormal();

    const double SideSign =
        Config.SideSign >= 0.0
        ? 1.0
        : -1.0;

    const double ToeRad =
        FMath::Atan2(
            WheelForward.Y,
            WheelForward.X);

    const double CamberRad =
        FMath::Atan2(
            SideSign * WheelUp.Y,
            WheelUp.Z);

    const FVector3d DamperChassisRuntime =
        Config.DamperChassisReference
        + Input.Damage.DamperChassisOffset;

    const double DamperLengthM =
        Distance(
            DamperChassisRuntime,
            Points[DamperPointIndex]);

    for (int32 Index = 0;
         Index < TARearMultiLinkCount;
         ++Index)
    {
        InOutState.UprightLinkPoints[Index] =
            Points[Index];
    }

    InOutState.WheelCenter =
        Points[WheelPointIndex];

    InOutState.DamperUpright =
        Points[DamperPointIndex];

    InOutState.SolvedTravelM =
        TravelM;

    OutOutput.bConverged =
        ResidualM <= Config.PositionToleranceM;

    OutOutput.IterationsUsed =
        IterationsUsed;

    OutOutput.MaxConstraintResidualM =
        ResidualM;

    OutOutput.WheelCenterLocalM =
        Points[WheelPointIndex];

    OutOutput.WheelForwardLocal =
        WheelForward;

    OutOutput.WheelRightLocal =
        WheelRight;

    OutOutput.WheelUpLocal =
        WheelUp;

    OutOutput.DamperUprightLocalM =
        Points[DamperPointIndex];

    OutOutput.DamperLengthM =
        DamperLengthM;

    OutOutput.CamberRad =
        CamberRad;

    OutOutput.ToeRad =
        ToeRad;

    InOutState.bHasValidPreviousSolution =
        OutOutput.bConverged;

    return OutOutput.bConverged;
}
