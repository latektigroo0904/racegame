# Chassis Dynamics v0.1

Status: implementation specification
Updated: 2026-09-23

## 1. Goal
Provide a pure native 6-DOF rigid chassis integrator that can receive tire/suspension/aero forces without depending on Unreal rigid-body state.

This is the rigid reference frame to which high-fidelity suspension, drivetrain and later structural deformation are coupled.

## 2. Chassis state
```
PositionWorldM
OrientationWorldQuat
LinearVelocityWorldMps
AngularVelocityWorldRadPerSec
```

The state position represents the chassis center of mass.

## 3. Chassis configuration
```
MassKg
PrincipalInertiaBodyKgm2
GravityWorldMps2
```

Principal inertia is authored/compiled in chassis/body axes.

Production vehicles must not rely on a generic box inertia approximation.

## 4. Force accumulator
Per fixed step:
```
TotalForceWorldN
TotalTorqueWorldNm
```

For force at world point:
```
r = PointWorld - CenterOfMassWorld
Torque += r × Force
```

All subsystem forces must be explicit.

## 5. Linear integration
Semi-implicit:
```
a = F / m + gravity
v_next = v + a*dt
x_next = x + v_next*dt
```

## 6. Angular dynamics
Convert world angular velocity and torque into body axes.

Euler rigid-body equation:
```
I * domega/dt + omega × (I*omega) = torque
```

Therefore:
```
alpha =
I^-1 * (torque - omega × (I*omega))
```

Integrate body angular velocity semi-implicitly.

## 7. Orientation integration
Convert integrated angular velocity to a world-space axis/angle over the fixed step and apply a delta quaternion.

Normalize every step.

This first implementation is an engineering baseline, not a final high-order rigid-body integrator.

## 8. Tire-force application
Later per wheel:
1. suspension solves wheel/contact position;
2. tire force exists in wheel/contact coordinates;
3. transform force to world;
4. apply at contact point;
5. force contributes to chassis translation;
6. lever arm contributes to pitch/roll/yaw.

This replaces the current scalar aggregate-only vehicle output.

## 9. Suspension-force application
Spring/damper/anti-roll forces are applied at chassis hardpoints along their actual axes.

Equal/opposite unsprung/wheel force handling is added with the suspension/wheel mass model.

## 10. Structural coupling
The rigid chassis is the baseline motion frame.

High-fidelity structural solver will provide:
- deformed mount positions;
- deformation cage/state;
- crash impulses/internal deformation.

The project must avoid double-counting crash energy between rigid contact response and structural deformation.

## 11. Numerical safeguards
- mass > 0;
- all principal inertia components > 0;
- finite vector/quaternion assertions in dev builds;
- quaternion normalization;
- fixed timestep;
- force accumulator reset exactly once per step;
- no hidden frame-rate scaling.

## 12. Tests
CHASSIS-001 centered force produces translation without torque.
CHASSIS-002 off-center force produces angular response.
CHASSIS-003 pure torque rotates without linear acceleration.
CHASSIS-004 gravity free-fall sanity.
CHASSIS-005 orientation remains normalized.
CHASSIS-006 zero force preserves constant linear velocity.
CHASSIS-007 symmetric left/right forces produce expected cancellation of yaw.
CHASSIS-008 fixed initial state/input gives repeatable output.

## 13. Deferred
- collision detection/manifolds;
- chassis contact solver;
- aerodynamic force positions;
- unsprung masses;
- structural energy transfer;
- constraint coupling to world.

## 14. Acceptance
v0.1 is accepted when tire/suspension forces can be applied at physical points and the chassis responds with stable, repeatable 6-DOF motion at the fixed prototype timestep.
