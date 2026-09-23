# Rear Multi-Link Geometry v0.1

Status: implementation specification
Updated: 2026-09-23

## 1. Goal
Implement the TA-P01 rear suspension as a true link-based mechanism rather than approximating it as a double wishbone.

Initial topology:
- five independent suspension links;
- one rigid upright/wheel carrier;
- one damper chassis mount;
- one damper upright mount.

## 2. Link model
Each link has:
- chassis-side pickup;
- upright-side reference pickup;
- fixed reference link length.

Runtime structural damage can move the chassis pickup.

The link then constrains:
```
|P_upright_runtime - P_chassis_runtime| =
ReferenceLinkLength
```

## 3. Rigid upright
Rigid reference points:
- five link upright pickups;
- wheel center;
- damper upright mount.

All pairwise reference distances are preserved by the positional solver.

This keeps the upright rigid without replacing individual links with fake upper/lower arms.

## 4. Degrees of freedom
A rigid upright has six rigid-body DOF.

The mechanism provides:
- five independent link-length constraints;
- one commanded/solved wheel-travel coordinate.

This closes the kinematic system in the prototype.

## 5. Travel
Positive travel = bump/compression.

Constraint:
```
WheelCenter.z =
WheelCenterReference.z + Travel
```

## 6. Solver
Prototype positional projection:
1. project each of five upright link points to its chassis pickup/link length;
2. project all upright reference-point pairs back to rigid reference distances;
3. project wheel center onto target travel Z;
4. iterate to tolerance.

Output includes:
- convergence state;
- maximum residual;
- wheel center;
- wheel orientation;
- camber;
- toe;
- damper length.

## 7. Upright orientation
Three non-collinear upright link points define reference/current bases.

The reference wheel-forward and wheel-up vectors are transformed through the basis.

This provides geometric:
- camber;
- toe;
- wheel axis orientation.

## 8. Structural damage
Per-link chassis pickup offset:
```
ChassisPickupRuntime =
ChassisPickupReference + StructuralOffset
```

Damper chassis mount may also move.

Later extensions:
- bent links/effective length changes;
- upright deformation;
- bushing compliance/play;
- subframe rigid displacement;
- bearing damage.

## 9. Damper
Damper upright mount is rigidly attached to the solved upright.

```
DamperLength =
|DamperUprightRuntime - DamperChassisRuntime|
```

Motion ratio can use the same finite-difference derivative strategy as the front prototype.

## 10. Mirroring
Right side may be authored as canonical.

Left-side chassis and upright reference points mirror through chassis centerline.

There is no steering-rack exception for the normal rear suspension.

Future rear-steer systems will add a dedicated toe-link actuator rather than abusing mirror logic.

## 11. TA-P01 handling intent
Seed geometry targets:
- modest negative camber gain in bump;
- stability-biased toe change;
- low unintended rear steer;
- progressive wheel path.

These are prototype intentions, not measured OEM data.

## 12. Tests
- reference geometry converges;
- bump/droop converge through valid travel;
- mirrored geometry remains symmetric;
- individual pickup displacement changes rear alignment;
- damper length changes with travel;
- degenerate/non-rigid reference geometry rejects;
- residual remains bounded.

## 13. Performance
Normal undamaged rear suspension can later receive a precomputed cache.

Damaged/high-fidelity vehicles use the runtime geometry solver.

## 14. Acceptance
v0.1 passes when the rear upright can be solved from five real link constraints and wheel travel, and structural pickup movement naturally changes wheel center, camber, toe and damper geometry.
