# Unsprung Vertical Dynamics v0.1

Status: isolated solver specification
Updated: 2026-09-23

## 1. Purpose
Move beyond the transitional massless-unsprung force equilibrium.

Each wheel/upright assembly receives an explicit generalized suspension-travel state:
- travel;
- travel velocity;
- travel acceleration;
- unsprung effective mass.

Positive travel remains bump/upward relative to the chassis.

## 2. Force convention
On the unsprung assembly:

```
+Q_tire       road/tire normal reaction toward bump
-Q_suspension spring/damper/stop reaction toward droop
```

Relative-coordinate acceleration also depends on:
- gravity projected onto the travel axis;
- acceleration of the chassis reference frame.

Prototype equation:

```
q_ddot =
(F_tire - F_suspension + F_external) / m_unsprung
+ g_along_axis
- a_chassis_along_axis
```

## 3. Integration
Semi-implicit:

```
q_dot_next = q_dot + q_ddot * dt
q_next = q + q_dot_next * dt
```

Then enforce finite suspension travel.

Travel-limit impact is not treated as an infinite constraint:
- velocity can be zeroed or weakly restituted;
- limit-hit state is reported;
- later mount/stop damage can consume the impact energy.

## 4. Why isolated first
The current canonical contact model solves quasi-static:

```
F_tire ≈ F_suspension
```

That model remains valuable as:
- initialization/static settle helper;
- lower-frequency LOD;
- reference result.

The dynamic unsprung solver is added first as a separately testable primitive before changing the whole vehicle step.

## 5. Required coupling
Future high-fidelity per-corner step:

1. read current unsprung travel;
2. solve suspension geometry at that travel;
3. compute tire-road radial deflection;
4. calculate tire vertical force;
5. calculate spring/damper/stop force;
6. integrate unsprung travel velocity/state;
7. update geometry/contact;
8. apply suspension reaction to chassis;
9. pass tire normal load to longitudinal/lateral tire solver.

## 6. Chassis coupling
Once dynamic unsprung state is canonical:
- chassis receives suspension/link forces;
- tire normal force acts on unsprung state;
- road/tire normal force must not also be added directly to chassis.

This is essential to avoid double-counting normal force.

## 7. Prototype configuration
Per corner:
- effective unsprung mass;
- min/max travel;
- travel-limit restitution;
- maximum allowed generalized travel speed.

Engineering seed:
```
UnsprungMass ≈ 35–50 kg
```
depending on vehicle/corner.

## 8. Tests
- balanced tire/suspension force gives near-zero acceleration;
- excess tire force accelerates toward bump;
- excess suspension force accelerates toward droop;
- gravity projection acts correctly;
- chassis upward acceleration creates relative droop tendency;
- travel limit clamps position;
- limit impact cannot numerically explode;
- fixed input/state is deterministic.

## 9. Deferred
- full 3D unsprung rigid body;
- hub spin gyroscopic coupling;
- control-arm/link inertias;
- bushing compliance;
- wheel-bearing compliance;
- curb contact impulse;
- rim strike/damage energy.

## 10. Acceptance
The primitive passes when it can stably integrate the vertical/generalized wheel coordinate and preserve the project’s force conventions, ready to replace the massless-equilibrium contact path in a later controlled integration.
