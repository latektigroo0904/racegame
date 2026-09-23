#include "TAAerodynamics.h"

bool TAAerodynamics::ValidateConfig(const FTAAerodynamicsConfig& Config)
{
    return
        FMath::IsFinite(Config.ReferenceAreaM2) &&
        Config.ReferenceAreaM2 > 0.0 &&
        FMath::IsFinite(Config.DragCoefficient) &&
        Config.DragCoefficient >= 0.0 &&
        FMath::IsFinite(Config.LiftCoefficient) &&
        !Config.ApplicationPointBodyM.ContainsNaN();
}

bool TAAerodynamics::Calculate(
    const FTAAerodynamicsConfig& Config,
    const FTAAerodynamicsEnvironment& Environment,
    const FTAChassisState& Chassis,
    FTAAerodynamicsOutput& OutOutput)
{
    OutOutput = FTAAerodynamicsOutput{};

    if (!ValidateConfig(Config) ||
        !FMath::IsFinite(Environment.AirDensityKgPerM3) ||
        Environment.AirDensityKgPerM3 < 0.0 ||
        Environment.WindVelocityWorldMps.ContainsNaN())
    {
        return false;
    }

    // Air velocity relative to vehicle. A stationary car in a +X wind sees +X air flow;
    // a car travelling +X in still air sees -X air flow.
    OutOutput.RelativeAirVelocityWorldMps =
        Environment.WindVelocityWorldMps - Chassis.LinearVelocityWorldMps;

    const double SpeedSquared =
        OutOutput.RelativeAirVelocityWorldMps.SquaredLength();

    OutOutput.ApplicationPointWorldM =
        Chassis.PositionWorldM +
        FVector3d(Chassis.OrientationWorld.RotateVector(
            FVector(Config.ApplicationPointBodyM)));

    if (SpeedSquared <= UE_DOUBLE_SMALL_NUMBER ||
        Environment.AirDensityKgPerM3 <= 0.0)
    {
        return true;
    }

    const double SpeedMps = FMath::Sqrt(SpeedSquared);
    const FVector3d FlowDirectionWorld =
        OutOutput.RelativeAirVelocityWorldMps / SpeedMps;

    OutOutput.DynamicPressurePa =
        0.5 * Environment.AirDensityKgPerM3 * SpeedSquared;

    OutOutput.DragForceN =
        OutOutput.DynamicPressurePa *
        Config.ReferenceAreaM2 *
        Config.DragCoefficient;

    OutOutput.LiftForceN =
        OutOutput.DynamicPressurePa *
        Config.ReferenceAreaM2 *
        Config.LiftCoefficient;

    // Drag follows the relative air-flow vector. Lift uses chassis up so authored
    // negative Cl creates physical downforce without an arcade grip multiplier.
    const FVector3d UpWorld =
        FVector3d(Chassis.OrientationWorld.RotateVector(FVector::UpVector)).GetSafeNormal();

    OutOutput.ForceWorldN =
        FlowDirectionWorld * OutOutput.DragForceN +
        UpWorld * OutOutput.LiftForceN;

    const FVector3d LeverWorldM =
        OutOutput.ApplicationPointWorldM - Chassis.PositionWorldM;

    OutOutput.TorqueWorldNm =
        FVector3d::CrossProduct(LeverWorldM, OutOutput.ForceWorldN);

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
    if (!Calculate(Config, Environment, Chassis, LocalOutput))
    {
        return false;
    }

    TAChassisDynamics::AddForceAtWorldPoint(
        Chassis,
        LocalOutput.ForceWorldN,
        LocalOutput.ApplicationPointWorldM,
        InOutAccumulator);

    if (OutOutput != nullptr)
    {
        *OutOutput = LocalOutput;
    }

    return true;
}
