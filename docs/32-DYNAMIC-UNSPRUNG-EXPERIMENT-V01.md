# Dynamic Unsprung Corner Experiment V0.1

Updated: 2026-09-23
Status: source-level implementation complete; UE 5.8 execution pending.

## Purpose
Evaluate whether explicit unsprung vertical inertia can improve wheel-hop and road-step behavior without violating Torque Atlas force ownership or destabilizing the canonical four-wheel solver.

## Implemented experimental path
`TAExperimentalUnsprungCorner` composes the existing double-wishbone geometry, suspension runtime, tire radial model and `TAUnsprungVerticalDynamics` primitive. It is intentionally isolated from `TAFourWheelVehicleRuntime`.

The adapter exposes separately:
- tire-road normal force acting on the unsprung mass;
- suspension reaction transmitted to the chassis;
- generalized gravity and chassis-frame inertial terms;
- generalized force balance;
- tire radial deflection;
- unsprung travel, velocity and acceleration;
- explicit chassis-side suspension application point.

## Force-ownership invariant
The dynamic path must obey:

```
road/tire normal force -> unsprung mass
suspension spring/damper/stop -> equal/opposite unsprung/chassis reaction
```

It must never apply the tire normal force directly to the chassis in addition to the suspension reaction. `FTAWheelContactInput.VerticalLoadN` remains the tire load used by the tire-force solver, while `SuspensionForceWorldN` is the chassis force. In the dynamic path those values are intentionally allowed to differ.

## Existing source-level acceptance coverage
Automation coverage now exercises:
1. quasi-static seeding and first-step equilibrium;
2. a 20 mm road step and bounded transient;
3. convergence toward the raised-road quasi-static reference;
4. unsprung-weight force accounting under gravity;
5. explicit no-normal-force-double-count ownership;
6. chassis-side damper-mount force application;
7. deterministic replay for identical fixed inputs;
8. config validation including matching travel limits, positive tire stiffness and bounded internal substeps.

## Acceptance gate
The experiment is **not canonical** yet. Promotion requires actual UE 5.8 execution and recorded evidence for all of the following:
- Automation suite passes after UHT/UBT compilation;
- static equilibrium agrees with the quasi-static baseline within reviewed tolerance;
- 20 mm step response remains bounded and damps;
- no duplicated road-normal force reaches the chassis;
- travel-limit contact remains numerically stable;
- repeated fixed-input runs are deterministic;
- CPU cost is measured against canonical quasi-static contact;
- energy behavior is inspected for non-physical growth over a fixed no-input settle/road-step window.

## Decision
Keep quasi-static compliant contact as the production baseline. Keep the dynamic-unsprung implementation behind an experimental boundary until runtime measurements justify the extra state and cost. No production four-wheel integration is authorized by source-level assertions alone.

## Risks
- Source-level thresholds may be too loose or too strict once compiled with the real UE toolchain.
- The current one-corner experiment does not yet prove stability under axle anti-roll coupling, steering, combined slip or damaged geometry.
- CPU cost is unknown until profiled in-engine.
- A formal mechanical-energy diagnostic is still desirable before any production promotion.

## Next engineering target
Move remaining handling-critical tire and powertrain defaults into versioned authored vehicle content and include every promoted coefficient in `PhysicsConfigHash`. In parallel, the first available UE 5.8 environment should execute the verification harness and turn provisional assertions into measured baselines.
