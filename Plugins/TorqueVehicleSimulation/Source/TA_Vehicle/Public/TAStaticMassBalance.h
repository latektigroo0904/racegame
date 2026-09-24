#pragma once

#include "CoreMinimal.h"

struct TA_VEHICLE_API FTAStaticMassBalanceInput
{
    double MassKg = 0.0;
    double GravityMagnitudeMps2 = 9.80665;
    double WheelbaseM = 0.0;
    double FrontTrackM = 0.0;
    double RearTrackM = 0.0;

    // Longitudinal COM position measured forward from the rear axle.
    double ComFromRearAxleM = 0.0;

    // Vehicle-local lateral COM offset. +Y is right.
    double ComLateralM = 0.0;
};

struct TA_VEHICLE_API FTAStaticMassBalanceOutput
{
    double TotalWeightN = 0.0;
    double FrontAxleLoadN = 0.0;
    double RearAxleLoadN = 0.0;
    double FrontLeftLoadN = 0.0;
    double FrontRightLoadN = 0.0;
    double RearLeftLoadN = 0.0;
    double RearRightLoadN = 0.0;
};

namespace TAStaticMassBalance
{
    /**
     * Computes an analytical four-support static-equilibrium target.
     * This is a regression oracle only; callers must not feed the result
     * back into the dynamic suspension/contact solver.
     */
    TA_VEHICLE_API bool Calculate(
        const FTAStaticMassBalanceInput& Input,
        FTAStaticMassBalanceOutput& OutOutput);
}
