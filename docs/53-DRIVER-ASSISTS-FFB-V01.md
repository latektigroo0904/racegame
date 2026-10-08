# Driver Assists & Steering FFB Architecture v0.1

Updated: 2026-10-08
Status: implementation-ready architecture

## 1. Principle

Driver assists are controllers around physical actuators.

They may modify:
- brake pressure requests;
- engine torque requests;
- clutch commands;
- active differential requests;
- steering assist torque where applicable.

They may not directly edit:
- tire friction;
- wheel angular speed;
- vehicle linear velocity;
- yaw rate;
- chassis orientation.

## 2. Controller stack

Recommended order:

```
Raw Driver Input
→ Input Conditioning
→ Driver Intent
→ Launch/TCS/ESC/ABS Supervisory Requests
→ Physical Actuator Commands
→ Vehicle Physics
```

ABS remains closest to brake pressure actuator.

## 3. ABS

Detailed hydraulic interaction is defined in `48-HYDRAULIC-BRAKE-ABS-V01.md`.

Per wheel:
- Build;
- Hold;
- Release.

Inputs:
- brake request;
- slip ratio;
- wheel deceleration;
- speed observer;
- contact validity.

Output:
- pressure modulation.

## 4. Traction control

Detect excessive driven-wheel positive slip under propulsion.

Primary interventions in order:
1. request engine torque reduction;
2. request active differential change if equipped;
3. optionally request individual brake pressure.

Do not reduce tire mu.

Torque request should be rate-limited and recover progressively.

## 5. Stability control

Inputs:
- steering intent;
- vehicle speed;
- measured yaw rate;
- lateral acceleration;
- sideslip estimate;
- wheel/contact state.

Reference yaw model can start with a bounded bicycle-model target.

Controller outputs:
- individual brake pressure requests;
- engine torque reduction;
- active diff request where equipped.

ESC must not apply direct corrective chassis torque.

## 6. Launch control

May coordinate:
- engine RPM target;
- throttle/torque request;
- clutch engagement request;
- wheel-slip target.

It operates through existing engine/clutch/differential systems.

## 7. Realism profiles

### Arcade
- stronger stability/traction intervention;
- more input filtering;
- optional automatic recovery helpers, but no hidden physics coefficient changes.

### Sport
- moderate assists;
- permissive slip/yaw thresholds.

### Simulation
- real-car-like ABS/TCS/ESC behavior based on equipped vehicle.

### Hardcore
- vehicle equipment only;
- no extra stability intervention;
- failures/damage persistent.

A realism profile chooses controller policy/default settings, not alternate laws of physics.

## 8. Steering FFB physical chain

```
Tire aligning moment
+ contact force through scrub/caster
+ suspension kickback
→ steering knuckle/tie rod
→ rack
→ steering column
→ steering wheel torque
→ device scaling
```

## 9. Rack force

Future exact geometry should convert tie-rod forces into rack force.

Until then, a geometry-aware reduced model may combine:
- tire Mz;
- mechanical trail;
- caster trail;
- scrub radius.

Do not use lateral acceleration alone as FFB torque.

## 10. Steering column state

Suggested:

```
FTAColumnState
{
    WheelAngleRad
    WheelAngularVelocityRadPerSec
    TorsionRad
}
```

Config:
- steering ratio;
- column stiffness;
- column damping;
- friction;
- power-steering assist curve.

## 11. EPS / hydraulic steering assist

Assist torque is an actuator.

EPS:
- speed-dependent assist map;
- motor torque limit;
- thermal state later.

Hydraulic:
- pump pressure/load;
- engine accessory load;
- fluid network later.

Damage may reduce or remove assist while preserving mechanical steering.

## 12. FFB device layer

Physics outputs physical steering-wheel torque.

Presentation/device layer handles:
- max device torque;
- user strength;
- clipping protection;
- smoothing;
- slew-rate safety;
- optional low-amplitude road texture.

Device scaling never feeds back into vehicle physics.

## 13. Damage

Steering damage already includes:
- authority loss;
- free play;
- geometry displacement.

Future FFB reflects those same states:
- off-center wheel;
- reduced self-aligning;
- knockback;
- friction/stiction.

Do not add separate “damage vibration” as the only feedback.

## 14. Telemetry

Assists:
- target/observed slip;
- ABS mode;
- TCS requested torque factor;
- ESC yaw target/error;
- per-wheel brake intervention;
- active diff request.

Steering:
- rack displacement;
- rack force;
- column torque;
- assist torque;
- final FFB torque;
- clipping state.

## 15. Tests

ASSIST-001 disabled controller is transparent.
ASSIST-002 ABS never commands > driver/allowed pressure unless EBD/ESC explicitly owns additional pressure.
ASSIST-003 TCS reduces actuator torque, not tire grip.
ASSIST-004 TCS releases smoothly when slip recovers.
ASSIST-005 ESC command sign counters yaw error.
ASSIST-006 ESC uses wheel brakes/torque requests, no direct chassis torque.
ASSIST-007 fixed state/input deterministic.

FFB-001 zero tire/rack load gives near-zero physical wheel torque.
FFB-002 opposite steering directions produce opposite aligning torque.
FFB-003 caster/trail sign continuity.
FFB-004 damage/free-play alters rack/column response.
FFB-005 device clipping does not alter physics torque.
FFB-006 fixed state/input deterministic.

## 16. Promotion order

1. hydraulic brakes;
2. ABS;
3. differential backend;
4. TCS;
5. transient tires;
6. ESC;
7. steering rack/column FFB;
8. advanced assist calibration.

## 17. Non-goals v0.1

Deferred:
- autonomous driving;
- lane keeping;
- adaptive cruise;
- torque-steer compensation;
- haptic seat/pedal systems;
- platform-specific FFB SDK details.

## 18. Acceptance

Assists are accepted only when disabling them leaves the underlying physical system intact and enabling them changes actuator commands rather than vehicle state directly.
