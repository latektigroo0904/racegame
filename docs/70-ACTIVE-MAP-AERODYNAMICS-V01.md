# Active & Map-Based Aerodynamics v0.1

Updated: 2026-10-08
Status: deferred implementation-ready design; base split aero remains canonical candidate

## 1. Entry condition

Do not promote until base split-aero UE traces exist.

## 2. Coefficient map

Replace constant coefficients only through an optional map backend:

Inputs:
- front ride height;
- rear ride height;
- pitch angle;
- yaw/slip angle;
- active device positions.

Outputs:
- Cd;
- Cl_front;
- Cl_rear;
- optional side-force/yaw coefficients later.

Interpolation must be bounded and continuous.

## 3. Ground effect

Ride height may alter:
- total downforce;
- front/rear balance;
- drag.

Map can include stall when too low/high.

Do not use separate speed-grip boost.

## 4. Active devices

Device state:
- position/angle;
- velocity;
- target;
- actuator health;
- rate limit.

Examples:
- rear wing;
- front flap;
- DRS;
- active grille;
- air brake.

Physics uses actual device state, not target command.

## 5. Controller

Inputs may include:
- speed;
- brake request;
- steering/lateral acceleration;
- drive mode.

Outputs device target.

Controller is separate from aero force solver.

## 6. DRS

DRS changes coefficient map/device state.
It does not subtract drag force directly.

Rules/event eligibility handled outside physics.

## 7. Air brake

Braking command may increase drag/downforce through device motion.

Actuator takes finite time.

## 8. Cooling drag

Radiator/grille opening may trade:
- cooling airflow;
- drag.

This couples aero device state to cooling airflow authority explicitly.

## 9. Damage

- stuck device;
- reduced range;
- missing wing/splitter;
- deformed application point/angle.

Damage selects altered coefficient/device state rather than generic aero damage multiplier.

## 10. Crosswind

Future extended map can output:
- side force;
- yaw moment.

Base current vector drag is not enough to represent real crosswind body aerodynamics completely.

## 11. Data provenance

Map sources:
- CFD;
- wind tunnel;
- measured/inferred literature;
- engineering seeds.

P3/P4 map requires explicit source/validation manifest.

## 12. Tests

AERO-MAP-001 interpolation endpoints.
AERO-MAP-002 interpolation continuity.
AERO-MAP-003 ride-height sweep bounded.
AERO-MAP-004 symmetric map yields expected balance.
AERO-ACT-001 actuator rate limit.
AERO-ACT-002 target does not teleport position.
AERO-ACT-003 DRS state reduces mapped drag only through map.
AERO-ACT-004 air-brake position increases expected coefficient state.
AERO-DMG-001 stuck actuator.
AERO-DMG-002 missing aero part selects damaged map.

## 13. Promotion

Base constant split-aero must remain selectable as fallback/reference backend.
