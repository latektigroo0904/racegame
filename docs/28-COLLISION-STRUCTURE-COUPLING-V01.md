# Collision / Structure Coupling v0.1

Status: implemented solver baseline
Updated: 2026-09-23

## Purpose
Connect collision impulses to both rigid vehicle motion and deformable structure without counting the same external momentum twice.

## Core rule
The external collision impulse belongs to the chassis rigid-body state.

```
J_external
→ chassis linear velocity
→ chassis angular velocity
```

Structural deformation receives a separate internal-mode excitation derived from the same collision event, but its rigid translation and rigid rotation modes are removed before node velocities are changed.

## Chassis impulse
At world contact point `P`:

```
DeltaV = J / M
L = (P - COM) × J
DeltaOmega_body = I_body^-1 * L_body
```

Implemented by:
`TAChassisDynamics::ApplyImpulseAtWorldPoint`.

## Structure impact distribution
Inputs are transformed into chassis-local/structure-local coordinates:
- contact point;
- collision impulse.

Nearby non-pinned nodes are selected inside a configurable distribution radius.

Raw spatial weight:

```
w =
(1 - distance / radius)^falloff
```

The configured deformation impulse fraction seeds a candidate node impulse field.

## Rigid-mode removal

### Translation
Candidate node impulses are corrected so:

```
sum(J_i) = 0
```

Mass-proportional subtraction removes rigid translation.

### Rotation
Around the affected-node mass centroid:

```
L_internal =
sum(r_i × J_i)
```

A point-mass inertia tensor is built.

Solve:
```
I * omega_equivalent = L_internal
```

Then subtract the corresponding rigid rotational impulse field:

```
J_rot_i =
m_i * (omega_equivalent × r_i)
```

After correction:
```
sum(r_i × J_i) ≈ 0
```

The remaining field contains internal deformation modes rather than another chassis kick.

## Velocity safety
If any node would exceed the configured impact delta-velocity ceiling, all internal impulses are scaled uniformly.

Uniform scaling preserves the zero rigid linear/angular mode conditions.

## Collision coupler
`TACollisionStructureCoupling`:
1. applies the full external world impulse once to the chassis;
2. converts contact/impulse to chassis-local coordinates;
3. distributes internal structural deformation;
4. reports rigid velocity deltas and structure energy/residuals.

A structure-distribution failure does not undo a valid chassis collision response.

## Telemetry
Structural impact output reports:
- affected node count;
- requested deformation impulse magnitude;
- total internal impulse L1 magnitude;
- injected deformation kinetic energy;
- residual linear impulse;
- residual angular impulse.

## Tests
Regression source covers:
- center chassis impulse: correct `J/m`, no rotation;
- off-center chassis impulse: angular response;
- multi-node deformation excitation;
- near-zero net structural linear momentum;
- near-zero net structural angular momentum;
- non-uniform spatial deformation mode;
- uniform velocity-limit scaling;
- chassis receives full external impulse while structure adds no duplicate rigid momentum.

## Important limitation
This is not yet a full contact manifold solver.

Missing:
- collision detection/manifold generation;
- normal/tangent impulse solve;
- restitution/friction impulse iteration;
- continuous collision detection;
- impact geometry/material metadata;
- direct fracture-energy calibration;
- deformable collision shape feedback.

## Next coupling
The next layer should convert structural solver results into:
- fracture damage signals;
- mount displacement events;
- radiator/body/mechanical component impact energy;
- crash sandbox telemetry.

## Acceptance
v0.1 is accepted when one collision event can create correct chassis momentum plus a localized internal deformation mode while the structural node field contributes effectively zero duplicate rigid linear/angular momentum.
