#include "TAAeroRegressionMetrics.h"

namespace TAAeroRegressionMetrics
{
    bool Extract(
        const FTATelemetrySample& Sample,
        const FVector3d& ForwardWorld,
        const FVector3d& UpWorld,
        FTAAeroRegressionMetrics& OutMetrics)
    {
        OutMetrics = FTAAeroRegressionMetrics{};

        const FVector3d Forward = ForwardWorld.GetSafeNormal();
        const FVector3d Up = UpWorld.GetSafeNormal();

        if (Forward.IsNearlyZero() || Up.IsNearlyZero() ||
            FMath::Abs(FVector3d::DotProduct(Forward, Up)) > 1.0e-3 ||
            !FMath::IsFinite(Sample.AeroRelativeAirSpeedMps) ||
            !FMath::IsFinite(Sample.AeroDynamicPressurePa) ||
            !Sample.AeroForceWorldN.IsFinite() ||
            !Sample.AeroTorqueWorldNm.IsFinite())
        {
            return false;
        }

        OutMetrics.RelativeAirSpeedMps = Sample.AeroRelativeAirSpeedMps;
        OutMetrics.DynamicPressurePa = Sample.AeroDynamicPressurePa;
        OutMetrics.ForceMagnitudeN = Sample.AeroForceWorldN.Length();

        // Positive drag means force opposing the canonical forward axis.
        OutMetrics.DragAxisForceN = -FVector3d::DotProduct(
            Sample.AeroForceWorldN,
            Forward);

        // Positive vertical force follows the supplied up axis; downforce is negative.
        OutMetrics.VerticalForceN = FVector3d::DotProduct(
            Sample.AeroForceWorldN,
            Up);

        const FVector3d Right = FVector3d::CrossProduct(Up, Forward).GetSafeNormal();
        OutMetrics.PitchTorqueNm = FVector3d::DotProduct(
            Sample.AeroTorqueWorldNm,
            Right);

        return true;
    }
}
