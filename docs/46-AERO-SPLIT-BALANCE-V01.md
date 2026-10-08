# Aerodynamic Split Balance v0.1

Updated: 2026-10-08  
Status: source-implemented, source-sanity green, UE 5.8 build/Automation unverified

## Purpose

Replace the earlier single-lift-coefficient/single-resultant simplification with a source-level model that can represent front/rear aerodynamic balance without introducing arcade speed grip.

The model remains deliberately compact:

- one vehicle reference area;
- one drag coefficient;
- one drag application point;
- one front lift/downforce coefficient and application point;
- one rear lift/downforce coefficient and application point;
- world-space wind and air density supplied by the step environment.

No tire friction coefficient is modified directly by speed.

## Canonical equations

Relative air velocity:

```
V_rel_world = V_wind_world - V_vehicle_world
```

Dynamic pressure:

```
q = 0.5 × rho × |V_rel|²
```

Drag magnitude:

```
F_drag = q × A × Cd
```

Front lift/downforce:

```
F_front_lift = q × A × Cl_front
```

Rear lift/downforce:

```
F_rear_lift = q × A × Cl_rear
```

Negative lift coefficients produce downforce in the current sign convention.

## Force directions

Drag follows the relative-airflow vector.

Front and rear lift/downforce act along chassis up/down.

The three forces are applied independently at their authored physical application points.

Total aerodynamic force:

```
F_total =
F_drag
+ F_front_lift
+ F_rear_lift
```

Total moment is not authored directly:

```
M_total =
r_drag × F_drag
+ r_front × F_front_lift
+ r_rear × F_rear_lift
```

Therefore pitch, roll and yaw emerge from force location and airflow rather than synthetic moment multipliers.

## Coordinate ownership

Authoring points are vehicle-origin-local.

Compilation subtracts vehicle COM exactly once.

Runtime points are COM-local.

During each vehicle step the runtime converts COM-local points into world-space from the current chassis transform.

This means equivalent authored coordinate systems that shift both vehicle origin and COM by the same vector preserve the effective aero physics/hash contribution.

## Runtime ownership

`FTAVehicleRuntimeConfig::Aerodynamics` owns effective compiled calibration.

`FTAVehicleStepInput::AerodynamicsEnvironment` owns transient:

- air density;
- wind velocity.

`TAVehicleAerodynamicsBridge` is the single vehicle-step integration boundary.

Aero is accumulated into the same `FTAChassisForceAccumulator` as tires, suspension and aligning moments before the single chassis integration.

## Grip invariant

There is no:

- speed-dependent grip multiplier;
- direct tire-mu increase from vehicle speed;
- separate “aero grip” scalar.

Any additional tire load must emerge through the physical chassis/suspension/contact solution after aerodynamic force acts on the vehicle.

## Front/rear balance

For downforce-only cases the compact telemetry/regression layer exposes:

```
front balance
=
front downforce
/
(front downforce + rear downforce)
```

When there is no downforce, the diagnostic balance defaults to 0.5 rather than producing NaN/divide-by-zero.

This balance channel is diagnostic only and does not feed physics.

## Telemetry

Compact telemetry now records the exact applied aero result:

- relative air speed;
- dynamic pressure;
- drag force magnitude;
- front lift/downforce force;
- rear lift/downforce force;
- total aero force vector;
- total aero torque vector.

Telemetry never recomputes aerodynamics.

## Regression/reporting

`TAAeroRegressionMetrics` exposes:

- relative air speed;
- dynamic pressure;
- total force magnitude;
- drag-axis force;
- vertical force;
- front lift/downforce;
- rear lift/downforce;
- front downforce balance;
- pitch torque.

`TAAeroRegressionReport` summarizes those channels with min/max/mean/steady-state values and exports JSON Lines / CSV-compatible machine-readable data.

## Source regressions

Current source coverage includes:

- zero relative speed → zero aero;
- drag scales with speed squared;
- headwind increases dynamic pressure/drag;
- negative lift coefficient produces downforce;
- off-COM drag produces pitch moment;
- split front/rear lift is applied at separate physical points;
- front-heavy downforce produces nose-down pitch torque;
- symmetric front/rear downforce cancels lift-induced pitch torque for symmetric points;
- authored non-default aero compiles into COM-local runtime state;
- effective aero changes alter `PhysicsConfigHash`;
- invalid authoring rejects compilation;
- vehicle full-step applies aero before chassis integration;
- compact telemetry copies applied aero output exactly;
- machine-readable report preserves front/rear balance.

Latest source-sanity evidence for the split-balance path:
- commit `ac1f0bd1f8b57923d695b0e4abe89ca6a36d60d5`;
- GitHub Actions run 220;
- conclusion: success.

## Explicit limits

This source model is not yet validated real-world aero calibration.

Still deferred:

- ride-height-dependent aero maps;
- pitch/yaw-sensitive coefficient maps;
- ground effect;
- diffuser stall;
- active aero;
- DRS;
- radiator/cooling drag coupling;
- detailed crosswind side-force/yaw coefficients;
- body damage modifying aero;
- detachable wings/splitters;
- transient wake/turbulence;
- drafting/slipstream;
- CFD-derived maps.

## Acceptance state

The source architecture is closed enough to proceed, but it is not an executable acceptance result.

The required external gate remains:

```
UE 5.8 UHT
→ UBT/C++ build
→ Editor/module load
→ Automation RunTest TorqueAtlas.
→ report validation
```

Do not label the aero model validated until that pipeline has run and real numerical traces have been reviewed.
