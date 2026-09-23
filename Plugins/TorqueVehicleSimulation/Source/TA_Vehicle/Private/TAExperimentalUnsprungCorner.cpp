#include "TAExperimentalUnsprungCorner.h"

namespace
{
    struct FCornerForceEvaluation
    {
        FVector3d RoadNormalWorld =
            FVector3d(0.0, 0.0, 1.0);

        FVector3d TravelAxisWorld =
            FVector3d(0.0, 0.0, 1.0);

        double RoadToTravelProjection01 = 1.0;

        double SignedUnloadedContactDistanceM = 0.0;
        double RequestedTireDeflectionM = 0.0;

        double TireNormalForceN = 0.0;
        double SuspensionReactionN = 0.0;

        FTADoubleWishboneSolveOutput Geometry;
        FTASuspensionRuntimeState SuspensionState;
        FTASuspensionForceOutput SuspensionForce;
        FTATireVerticalForceOutput TireVertical;

        FVector3d WheelCenterWorldM =
            FVector3d::ZeroVector;

        FVector3d ContactPointWorldM =
            FVector3d::ZeroVector;

        FVector3d ForwardTangentWorld =
            FVector3d(1.0, 0.0, 0.0);

        FVector3d RightTangentWorld =
            FVector3d(0.0, 1.0, 0.0);
    };

    bool IsFiniteVector(
        const FVector3d& Value)
    {
        return
            FMath::IsFinite(Value.X)
            && FMath::IsFinite(Value.Y)
            && FMath::IsFinite(Value.Z);
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
                CurrentTravelM
                    + ProbeDistanceM);

        if (FMath::IsNearlyEqual(
                ProbeTravelM,
                CurrentTravelM,
                1.0e-9))
        {
            ProbeTravelM =
                FMath::Max(
                    GeometryConfig.MinTravelM,
                    CurrentTravelM
                        - ProbeDistanceM);
        }

        const double TravelDeltaM =
            ProbeTravelM
            - CurrentTravelM;

        if (FMath::Abs(TravelDeltaM)
            <= 1.0e-9)
        {
            return 1.0;
        }

        FTADoubleWishboneState ProbeState;
        FTADoubleWishboneSolveOutput ProbeOutput;
        FTADoubleWishboneSolveInput ProbeInput;

        ProbeInput.TravelM =
            ProbeTravelM;

        ProbeInput.RackDisplacementM =
            RackDisplacementM;

        ProbeInput.Damage =
            DamageOffsets;

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
                (ProbeOutput.DamperLengthM
                    - CurrentDamperLengthM)
                / TravelDeltaM);

        return FMath::Clamp(
            Ratio,
            0.05,
            3.0);
    }

    bool EvaluateCurrentCorner(
        const FTAExperimentalUnsprungCornerConfig& Config,
        const FTAExperimentalUnsprungCornerInput& Input,
        const double ForceDeltaTimeSeconds,
        FTAExperimentalUnsprungCornerState& InOutState,
        FCornerForceEvaluation& Out)
    {
        Out =
            FCornerForceEvaluation{};

        const FVector3d RoadNormal =
            Input.Road.NormalWorld.GetSafeNormal();

        if (RoadNormal.IsNearlyZero())
        {
            return false;
        }

        const FVector3d TravelAxisWorld =
            Input.Chassis.OrientationWorld.RotateVector(
                FVector3d(0.0, 0.0, 1.0))
            .GetSafeNormal();

        if (TravelAxisWorld.IsNearlyZero())
        {
            return false;
        }

        const double RoadProjection =
            FVector3d::DotProduct(
                RoadNormal,
                TravelAxisWorld);

        if (RoadProjection < 0.20)
        {
            return false;
        }

        FTADoubleWishboneSolveInput GeometryInput;

        GeometryInput.TravelM =
            InOutState.Unsprung.TravelM;

        GeometryInput.RackDisplacementM =
            Input.RackDisplacementM;

        GeometryInput.Damage =
            Input.Damage;

        if (!TADoubleWishboneSolver::Solve(
                Config.Geometry,
                GeometryInput,
                InOutState.Geometry,
                Out.Geometry))
        {
            return false;
        }

        Out.RoadNormalWorld =
            RoadNormal;

        Out.TravelAxisWorld =
            TravelAxisWorld;

        Out.RoadToTravelProjection01 =
            RoadProjection;

        Out.SuspensionState =
            InOutState.Suspension;

        Out.SuspensionState.TravelM =
            InOutState.Unsprung.TravelM;

        Out.SuspensionState.TravelVelocityMps =
            InOutState.Unsprung.TravelVelocityMps;

        Out.SuspensionState.MotionRatio =
            EstimateMotionRatio(
                Config.Geometry,
                Input.RackDisplacementM,
                Input.Damage,
                InOutState.Unsprung.TravelM,
                Out.Geometry.DamperLengthM);

        Out.SuspensionState.WheelCenterOffsetM =
            Out.Geometry.WheelCenterLocalM
            - Config.Geometry.Hardpoints
                .WheelCenterReference;

        Out.SuspensionState.CamberRad =
            Out.Geometry.CamberRad;

        Out.SuspensionState.ToeRad =
            Out.Geometry.ToeRad;

        Out.SuspensionState.bTravelInitialized =
            true;

        Out.SuspensionForce =
            TASuspensionRuntime::CalculateForce(
                Config.Suspension,
                Out.SuspensionState);

        Out.SuspensionReactionN =
            FMath::Max(
                0.0,
                Out.SuspensionForce.TotalForceN
                    + Input.AdditionalSuspensionReactionN);

        Out.WheelCenterWorldM =
            Input.Chassis.PositionWorldM
            + Input.Chassis.OrientationWorld.RotateVector(
                Out.Geometry.WheelCenterLocalM);

        const FVector3d UnloadedContactPointWorldM =
            Out.WheelCenterWorldM
            - RoadNormal
                * Config.Tire.UnloadedRadiusM;

        Out.SignedUnloadedContactDistanceM =
            FVector3d::DotProduct(
                UnloadedContactPointWorldM
                    - Input.Road.PointWorldM,
                RoadNormal);

        Out.RequestedTireDeflectionM =
            FMath::Max(
                0.0,
                -Out.SignedUnloadedContactDistanceM);

        const double DeflectionVelocityMps =
            InOutState.Tire.bRadialStateInitialized
            ? (Out.RequestedTireDeflectionM
                - InOutState.Tire.RadialDeflectionM)
                / ForceDeltaTimeSeconds
            : 0.0;

        Out.TireVertical =
            TATireSolver::CalculateVerticalForce(
                Config.Tire,
                InOutState.Tire,
                Out.RequestedTireDeflectionM,
                DeflectionVelocityMps);

        Out.TireNormalForceN =
            Out.TireVertical.NormalForceN;

        const double LoadedRadiusM =
            FMath::Max(
                0.01,
                Config.Tire.UnloadedRadiusM
                    - Out.TireVertical
                        .EffectiveDeflectionM);

        Out.ContactPointWorldM =
            Out.WheelCenterWorldM
            - RoadNormal
                * LoadedRadiusM;

        FVector3d ForwardWorld =
            Input.Chassis.OrientationWorld.RotateVector(
                Out.Geometry.WheelForwardLocal);

        ForwardWorld -=
            RoadNormal
            * FVector3d::DotProduct(
                ForwardWorld,
                RoadNormal);

        ForwardWorld =
            ForwardWorld.GetSafeNormal();

        if (ForwardWorld.IsNearlyZero())
        {
            return false;
        }

        Out.ForwardTangentWorld =
            ForwardWorld;

        Out.RightTangentWorld =
            FVector3d::CrossProduct(
                RoadNormal,
                ForwardWorld)
            .GetSafeNormal();

        return
            !Out.RightTangentWorld.IsNearlyZero();
    }

    void BuildOutput(
        const FTAExperimentalUnsprungCornerInput& Input,
        const FCornerForceEvaluation& Evaluation,
        const FTAUnsprungVerticalOutput& UnsprungOutput,
        const FTAExperimentalUnsprungCornerState& State,
        FTAExperimentalUnsprungCornerOutput& OutOutput)
    {
        OutOutput =
            FTAExperimentalUnsprungCornerOutput{};

        OutOutput.bSolved =
            true;

        OutOutput.bInContact =
            Evaluation.TireNormalForceN > 0.0
            || Evaluation.TireVertical
                .EffectiveDeflectionM > 0.0;

        OutOutput.TireNormalForceN =
            Evaluation.TireNormalForceN;

        OutOutput.ChassisSuspensionReactionN =
            Evaluation.SuspensionReactionN;

        OutOutput.RequestedTireDeflectionM =
            Evaluation.RequestedTireDeflectionM;

        OutOutput.TireRadialDeflectionM =
            Evaluation.TireVertical
                .EffectiveDeflectionM;

        OutOutput.TravelM =
            State.Unsprung.TravelM;

        OutOutput.TravelVelocityMps =
            State.Unsprung.TravelVelocityMps;

        OutOutput.RoadNormalWorld =
            Evaluation.RoadNormalWorld;

        OutOutput.ChassisSuspensionForceWorldN =
            Evaluation.TravelAxisWorld
            * Evaluation.SuspensionReactionN;

        OutOutput.Geometry =
            Evaluation.Geometry;

        OutOutput.SuspensionForce =
            Evaluation.SuspensionForce;

        OutOutput.Unsprung =
            UnsprungOutput;

        FTAWheelContactInput& Contact =
            OutOutput.VehicleContact;

        Contact.VerticalLoadN =
            Evaluation.TireNormalForceN;

        const FVector3d ChassisPointVelocityWorldMps =
            TAWheelContactResolver::CalculatePointVelocityWorld(
                Input.Chassis,
                Evaluation.ContactPointWorldM);

        const FVector3d UnsprungRelativeVelocityWorldMps =
            Evaluation.TravelAxisWorld
            * State.Unsprung.TravelVelocityMps;

        const FVector3d ContactVelocityWorldMps =
            ChassisPointVelocityWorldMps
            + UnsprungRelativeVelocityWorldMps;

        Contact.LongitudinalVelocityMps =
            FVector3d::DotProduct(
                ContactVelocityWorldMps,
                Evaluation.ForwardTangentWorld);

        Contact.LateralVelocityMps =
            FVector3d::DotProduct(
                ContactVelocityWorldMps,
                Evaluation.RightTangentWorld);

        Contact.CamberRad =
            Evaluation.Geometry.CamberRad;

        Contact.ContactPointWorldM =
            Evaluation.ContactPointWorldM;

        Contact.ForwardDirectionWorld =
            Evaluation.ForwardTangentWorld;

        Contact.RightDirectionWorld =
            Evaluation.RightTangentWorld;

        // Critical ownership rule:
        // the road/tire normal force accelerates unsprung mass;
        // only the suspension/link reaction is transmitted directly
        // to the chassis.
        Contact.SuspensionForceWorldN =
            OutOutput.ChassisSuspensionForceWorldN;

        Contact.Surface =
            Input.Road.Surface;
    }
}

bool TAExperimentalUnsprungCorner::ValidateConfig(
    const FTAExperimentalUnsprungCornerConfig& Config)
{
    return
        TADoubleWishboneSolver::ValidateConfig(
            Config.Geometry)
        && TAUnsprungVerticalDynamics::ValidateConfig(
            Config.Unsprung)
        && Config.Tire.UnloadedRadiusM
            > UE_DOUBLE_SMALL_NUMBER
        && Config.InternalSubsteps >= 1
        && Config.InternalSubsteps <= 16
        && IsFiniteVector(
            Config.GravityWorldMps2);
}

bool TAExperimentalUnsprungCorner::InitializeFromQuasiStatic(
    const FTAExperimentalUnsprungCornerConfig& Config,
    const FTAExperimentalUnsprungCornerInput& Input,
    const double DeltaTimeSeconds,
    FTAExperimentalUnsprungCornerState& OutState,
    FTAExperimentalUnsprungCornerOutput& OutOutput)
{
    OutState =
        FTAExperimentalUnsprungCornerState{};

    OutOutput =
        FTAExperimentalUnsprungCornerOutput{};

    if (!ValidateConfig(Config)
        || DeltaTimeSeconds <= 0.0)
    {
        return false;
    }

    OutState.Tire.PressureKPa =
        Config.Tire.ReferencePressureKPa;

    OutState.Tire.TreadDepthMm =
        Config.Tire.NewTreadDepthMm;

    FTAResolvedWheelContact Contact;

    if (!TAWheelContactResolver::
            ResolveDoubleWishboneCompliantRoadContact(
                Input.Chassis,
                Config.Geometry,
                Config.Suspension,
                Config.Tire,
                Input.RackDisplacementM,
                Input.AdditionalSuspensionReactionN,
                Input.Damage,
                Input.Road,
                DeltaTimeSeconds,
                OutState.Geometry,
                OutState.Suspension,
                OutState.Tire,
                Contact))
    {
        return false;
    }

    OutState.Unsprung.TravelM =
        Contact.TravelM;

    OutState.Unsprung.TravelVelocityMps =
        0.0;

    OutState.bInitialized =
        true;

    FCornerForceEvaluation Evaluation;

    if (!EvaluateCurrentCorner(
            Config,
            Input,
            DeltaTimeSeconds,
            OutState,
            Evaluation))
    {
        return false;
    }

    TATireSolver::CommitVerticalState(
        Evaluation.TireVertical,
        OutState.Tire);

    FTAUnsprungVerticalOutput UnsprungOutput;

    BuildOutput(
        Input,
        Evaluation,
        UnsprungOutput,
        OutState,
        OutOutput);

    return true;
}

bool TAExperimentalUnsprungCorner::Step(
    const FTAExperimentalUnsprungCornerConfig& Config,
    const FTAExperimentalUnsprungCornerInput& Input,
    const double DeltaTimeSeconds,
    FTAExperimentalUnsprungCornerState& InOutState,
    FTAExperimentalUnsprungCornerOutput& OutOutput)
{
    OutOutput =
        FTAExperimentalUnsprungCornerOutput{};

    if (!ValidateConfig(Config)
        || !InOutState.bInitialized
        || DeltaTimeSeconds <= 0.0
        || !IsFiniteVector(
            Input.ChassisLinearAccelerationWorldMps2))
    {
        return false;
    }

    const int32 Substeps =
        FMath::Clamp(
            Config.InternalSubsteps,
            1,
            16);

    const double SubstepSeconds =
        DeltaTimeSeconds
        / static_cast<double>(Substeps);

    FTAUnsprungVerticalOutput LastUnsprungOutput;

    for (int32 Substep = 0;
         Substep < Substeps;
         ++Substep)
    {
        FCornerForceEvaluation Evaluation;

        if (!EvaluateCurrentCorner(
                Config,
                Input,
                SubstepSeconds,
                InOutState,
                Evaluation))
        {
            return false;
        }

        InOutState.Suspension =
            Evaluation.SuspensionState;

        TATireSolver::CommitVerticalState(
            Evaluation.TireVertical,
            InOutState.Tire);

        FTAUnsprungVerticalInput UnsprungInput;

        UnsprungInput.TireNormalForceN =
            Evaluation.TireNormalForceN
            * Evaluation.RoadToTravelProjection01;

        UnsprungInput.SuspensionReactionForceN =
            Evaluation.SuspensionReactionN;

        UnsprungInput.GravityAlongTravelAxisMps2 =
            FVector3d::DotProduct(
                Config.GravityWorldMps2,
                Evaluation.TravelAxisWorld);

        UnsprungInput.ChassisAccelerationAlongTravelAxisMps2 =
            FVector3d::DotProduct(
                Input.ChassisLinearAccelerationWorldMps2,
                Evaluation.TravelAxisWorld);

        if (!TAUnsprungVerticalDynamics::Integrate(
                Config.Unsprung,
                UnsprungInput,
                SubstepSeconds,
                InOutState.Unsprung,
                LastUnsprungOutput))
        {
            return false;
        }
    }

    FCornerForceEvaluation FinalEvaluation;

    if (!EvaluateCurrentCorner(
            Config,
            Input,
            SubstepSeconds,
            InOutState,
            FinalEvaluation))
    {
        return false;
    }

    InOutState.Suspension =
        FinalEvaluation.SuspensionState;

    TATireSolver::CommitVerticalState(
        FinalEvaluation.TireVertical,
        InOutState.Tire);

    BuildOutput(
        Input,
        FinalEvaluation,
        LastUnsprungOutput,
        InOutState,
        OutOutput);

    return true;
}
