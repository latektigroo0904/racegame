#pragma once

#include "CoreMinimal.h"

enum class ETADifferentialType : uint8
{
    Open,
    Spool,
    ClutchLsd,
    Helical,
    Viscous,
    Active
};

struct TA_POWERTRAIN_API FTADifferentialConfig
{
    ETADifferentialType Type = ETADifferentialType::Open;

    double MechanicalEfficiency01 = 0.98;

    // Finite lock/bias actuator parameters.
    double PreloadTorqueNm = 40.0;
    double LockStiffnessNmsPerRad = 25.0;
    double DriveRampGain = 0.20;
    double CoastRampGain = 0.10;
    double MaxLockTorqueNm = 500.0;

    double TorqueBiasRatio = 3.0;

    double ViscousCoefficientNmsPerRad = 8.0;
    double ViscousMaxTorqueNm = 300.0;

    double ActiveMaxBiasTorqueNm = 400.0;
};

struct TA_POWERTRAIN_API FTADifferentialInput
{
    double InputTorqueNm = 0.0;

    double LeftAngularSpeedRadPerSec = 0.0;
    double RightAngularSpeedRadPerSec = 0.0;

    double LeftReactionCapacityNm = 1.0e9;
    double RightReactionCapacityNm = 1.0e9;

    // Positive request biases torque toward the right side.
    double ActiveBiasRequestNm = 0.0;

    bool bCoast = false;
};

struct TA_POWERTRAIN_API FTADifferentialOutput
{
    double LeftWheelTorqueNm = 0.0;
    double RightWheelTorqueNm = 0.0;

    // Equal/opposite internal side-to-side transfer torque.
    // Positive value biases the right side.
    double BiasTorqueNm = 0.0;

    double UntransmittedInputTorqueNm = 0.0;

    double RelativeAngularSpeedRadPerSec = 0.0;
    double DissipatedPowerW = 0.0;
};

namespace TADifferentialSolver
{
    TA_POWERTRAIN_API bool ValidateConfig(
        const FTADifferentialConfig& Config);

    TA_POWERTRAIN_API bool Solve(
        const FTADifferentialConfig& Config,
        const FTADifferentialInput& Input,
        FTADifferentialOutput& OutOutput);
}
