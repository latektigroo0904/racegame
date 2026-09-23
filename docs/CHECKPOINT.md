# Active Development Checkpoint

Updated: 2026-09-23

## Current phase
**Integrated native vehicle-motion prototype foundation.**

The repository now contains a connected native physics path from engine torque through tires into a 6-DOF chassis, plus a first functional cooling-damage chain and fixed-capacity telemetry.

The source remains **build-unverified** until Unreal Engine 5.8 UHT/UBT/C++ compilation and Automation execution are actually run.

## Canonical repository
`latektigroo0904/racegame`

## Runtime modules
```
TA_Core
TA_Surface
TA_Tire
TA_Powertrain
TA_Structure
TA_Damage
TA_Vehicle
TA_Telemetry
```

## Implemented physical chains

### Propulsion and tire reaction
```
driver throttle
→ engine torque curve
→ clutch
→ gearbox
→ final drive
→ open differential
→ wheel inertia
→ tire slip
→ tire force
→ wheel reaction torque
```

### Chassis motion
```
per-wheel tire force
+ per-wheel suspension reaction input
→ force at physical contact point
→ chassis total force + moment
→ linear/angular acceleration
→ 6-DOF chassis state
→ quaternion orientation
```

### Cooling damage
```
radiator impact/crush
→ puncture/leak
→ coolant loss
→ cooling efficiency loss
→ coolant temperature rise
→ thermal torque derate
→ cumulative thermal engine damage
```

### Engine restart
```
Running below stall RPM
→ Stalled
→ starter engagement
→ finite starter torque
→ Cranking
→ combustion-start RPM
→ Running
```

## Implemented systems

### TA_Surface
- surface samples;
- dry/wet/ice baseline friction.

### TA_Tire
- slip ratio/angle;
- load sensitivity;
- combined slip;
- camber contribution;
- aligning moment;
- rolling resistance;
- per-wheel continuous aquaplaning;
- reaction-capacity estimate for driveline coupling.

### TA_Powertrain
- RPM/torque curve;
- throttle/idle;
- limiter fade;
- engine friction/inertia;
- stall/cranking/running state;
- starter torque;
- clutch torque/heat/wear/fade;
- gearbox/final-drive transforms;
- torsional compliance helpers;
- quasi-static open differential;
- engine thermal state;
- overheat torque derate and damage.

### TA_Structure
- nodes;
- compliant distance constraints;
- XPBD-style iteration;
- plastic rest-state change;
- fracture;
- velocity reconstruction.

### TA_Damage
- typed deterministic damage signals;
- fixed-capacity damage queue;
- radiator crush/puncture/leak;
- coolant-loss/cooling degradation.

### TA_Vehicle
- fixed-step integrated solver;
- wheel rotational dynamics;
- brake-to-zero clamp;
- powertrain/tire coupling;
- physical wheel contact points/basis;
- tire/suspension force application to chassis;
- first suspension kinematic cache;
- spring/damper/bump/droop force model;
- static spring compression;
- native 6-DOF chassis integration.

### TA_Telemetry
- fixed-capacity ring buffer;
- no allocation required during push after initialization;
- chronological wraparound read;
- engine/clutch/coolant/chassis sample data;
- four-wheel prototype slip/force channels;
- reserved solver profiling fields.

## Test source currently present
Automation source now covers:
- vehicle definition compilation;
- powertrain torque balance;
- gearbox/final drive;
- clutch heating/wear;
- open differential;
- starter/stall transitions;
- tire force/aquaplaning;
- worn vs new tire standing water;
- structural constraint correction;
- plastic deformation/fracture;
- deterministic damage queue;
- radiator leak/cooling degradation;
- suspension cache interpolation;
- spring/damper force sign/preload;
- integrated powertrain → wheel torque;
- neutral decoupling;
- brake non-reversal;
- split-mu open differential;
- radiator → thermal derate;
- tire force → chassis motion;
- asymmetric tire force → yaw;
- chassis centered/off-center/pure torque;
- telemetry ring-buffer wraparound.

These tests are **written but not yet executed**.

## Verified by current source/API review
- official Unreal 5.8 documentation confirms the vector clamp and quaternion APIs used by current chassis/structure code;
- architecture dependencies remain one-directional around telemetry;
- no global car HP drives physical behaviour;
- no mutable Unreal asset read is required inside the current native solver step.

## Still unverified
- UnrealHeaderTool;
- UnrealBuildTool;
- MSVC/Clang compilation;
- module load order in Editor;
- Automation test execution;
- performance targets;
- numerical determinism across machines;
- actual drive feel/calibration.

## Known limitations / technical debt
1. Tire contact vertical load remains an external input; suspension/contact solver must own it next.
2. Suspension damaged-hardpoint mode currently invalidates the cache but does not yet solve the deformed geometry.
3. Chassis collision/contact manifolds are not implemented.
4. Structural solver does not yet receive distributed impulses from real collision geometry.
5. Open differential is quasi-static, not a full carrier/side-gear inertia solve.
6. Tire low-speed model is regularized but not yet a dedicated static-friction contact formulation.
7. Engine thermal heat generation is still a simplified load proxy.
8. Four-wheel telemetry layout is TA-P01-specific.
9. Unreal compile may expose UHT/API/build-rule corrections.

## Highest next risks
1. Suspension/contact-derived vertical load stability.
2. Damaged suspension geometry solve.
3. Energy stability once chassis, suspension, tires and drivetrain iterate together.
4. Structure/contact energy transfer without double-counting.
5. First actual UE 5.8 build.
6. Tire calibration/validation.

## Immediate next work — no user input required

### 1. Suspension geometry v0.2
Implement authored hardpoints and validation for TA-P01 front double wishbone:
- upper-arm inner axis;
- lower-arm inner axis;
- outer ball joints;
- tie rod;
- damper mounts;
- wheel center;
- damaged structural pickup offsets.

### 2. Contact/load solve
Implement first wheel/road vertical contact path:
- road plane/contact point;
- suspension travel solve;
- spring/damper force;
- vertical tire/contact reaction;
- chassis equal/opposite force.

This removes the externally supplied `VerticalLoadN` from the canonical proving-ground path.

### 3. Damaged pickup solve
When a bound structural mount moves:
- invalidate undamaged cache;
- solve current geometry;
- recompute camber/toe/wheel center;
- feed tire solver.

### 4. Chassis coupling refinement
- wheel-point velocity derived from chassis linear + angular velocity;
- local wheel basis derived from chassis/steering/suspension;
- remove externally supplied longitudinal/lateral contact velocity from canonical path.

### 5. Telemetry/test reporting
- capture config hash/build metadata;
- CSV export;
- deterministic regression-envelope comparison.

### 6. First Unreal build gate
As soon as UE 5.8 build access exists:
1. generate project files;
2. compile Development Editor;
3. fix UHT/UBT/compiler errors;
4. launch Editor;
5. run all `TorqueAtlas.*` Automation tests;
6. record exact engine/toolchain/commit;
7. profile first integrated fixture.

## Exact continuation point
Resume with **TA-P01 double-wishbone hardpoint schema + front suspension geometric solver**, then derive wheel/contact velocity and vertical load from chassis/suspension state.

Do not expand the world, brand roster, career, economy or art production until the proving-ground vehicle can support its own weight, accelerate, brake and corner through the integrated solver.

## Checkpoint rule
Update this file before ending every substantial work session and before switching to a new major subsystem.
