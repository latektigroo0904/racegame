#include "TAFrontAxleRuntime.h"

namespace
{
    bool CalculateNeutralToe(
        const FTADoubleWishboneSolverConfig& GeometryConfig,
        const FTADoubleWishboneDamageOffsets& Damage,
        const double TravelM,
        double& OutToeRad)
    {
        FTADoubleWishboneState State;
        FTADoubleWishboneSolveOutput Output;
        FTADoubleWishboneSolveInput Input;

        Input.TravelM = TravelM;
        Input.RackDisplacementM = 0.0;
        Input.Damage = Damage;

        if (!TADoubleWishboneSolver::Solve(
                GeometryConfig,
                Input,
                State,
                Output))
        {
            return false;
        }

        OutToeRad = Output.ToeRad;
        return true;
    }

    bool CalculateBumpSteer(
        const FTADoubleWishboneSolverConfig& GeometryConfig,
        const FTADoubleWishboneDamageOffsets& Damage,
        const double CurrentTravelM,
        double& OutBumpSteerRad)
    {
        double ReferenceToeRad = 0.0;
        double CurrentNeutralToeRad = 0.0;

        if (!CalculateNeutralToe(
                GeometryConfig,
                Damage,
                0.0,
                ReferenceToeRad) ||
            !CalculateNeutralToe(
                GeometryConfig,
                Damage,
                CurrentTravelM,
                CurrentNeutralToeRad))
        {
            return false;
        }

        OutBumpSteerRad =
            CurrentNeutralToeRad - ReferenceToeRad;

        return true;
    }
}

double TAFrontAxleRuntime::CalculateRackDisplacementM(
    const FTASteeringRackRuntimeConfig& Config,
    const double Steering01)
{
    const double Input =
        FMath::Clamp(Steering01, -1.0, 1.0);

    const double Magnitude =
        FMath::Pow(
            FMath::Abs(Input),
            FMath::Max(0.05, Config.InputExponent));

    const double ShapedInput =
        FMath::Sign(Input) * Magnitude;

    return
        ShapedInput
        * FMath::Max(0.0, Config.MaxRackDisplacementM)
        * FMath::Clamp(Config.SteeringSign, -1.0, 1.0);
}

bool TAFrontAxleRuntime::Resolve(
    const FTAChassisState& Chassis,
    const FTAFrontAxleRuntimeConfig& Config,
    const FTAFrontAxleSolveInput& Input,
    const double LeftWheelRadiusM,
    const double RightWheelRadiusM,
    const double DeltaTimeSeconds,
    FTAFrontAxleRuntimeState& InOutState,
    FTAFrontAxleSolveOutput& OutOutput)
{
    OutOutput = FTAFrontAxleSolveOutput{};

    if (DeltaTimeSeconds <= 0.0 ||
        LeftWheelRadiusM <= UE_DOUBLE_SMALL_NUMBER ||
        RightWheelRadiusM <= UE_DOUBLE_SMALL_NUMBER ||
        !TADoubleWishboneSolver::ValidateConfig(Config.RightGeometry))
    {
        return false;
    }

    const FTADoubleWishboneSolverConfig LeftGeometry =
        TADoubleWishboneSolver::MirrorAcrossCenterline(
            Config.RightGeometry);

    if (!TADoubleWishboneSolver::ValidateConfig(LeftGeometry))
    {
        return false;
    }

    const double RackDisplacementM =
        CalculateRackDisplacementM(
            Config.SteeringRack,
            Input.Steering01);

    if (!TAWheelContactResolver::ResolveDoubleWishboneRoadContact(
            Chassis,
            LeftGeometry,
            Config.LeftSuspension,
            LeftWheelRadiusM,
            RackDisplacementM,
            Input.LeftDamage,
            Input.LeftRoad,
            DeltaTimeSeconds,
            InOutState.LeftGeometry,
            InOutState.LeftSuspension,
            OutOutput.LeftContact))
    {
        return false;
    }

    if (!TAWheelContactResolver::ResolveDoubleWishboneRoadContact(
            Chassis,
            Config.RightGeometry,
            Config.RightSuspension,
            RightWheelRadiusM,
            RackDisplacementM,
            Input.RightDamage,
            Input.RightRoad,
            DeltaTimeSeconds,
            InOutState.RightGeometry,
            InOutState.RightSuspension,
            OutOutput.RightContact))
    {
        return false;
    }

    TAWheelContactResolver::ApplyAntiRollBarToPair(
        Config.AntiRollBar,
        OutOutput.LeftContact,
        OutOutput.RightContact);

    OutOutput.LeftVehicleContact =
        TAWheelContactResolver::BuildVehicleWheelContactInput(
            OutOutput.LeftContact);

    OutOutput.RightVehicleContact =
        TAWheelContactResolver::BuildVehicleWheelContactInput(
            OutOutput.RightContact);

    OutOutput.RackDisplacementM =
        RackDisplacementM;

    OutOutput.LeftSteeringAngleRad =
        OutOutput.LeftContact.Geometry.ToeRad;

    OutOutput.RightSteeringAngleRad =
        OutOutput.RightContact.Geometry.ToeRad;

    if (!CalculateBumpSteer(
            LeftGeometry,
            Input.LeftDamage,
            OutOutput.LeftContact.TravelM,
            OutOutput.LeftBumpSteerRad) ||
        !CalculateBumpSteer(
            Config.RightGeometry,
            Input.RightDamage,
            OutOutput.RightContact.TravelM,
            OutOutput.RightBumpSteerRad))
    {
        return false;
    }

    const double AverageSteeringAngleRad =
        0.5
        * (OutOutput.LeftSteeringAngleRad
            + OutOutput.RightSteeringAngleRad);

    if (FMath::Abs(AverageSteeringAngleRad)
        > 1.0e-7)
    {
        if (AverageSteeringAngleRad > 0.0)
        {
            // Right turn: right wheel is inside.
            OutOutput.AckermannDeltaRad =
                FMath::Abs(
                    OutOutput.RightSteeringAngleRad)
                - FMath::Abs(
                    OutOutput.LeftSteeringAngleRad);
        }
        else
        {
            // Left turn: left wheel is inside.
            OutOutput.AckermannDeltaRad =
                FMath::Abs(
                    OutOutput.LeftSteeringAngleRad)
                - FMath::Abs(
                    OutOutput.RightSteeringAngleRad);
        }
    }

    OutOutput.bSolved = true;
    return true;
}
