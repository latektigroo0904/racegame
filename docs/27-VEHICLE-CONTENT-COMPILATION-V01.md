# Vehicle Content Compilation v0.1

Status: implemented prototype baseline
Updated: 2026-09-23

## Purpose
Make the Unreal vehicle asset the source of authored vehicle physics while keeping the fixed-step solver free of mutable UObject reads.

Canonical path:

```
UTAVehicleDefinition
→ validate authored data
→ compile SI/runtime structures
→ FTAVehicleCompiledConfig
→ TAFourWheelVehicleRuntime
```

## Compiled package
`FTAVehicleCompiledConfig` now contains:
- identity/version;
- physical dimensions;
- mass/COM/inertia;
- `FTAVehicleRuntimeConfig`;
- `FTAFourWheelRuntimeConfig`;
- stable prototype `PhysicsConfigHash`.

## Authored physics currently represented

### Tire
- unloaded radius;
- reference load;
- reference pressure;
- dry peak friction seed;
- tread depth;
- radial stiffness;
- progressive radial stiffness;
- radial damping;
- maximum radial deflection.

Additional runtime tire coefficients still use shared solver defaults and should migrate into authored tire assets in later schema versions.

### Wheel/brake
- rotational inertia;
- maximum brake torque.

### Front suspension
- right-side double-wishbone hardpoints;
- travel limits;
- solver iterations/tolerance;
- spring preload/rate;
- bump/rebound damping;
- anti-roll rate/limit;
- steering rack travel/exponent/sign.

Left side is mirrored by the runtime.

### Rear suspension
- five right-side chassis pickups;
- five right-side upright pickups;
- wheel center;
- damper mounts;
- travel limits;
- solver iterations/tolerance;
- spring preload/rate;
- damping;
- anti-roll rate/limit.

Left side is mirrored by the runtime.

### Drivetrain
Prototype authoring currently includes:
- idle/redline/limiter;
- crank inertia;
- clutch torque capacity;
- forward ratios;
- reverse ratio;
- final drive;
- efficiency;
- front-vs-rear driven axle selection.

Full torque-curve, differential and thermal authoring remain later schema work.

## Coordinate convention
Authoring hardpoints are expressed in the vehicle reference frame.

Compilation converts hardpoints to COM-local coordinates:

```
P_com_local =
P_vehicle_local - COM_vehicle_local
```

The fixed-step chassis solver therefore uses COM-centered geometry consistently.

## Validation
Compilation rejects:
- missing definition ID;
- non-positive mass/inertia;
- non-four-wheel configuration in the current high-fidelity prototype;
- invalid tire radial parameters;
- invalid suspension travel ranges;
- invalid gear/final-drive data;
- degenerate front double-wishbone geometry;
- degenerate rear five-link geometry.

Warnings are produced when metadata and physical wheel-center geometry disagree materially:
- wheelbase;
- front track;
- rear track.

## Physics configuration hash
The current hash covers:
- identity/version;
- mass/COM/inertia;
- wheelbase/tracks;
- wheel count;
- tire force/compliance parameters;
- wheel inertia/brake torque/driven flags;
- powertrain ratios/core limits;
- front hardpoints/solver/suspension/anti-roll/rack;
- rear links/solver/suspension/anti-roll.

Use cases:
- regression trace identity;
- replay diagnostics;
- multiplayer configuration comparison;
- save migration diagnostics.

The current hash implementation is a prototype hash, not yet a cross-platform cryptographic content digest.

## End-to-end regression
Automation source now verifies:

```
NewObject<UTAVehicleDefinition>
→ BuildCompiledConfig
→ initialize four-wheel runtime
→ flat-road four-corner step
→ four contacts + compliant tire deflection
```

No hand-built physics config is required by this integration test.

## Known limitations
- only the high-fidelity four-wheel topology compiles today;
- front/rear can use one shared tire authoring block;
- full engine torque-map authoring is not yet exposed;
- aero/brake thermal/unsprung/structure assets are not yet compiled here;
- authored structural bindings still need schema integration.

## Acceptance
v0.1 is accepted when the default TA-P01 definition can compile into a complete four-wheel runtime, invalid suspension geometry fails validation, and a handling-critical data change changes the physics config hash.
