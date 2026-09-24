# Static Mass Balance v0.1

Status: implementation contract
Updated: 2026-09-24

## Purpose
Close Proof-of-Physics P1.1 with a deterministic, solver-independent reference calculation before world/contact integration.

The reference result is not a suspension replacement. It is the analytical equilibrium target used to verify that the settled vehicle carries the correct total weight and front/rear/left/right distribution.

## Coordinate convention
Canonical vehicle-local axes:
- +X forward;
- +Y right;
- +Z up.

All dimensions are meters and forces are newtons.

## Inputs
- mass `m`;
- gravity magnitude `g`;
- wheelbase `L`;
- front track `Tf`;
- rear track `Tr`;
- COM longitudinal coordinate relative to rear axle `x` (0 at rear axle, L at front axle);
- COM lateral coordinate `y` relative to centerline (+ right).

For the first reference implementation, lateral load fraction is derived independently per axle from that axle's track. This is an analytical four-support target, not a claim about transient roll stiffness distribution.

## Equilibrium
Total weight:

`W = m g`

Front and rear axle loads:

`Wf = W * x / L`

`Wr = W - Wf`

For an axle with track `T`, right-side fraction is:

`fr = 0.5 + y / T`

and left-side fraction is `1 - fr`.

Therefore:
- `FL = Wf * (0.5 - y/Tf)`
- `FR = Wf * (0.5 + y/Tf)`
- `RL = Wr * (0.5 - y/Tr)`
- `RR = Wr * (0.5 + y/Tr)`

## Validation
Reject the reference calculation when:
- mass or gravity is non-finite or <= 0;
- wheelbase/front track/rear track is non-finite or <= 0;
- `x` is outside `[0, L]`;
- `abs(y)` is greater than or equal to half of either track.

The lateral bound deliberately prevents a reference target that requires a negative static corner load.

## Required invariants
1. `FL + FR + RL + RR == W` within Class-A numerical tolerance.
2. `FL + FR == Wf` and `RL + RR == Wr`.
3. Centered COM produces left/right symmetry.
4. Moving COM forward monotonically increases front axle load.
5. Positive Y COM offset increases right-side load.
6. No valid input produces a negative corner load.

## TA-P01 baseline seed
Use current authored mass/geometry only as a seed; do not freeze commercial calibration from it.

For centered lateral COM and longitudinal COM at 55% of wheelbase from the rear axle:
- front axle target = 55% of weight;
- rear axle target = 45%;
- each front corner = 27.5%;
- each rear corner = 22.5%.

This seed is intentionally simple so sign and axle-order mistakes are obvious.

## Runtime integration rule
The analytical target must remain separate from the dynamic suspension solver. A proving-ground P1.1 test will:
1. compile the vehicle definition;
2. derive the analytical static target;
3. spawn and settle the dynamic vehicle on a flat symmetric surface;
4. average wheel normal loads over a stable window;
5. compare total/axle/corner loads against the analytical target using a dynamic-regression tolerance.

Do not inject the analytical result back into the physics state to force the test to pass.

## Telemetry requirements
Record:
- physics config hash;
- mass and gravity;
- COM;
- wheelbase/tracks;
- analytical FL/FR/RL/RR;
- measured mean FL/FR/RL/RR;
- total weight error;
- front/rear distribution error;
- left/right symmetry error for centered setups;
- settle duration and sample window.

## Risks
- Suspension preload/static spring compression may currently encode assumptions inconsistent with the authored COM. The P1.1 gate should expose this rather than compensate for it.
- Tire vertical compliance adds another equilibrium layer; measured contact loads should still converge to the same total/axle equilibrium.
- Unequal track widths mean a single lateral percentage is not sufficient; the analytical formula intentionally uses each axle track independently.

## Deliverable sequence
1. Implement a pure `TAStaticMassBalance` helper with no UObject/world dependency.
2. Add Class-A Automation tests for all invariants and invalid inputs.
3. Add a telemetry comparison helper for analytical versus measured corner loads.
4. Only then build the world-settle P1.1 regression.

## Checkpoint
Next implementation target after canonical aero source closure is executable verification. If Unreal verification remains externally unavailable, `TAStaticMassBalance` is the next independent Proof-of-Physics implementation slice.