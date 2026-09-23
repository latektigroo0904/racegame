# Functional Damage Graph v0.1

Status: implementation-oriented specification
Updated: 2026-09-23

## 1. Purpose
Convert physical events into persistent mechanical consequences without reducing the vehicle to a single hit-point value.

```
Contact / structural deformation / heat / wear
            ↓
Typed damage signals
            ↓
Component-specific damage models
            ↓
Changed physical parameters
            ↓
Vehicle behaviour changes
```

## 2. No global HP
Forbidden as primary simulation:
```
CarHealth = 37%
```

Allowed:
- summary UI derived from real subsystem state;
- economic repair estimate;
- categorical driveability state.

The solver itself uses component-specific variables.

## 3. Typed signal system
Initial signal families:
- ImpactImpulse;
- ImpactEnergy;
- StructuralDisplacement;
- StructuralFracture;
- TorsionalOverload;
- ThermalExposure;
- WearEnergy;
- FluidPressureLoss;
- ElectricalDisconnection.

Each signal carries:
- stable target binding/index;
- source ID;
- simulation tick/substep;
- scalar/vector payload as applicable;
- optional location/normal;
- severity metadata for telemetry only.

## 4. Compiled damage bindings
Authored references such as:
```
front_radiator_support_nodes → Radiator
left_lower_arm_mount_nodes   → Suspension.FL.LowerArmMount
steering_rack_mount_nodes    → SteeringRack
engine_mount_front           → EngineMount.Front
```

are resolved at vehicle compilation into integer indices.

High-frequency runtime damage processing performs no string lookup or asset lookup.

## 5. Evaluation phases
Per relevant physics step:
1. structural/contact systems emit typed signals;
2. signals are sorted/grouped deterministically;
3. target component models integrate exposure;
4. component physical parameters are updated;
5. fluid/thermal/electrical systems advance;
6. derived failures are emitted for the next dependency phase;
7. telemetry receives a compact event record.

Feedback loops are time-stepped rather than recursively evaluated in one call.

Example:
radiator leak → coolant loss → high engine temperature → thermal engine damage.

## 6. Component-state examples

### Radiator
Do not store only "radiator health".

State:
- frontal area remaining;
- flow efficiency;
- coolant-side integrity;
- leak area;
- mount displacement;
- fan state.

### Steering rack
State:
- rack position;
- housing displacement;
- internal friction;
- lash/free play;
- travel limits;
- tie-rod connection state.

### Suspension arm
State:
- geometry/deformation;
- joint integrity;
- bushing compliance;
- fatigue;
- attachment state.

### Brake hydraulic line
State:
- line pressure capability;
- leak conductance;
- fluid remaining/air ingress proxy;
- connected/disconnected.

### Gearbox
State:
- gear tooth condition by gear where needed;
- bearing friction;
- shaft alignment;
- lubrication;
- shift-system condition.

## 7. Structural displacement is often direct
Where structural geometry already determines function, do not convert it to a generic damage scalar.

Example:
a suspension pickup moved 18 mm by chassis deformation.

The suspension solver consumes that new pickup transform directly.

A damage event may still be logged for repair/UI/economy, but handling comes from geometry.

## 8. Thresholds and continuous degradation
Use continuous physical changes where practical.

Example radiator:
- small deformation: airflow decreases;
- puncture: leak area > 0;
- severe crush: airflow/flow collapse.

Discrete states are reserved for topology changes:
- connection broken;
- wheel detached;
- circuit disconnected;
- shaft fractured.

## 9. Driveability summary
A derived layer may classify:
```
Nominal
Driveable
Degraded
Critical
Immobilized
```

This classification never replaces underlying subsystem state.

## 10. Deterministic ordering
Each runtime component receives a stable compiled index.

Damage signals sort by:
1. simulation tick;
2. substep;
3. target index;
4. source index/type;
5. deterministic sequence ID.

This reduces replay/network divergence caused by unordered event processing.

## 11. Serialization
Persistent damage stores:
- topology failures;
- permanent deformation references/state;
- leaks and fluid amounts;
- persistent component wear/damage;
- repair-relevant history.

Transient quantities such as instantaneous impulse are not saved after they have been integrated.

## 12. Repair interface
Every component model must expose repair-relevant state:
- replaceable?
- repairable?
- alignment required?
- fluid refill required?
- structural jig/body repair required?
- cosmetic-only consequence?

Economy/garage systems consume this interface later.

## 13. First functional chain
Canonical implementation target:

```
front-left impact
→ radiator support deformation
→ radiator puncture/leak
→ coolant mass decreases
→ cooling capacity drops
→ engine coolant temperature rises
→ protection derates torque
→ continued operation causes engine thermal damage
→ possible seizure
```

Parallel chain:
```
front-left impact
→ lower-arm/tie-rod geometry moves
→ toe/camber changes
→ steering pull
→ tire scrub/heat/wear
```

## 14. Performance rule
Do not evaluate every possible component dependency every 240 Hz step.

Rates:
- immediate topology/contact consequences: physics rate;
- fluid/thermal: lower fixed rate;
- wear/economy/history: lower again.

Events bridge rate domains.

## 15. First code milestone
Implement:
- stable typed damage signal;
- deterministic event queue;
- runtime target index;
- no-allocation queue after reserve;
- radiator damage model v0.1;
- test that leak rate increases coolant loss and eventually degrades cooling.

## 16. Acceptance
Damage Graph v0.1 succeeds when the canonical radiator chain can be driven entirely by simulated state and tests, with no scripted post-crash penalty.
