# Aerodynamics v0.1

Updated: 2026-09-23

## Purpose
Add physically applied aerodynamic force and moment without introducing a speed-dependent tire-grip multiplier.

## Coordinate/ownership decisions
- Chassis state is world-space SI; authored aero application point is vehicle/body-local metres relative to chassis COM.
- Relative air velocity is `wind_world - vehicle_velocity_world`.
- Dynamic pressure is `q = 0.5 * rho * |Vrel|^2`.
- Drag magnitude is `q * A * Cd` and follows relative airflow.
- Lift magnitude is `q * A * Cl` along chassis up; negative `Cl` therefore produces downforce.
- The resultant is applied at an authored physical point. Pitch/yaw/roll moments arise from `r x F`; no synthetic moment multiplier is used in v0.1.
- Environment owns air density and wind. Vehicle content owns area, Cd, Cl and application point.

## Implemented deliverables
- `TAAerodynamics.h/.cpp`: deterministic, allocation-free solver and chassis-accumulator adapter.
- Automation regressions for zero relative speed, V-squared scaling, headwind/tailwind, negative-Cl downforce, and off-center pitch moment.

## Assumptions
- v0.1 uses one resultant application point and one scalar lift coefficient.
- Crosswind drag follows the full relative-air vector; a dedicated side-force/yaw coefficient model is deferred.
- Ground effect, ride-height sensitivity, active aero, diffuser stall and aero damage are deferred until the base executable model is measured.

## Risks
- This source has not yet passed UHT/UBT/UE Automation.
- A single application point cannot independently calibrate front/rear aero balance over attitude changes.
- Chassis-up lift is intentionally simple; high-angle flow will require body-axis coefficient decomposition.

## Next integration checkpoint
1. Promote aero config into authored vehicle definition and compiled runtime.
2. Include effective aero values in `PhysicsConfigHash` and validation.
3. Add environment aero input to the vehicle step.
4. Apply aero to the same chassis force accumulator before 6-DOF integration.
5. Expose q/drag/lift/aero moments in telemetry and regression reports.
6. Re-run source-sanity and keep UE 5.8 executable verification as the external acceptance gate.
