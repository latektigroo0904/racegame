# Integrated Vehicle Runtime v0.1

Status: implementation specification
Updated: 2026-09-23

## 1. Goal
Create the first deterministic fixed-step orchestrator that connects existing subsystems without inventing unfinished suspension/chassis behaviour.

Initial chain:

```
driver controls
→ engine/clutch/gearbox/final drive/open differential
→ driven wheel torque
→ wheel angular dynamics
→ tire solver
→ road reaction torque
→ aggregate tire forces
```

Suspension/chassis integration will later provide wheel-local velocities, loads and orientations.

## 2. Boundary strategy
v0.1 receives wheel contact/kinematic inputs from outside:
- vertical load;
- longitudinal velocity at contact;
- lateral velocity at contact;
- camber;
- surface sample.

It does **not** yet raycast or solve suspension itself.

This permits:
- deterministic subsystem tests;
- no fake suspension;
- clean replacement with real suspension output later.

## 3. Runtime configuration
`FTAVehicleRuntimeConfig` composes native config only:
- reference vehicle mass;
- wheel configs;
- tire configs;
- engine config;
- clutch config;
- gearbox/final drive;
- driveline compliance;
- driven-wheel indices;
- brake config.

No mutable UObject access during `Step()`.

## 4. Wheel configuration/state
Per wheel config:
```
RadiusM
InertiaKgm2
MaxBrakeTorqueNm
IsDriven
```

State:
```
AngularSpeedRadPerSec
TireState
LastTireOutput
```

Rotational equation:
```
I_wheel * domega/dt =
T_drive
+ T_brake
- Fx * R
```

Brake torque must oppose wheel rotation and must not numerically reverse a near-stopped wheel.

## 5. Driver controls
v0.1:
- throttle [0..1];
- brake [0..1];
- clutch engagement [0..1];
- selected gear.

Later:
- per-axle brake pressure;
- ABS;
- traction control;
- handbrake;
- shift state machine.

## 6. Engine torque seed
Until authored 2D torque maps are implemented, integrated tests may use a deterministic callback or simple curve interface.

Do not hide TA-P01 constants inside the orchestrator.

## 7. Drivetrain coupling
For RWD TA-P01:
1. average driven-wheel angular speed;
2. propagate through final drive and selected gear to gearbox-input target;
3. solve clutch slip torque;
4. apply opposite reaction torque to engine;
5. multiply transmitted clutch torque through gear/final drive;
6. open differential allocates equal side torque limited by wheel reaction capacity;
7. drive torque enters each driven wheel.

This quasi-static differential coupling is temporary and explicitly marked as such.

## 8. Tire reaction coupling
Each tire returns longitudinal force `Fx`.

Reaction torque on wheel:
```
T_tire = -Fx * R
```

The tire force itself is accumulated into chassis/world force output.

## 9. Brake integration
Braking should not produce a direction flip from numerical overshoot.

Recommended:
1. integrate non-brake torque;
2. compute predicted wheel speed;
3. apply brake angular impulse toward zero;
4. clamp brake correction to the amount required to reach zero.

## 10. Step ordering
v0.1 fixed step:
1. validate dt/state size;
2. derive driven-wheel kinematic target;
3. calculate clutch torque;
4. calculate engine torque/friction and integrate engine speed;
5. gearbox + final-drive transforms;
6. differential drive torque;
7. solve tire force using current wheel speed/contact input;
8. integrate each wheel with drive + tire reaction + brake;
9. store outputs;
10. update clutch thermal/wear;
11. increment simulation tick.

Once suspension/chassis are integrated, ordering may gain a small local iteration to resolve wheel/tire/driveline coupling.

## 11. Aggregated output
Expose:
- total longitudinal tire force;
- total lateral tire force;
- total aligning moment contribution;
- per-wheel outputs;
- engine speed;
- clutch slip/temperature;
- selected gear;
- simulation tick.

Do not apply these forces to Unreal rigid bodies inside the pure solver.

## 12. Determinism/performance
Inner step rules:
- fixed dt;
- stable wheel index order;
- no asset lookup;
- no dynamic allocation after initialization;
- no Blueprint calls;
- no random state;
- explicit inputs/outputs.

## 13. Required integration tests
INT-001 zero controls remain bounded.
INT-002 throttle in first gear creates positive driven-wheel torque.
INT-003 tire force creates opposite wheel reaction torque.
INT-004 braking reduces wheel speed without sign flip.
INT-005 neutral isolates engine from wheel-speed target.
INT-006 low-mu driven wheel limits open-diff transmitted side torque.
INT-007 standing water alters only affected wheel tire output.
INT-008 fixed identical initial state produces identical result.
INT-009 clutch slip generates heat.
INT-010 no allocations during steady-state stepping after reserve/initialization (profile gate later).

## 14. Deferred
Not in v0.1:
- chassis 6-DOF integration;
- actual suspension kinematics;
- ABS/TCS;
- tire thermal update;
- engine torque-map asset;
- turbo;
- starter/stall;
- shift synchronizers;
- full differential inertias;
- FFB.

These have their own next milestones.

## 15. Acceptance
The integrated runtime is accepted when a deterministic fixture can close:
```
engine → drivetrain → wheel → tire → wheel reaction
```
and pass the integration tests without scripted vehicle-speed behaviour.
