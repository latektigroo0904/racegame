#include "TAChassisDynamics.h"

bool TAChassisDynamics::IsConfigValid(
    const FTAChassisConfig& Config)
{
    return
        Config.MassKg > UE_DOUBLE_SMALL_NUMBER
        && Config.PrincipalInertiaBodyKgm2.X > UE_DOUBLE_SMALL_NUMBER
        && Config.PrincipalInertiaBodyKgm2.Y > UE_DOUBLE_SMALL_NUMBER
        && Config.PrincipalInertiaBodyKgm2.Z > UE_DOUBLE_SMALL_NUMBER;
}

void TAChassisDynamics::AddForceWorld(
    const FVector3d& ForceWorldN,
    FTAChassisForceAccumulator& InOutAccumulator)
{
    InOutAccumulator.TotalForceWorldN += ForceWorldN;
}

void TAChassisDynamics::AddTorqueWorld(
    const FVector3d& TorqueWorldNm,
    FTAChassisForceAccumulator& InOutAccumulator)
{
    InOutAccumulator.TotalTorqueWorldNm += TorqueWorldNm;
}

void TAChassisDynamics::AddForceAtWorldPoint(
    const FTAChassisState& State,
    const FVector3d& ForceWorldN,
    const FVector3d& PointWorldM,
    FTAChassisForceAccumulator& InOutAccumulator)
{
    const FVector3d LeverArmM =
        PointWorldM - State.PositionWorldM;

    InOutAccumulator.TotalForceWorldN += ForceWorldN;
    InOutAccumulator.TotalTorqueWorldNm +=
        FVector3d::CrossProduct(LeverArmM, ForceWorldN);
}

bool TAChassisDynamics::Integrate(
    const FTAChassisConfig& Config,
    const FTAChassisForceAccumulator& Accumulator,
    const double DeltaTimeSeconds,
    FTAChassisState& InOutState)
{
    if (!IsConfigValid(Config) || DeltaTimeSeconds <= 0.0)
    {
        return false;
    }

    InOutState.OrientationWorld.Normalize();

    const FVector3d LinearAccelerationWorldMps2 =
        Accumulator.TotalForceWorldN / Config.MassKg
        + Config.GravityWorldMps2;

    InOutState.LinearVelocityWorldMps +=
        LinearAccelerationWorldMps2 * DeltaTimeSeconds;

    InOutState.PositionWorldM +=
        InOutState.LinearVelocityWorldMps * DeltaTimeSeconds;

    const FVector3d OmegaBody =
        InOutState.OrientationWorld.UnrotateVector(
            InOutState.AngularVelocityWorldRadPerSec);

    const FVector3d TorqueBody =
        InOutState.OrientationWorld.UnrotateVector(
            Accumulator.TotalTorqueWorldNm);

    const FVector3d AngularMomentumBody(
        Config.PrincipalInertiaBodyKgm2.X * OmegaBody.X,
        Config.PrincipalInertiaBodyKgm2.Y * OmegaBody.Y,
        Config.PrincipalInertiaBodyKgm2.Z * OmegaBody.Z);

    const FVector3d GyroscopicTorqueBody =
        FVector3d::CrossProduct(
            OmegaBody,
            AngularMomentumBody);

    const FVector3d EffectiveTorqueBody =
        TorqueBody - GyroscopicTorqueBody;

    const FVector3d AngularAccelerationBody(
        EffectiveTorqueBody.X / Config.PrincipalInertiaBodyKgm2.X,
        EffectiveTorqueBody.Y / Config.PrincipalInertiaBodyKgm2.Y,
        EffectiveTorqueBody.Z / Config.PrincipalInertiaBodyKgm2.Z);

    const FVector3d NextOmegaBody =
        OmegaBody + AngularAccelerationBody * DeltaTimeSeconds;

    const FVector3d NextOmegaWorld =
        InOutState.OrientationWorld.RotateVector(NextOmegaBody);

    const double AngularSpeed =
        NextOmegaWorld.Length();

    if (AngularSpeed > UE_DOUBLE_SMALL_NUMBER)
    {
        const FVector3d AxisWorld =
            NextOmegaWorld / AngularSpeed;

        const double AngleRad =
            AngularSpeed * DeltaTimeSeconds;

        const FQuat DeltaRotation(
            FVector(AxisWorld),
            AngleRad);

        InOutState.OrientationWorld =
            DeltaRotation * InOutState.OrientationWorld;

        InOutState.OrientationWorld.Normalize();
    }

    InOutState.AngularVelocityWorldRadPerSec =
        InOutState.OrientationWorld.RotateVector(NextOmegaBody);

    return true;
}
