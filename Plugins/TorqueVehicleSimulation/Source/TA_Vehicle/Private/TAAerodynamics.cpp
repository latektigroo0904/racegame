#include "TAAerodynamics.h"

namespace
{
    bool IsFiniteVector(const FVector3d& Value)
    {
        return
            FMath::IsFinite(Value.X)
            && FMath::IsFinite(Value.Y)
            && FMath::IsFinite(Value.Z);
    }

    FVector3d BodyPointToWorld(
        const FTAChassisState& Chassis,
        const FVector3d& BodyPointM)
    {
        return
            Chassis.PositionWorldM
            + FVector3d(
                Chassis.OrientationWorld.RotateVector(
                    FVector(BodyPointM)));
    }

    FVector3d TorqueFromForceAtPoint(
        const FTAChassisState& Chassis,
        const FVector3d& ForceWorldN,
        const FVector3d& PointWorldM)
    {
        return FVector3d::CrossProduct(
            PointWorldM - Chassis.PositionWorldM,
            ForceWorldN);
    }
}

bool TAAerodynamics::ValidateConfig(
    const FTAAerodynamicsConfig& Config)
{
    return
        FMath::IsFinite(Config.ReferenceAreaM2)
        && Config.ReferenceAreaM2 > 0.0
        && FMath::IsFinite(Config.DragCoefficient)
        && Config.DragCoefficient >= 0.0
        && FMath::IsFinite(Config.FrontLiftCoefficient)
        && FMath::IsFinite(Config.RearLiftCoefficient)
        && IsFiniteVector(Config.DragApplicationPointBodyM)
        && IsFiniteVector(Config.FrontLiftApplicationPointBodyM)
        && IsFiniteVector(Config.RearLiftApplicationPointBodyM);
}

bool TAAerodynamics::Calculate(
    const FTAAerodynamicsConfig& Config,
    const FTAAerodynamicsEnvironment& Environment,
    const FTAChassisState& Chassis,
    FTAAerodynamicsOutput& OutOutput)
{
    OutOutput =
        FTAAerodynamicsOutput{};

    if (!ValidateConfig(Config)
        || !FMath::IsFinite(Environment.AirDensityKgPerM3)
        || Environment.AirDensityKgPerM3 < 0.0
        || !IsFiniteVector(Environment.WindVelocityWorldMps)
        || !IsFiniteVector(Chassis.LinearVelocityWorldMps))
    {
        return false;
    }

    OutOutput.RelativeAirVelocityWorldMps =
        Environment.WindVelocityWorldMps
        - Chassis.LinearVelocityWorldMps;

    OutOutput.DragApplicationPointWorldM =
        BodyPointToWorld(
            Chassis,
            Config.DragApplicationPointBodyM);

    OutOutput.FrontLiftApplicationPointWorldM =
        BodyPointToWorld(
            Chassis,
            Config.FrontLiftApplicationPointBodyM);

    OutOutput.RearLiftApplicationPointWorldM =
        BodyPointToWorld(
            Chassis,
            Config.RearLiftApplicationPointBodyM);

    const double SpeedSquared =
        OutOutput.RelativeAirVelocityWorldMps.SizeSquared();

    if (SpeedSquared <= UE_DOUBLE_SMALL_NUMBER
        || Environment.AirDensityKgPerM3 <= 0.0)
    {
        return true;
    }

    const double SpeedMps =
        FMath::Sqrt(SpeedSquared);

    const FVector3d FlowDirectionWorld =
        OutOutput.RelativeAirVelocityWorldMps
        / SpeedMps;

    OutOutput.DynamicPressurePa =
        0.5
        * Environment.AirDensityKgPerM3
        * SpeedSquared;

    OutOutput.DragForceN =
        OutOutput.DynamicPressurePa
        * Config.ReferenceAreaM2
        * Config.DragCoefficient;

    OutOutput.FrontLiftForceN =
        OutOutput.DynamicPressurePa
        * Config.ReferenceAreaM2
        * Config.FrontLiftCoefficient;

    OutOutput.RearLiftForceN =
        OutOutput.DynamicPressurePa
        * Config.ReferenceAreaM2
        * Config.RearLiftCoefficient;

    const FVector3d UpWorld =
        FVector3d(
            Chassis.OrientationWorld.RotateVector(
                FVector::UpVector))
        .GetSafeNormal();

    OutOutput.DragForceWorldN =
        FlowDirectionWorld
        * OutOutput.DragForceN;

    OutOutput.FrontLiftForceWorldN =
        UpWorld
        * OutOutput.FrontLiftForceN;

    OutOutput.RearLiftForceWorldN =
        UpWorld
        * OutOutput.RearLiftForceN;

    OutOutput.TotalForceWorldN =
        OutOutput.DragForceWorldN
        + OutOutput.FrontLiftForceWorldN
        + OutOutput.RearLiftForceWorldN;

    OutOutput.TotalTorqueWorldNm =
        TorqueFromForceAtPoint(
            Chassis,
            OutOutput.DragForceWorldN,
            OutOutput.DragApplicationPointWorldM)
        + TorqueFromForceAtPoint(
            Chassis,
            OutOutput.FrontLiftForceWorldN,
            OutOutput.FrontLiftApplicationPointWorldM)
        + TorqueFromForceAtPoint(
            Chassis,
            OutOutput.RearLiftForceWorldN,
            OutOutput.RearLiftApplicationPointWorldM);

    return true;
}

bool TAAerodynamics::AddToChassis(
    const FTAAerodynamicsConfig& Config,
    const FTAAerodynamicsEnvironment& Environment,
    const FTAChassisState& Chassis,
    FTAChassisForceAccumulator& InOutAccumulator,
    FTAAerodynamicsOutput* OutOutput)
{
    FTAAerodynamicsOutput LocalOutput;

    if (!Calculate(
            Config,
            Environment,
            Chassis,
            LocalOutput))
    {
        return false;
    }

    TAChassisDynamics::AddForceAtWorldPoint(
        Chassis,
        LocalOutput.DragForceWorldN,
        LocalOutput.DragApplicationPointWorldM,
        InOutAccumulator);

    TAChassisDynamics::AddForceAtWorldPoint(
        Chassis,
        LocalOutput.FrontLiftForceWorldN,
        LocalOutput.FrontLiftApplicationPointWorldM,
        InOutAccumulator);

    TAChassisDynamics::AddForceAtWorldPoint(
        Chassis,
        LocalOutput.RearLiftForceWorldN,
        LocalOutput.RearLiftApplicationPointWorldM,
        InOutAccumulator);

    if (OutOutput != nullptr)
    {
        *OutOutput =
            LocalOutput;
    }

    return true;
}
