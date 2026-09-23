# Aerodynamics Telemetry Integration V0.1

Updated: 2026-09-23

## Purpose

Close the observability side of the aerodynamic pipeline without introducing a second physics calculation path.

## Canonical ownership

`FTAVehicleStepOutput::Aerodynamics` is the only source accepted by telemetry. `TA_Telemetry` must not invoke `TAAerodynamics::Calculate` or reconstruct drag/lift from speed, Cd, Cl, area or air density.

This preserves the invariant that diagnostics report the force and torque that the vehicle step actually applied to the chassis accumulator.

## Added telemetry channels

`FTATelemetrySample` now retains:

- relative air speed magnitude in m/s, derived only from the exact output relative-air-velocity vector;
- dynamic pressure in Pa;
- applied aerodynamic force vector in world N;
- applied aerodynamic torque vector in world N*m.

The vector channels are deliberately retained instead of only scalar drag/downforce because they remain valid under wind, yaw and future non-axis-aligned aero models.

## Regression coverage

`TorqueAtlas.Telemetry.Aerodynamics.CapturesAppliedStepOutput` constructs a known `FTAVehicleStepOutput`, converts it through `TATelemetry::MakeSample`, and checks exact force/torque/dynamic-pressure propagation plus relative-air-speed magnitude.

## Decisions

1. Telemetry is a consumer, never an aerodynamic solver.
2. World-space vectors are stored because that is the space of the applied chassis force accumulator.
3. No new authored coefficient is introduced by telemetry.
4. No baseline envelope is promoted until UE 5.8 executes the full canonical vehicle step.

## Risks

- The vehicle step still needs the canonical `TAVehicleAerodynamicsBridge::AddToChassis` call immediately before chassis integration. Until that is wired, these telemetry channels correctly remain zero in the ordinary full-step path.
- Regression/report serializers may need explicit vector-component fields if the machine-readable report format remains scalar-only.
- UE 5.8 compile/Automation execution is still required before these source-level changes can be accepted as executable evidence.

## Deliverables

- aerodynamic fields in `FTATelemetrySample`;
- exact-copy implementation in `TATelemetry::MakeSample`;
- Automation regression for aero telemetry capture;
- updated active checkpoint.

## Next checkpoint

Finish the canonical aero physics call chain:

`UTAVehicleDefinition.Aerodynamics -> validation -> COM-local compile -> PhysicsConfigHash -> TAVehicleAerodynamicsBridge::AddToChassis -> TAChassisDynamics::Integrate -> FTAVehicleStepOutput::Aerodynamics -> TATelemetry::MakeSample`.

After that, add a full-step regression proving speed/wind produces drag/downforce/moment and changes the 6-DOF chassis state, then extend machine-readable regression metrics with scalar aero channels.
