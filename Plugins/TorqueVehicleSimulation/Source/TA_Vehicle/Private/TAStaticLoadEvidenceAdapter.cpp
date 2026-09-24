#include "TAStaticLoadEvidenceAdapter.h"

#include <cmath>

namespace
{
    bool IsFiniteNonNegative(double Value)
    {
        return std::isfinite(Value) && Value >= 0.0;
    }

    bool IsFiniteVector(const FVector3d& Value)
    {
        return std::isfinite(Value.X) && std::isfinite(Value.Y) && std::isfinite(Value.Z);
    }
}

bool TAStaticLoadEvidenceAdapter::BuildSample(
    const FTAStaticLoadEvidenceAdapterConfig& Config,
    const FTAFourWheelRuntimeState& RuntimeState,
    const FTAFourWheelStepOutput& StepOutput,
    double DeltaTimeSeconds,
    FTAStaticLoadSample& OutSample)
{
    OutSample = FTAStaticLoadSample{};

    if (!std::isfinite(Config.ExpectedDeltaTimeSeconds) || Config.ExpectedDeltaTimeSeconds <= 0.0 ||
        !std::isfinite(Config.DeltaTimeToleranceSeconds) || Config.DeltaTimeToleranceSeconds < 0.0 ||
        !std::isfinite(DeltaTimeSeconds) || DeltaTimeSeconds <= 0.0 ||
        std::abs(DeltaTimeSeconds - Config.ExpectedDeltaTimeSeconds) > Config.DeltaTimeToleranceSeconds ||
        !StepOutput.bContactsSolved ||
        !IsFiniteVector(RuntimeState.Vehicle.Chassis.LinearVelocityWorldMps) ||
        !IsFiniteVector(RuntimeState.Vehicle.Chassis.AngularVelocityWorldRadPerSec))
    {
        return false;
    }

    const double FrontLeftLoadN = StepOutput.FrontAxle.LeftVehicleContact.VerticalLoadN;
    const double FrontRightLoadN = StepOutput.FrontAxle.RightVehicleContact.VerticalLoadN;
    const double RearLeftLoadN = StepOutput.RearAxle.LeftVehicleContact.VerticalLoadN;
    const double RearRightLoadN = StepOutput.RearAxle.RightVehicleContact.VerticalLoadN;

    if (!IsFiniteNonNegative(FrontLeftLoadN) || !IsFiniteNonNegative(FrontRightLoadN) ||
        !IsFiniteNonNegative(RearLeftLoadN) || !IsFiniteNonNegative(RearRightLoadN))
    {
        return false;
    }

    OutSample.ChassisLinearSpeedMps = RuntimeState.Vehicle.Chassis.LinearVelocityWorldMps.Length();
    OutSample.ChassisAngularSpeedRadPerSec = RuntimeState.Vehicle.Chassis.AngularVelocityWorldRadPerSec.Length();
    OutSample.FrontLeftLoadN = FrontLeftLoadN;
    OutSample.FrontRightLoadN = FrontRightLoadN;
    OutSample.RearLeftLoadN = RearLeftLoadN;
    OutSample.RearRightLoadN = RearRightLoadN;
    return true;
}
