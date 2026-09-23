# Active Development Checkpoint

Updated: 2026-09-23

## Current phase
**Integrated front-suspension/contact vehicle-physics prototype.**

The high-fidelity front-corner path can now derive suspension geometry, road contact, tire normal load and contact velocity from chassis/road state rather than requiring those values to be manually supplied.

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

### Propulsion
```
driver throttle
→ engine torque curve
→ clutch
→ gearbox
→ final drive
→ open differential
→ wheel inertia
→ tire slip/force
→ wheel reaction torque
```

### Front suspension / road / tire
```
chassis pose
→ double-wishbone constraints
→ wheel center + orientation
→ road-plane travel solve
→ damper motion ratio
→ spring/damper/stop force
→ anti-roll load transfer
→ tire normal load
→ contact-patch velocity
→ tire force
→ force/moment at physical contact point
→ 6-DOF chassis motion
```

### Structural suspension damage
```
structure node current position
- structure node reference position
→ weighted pickup binding
→ wishbone pickup displacement
→ new 3D suspension geometry
→ changed camber/toe/motion ratio/contact
→ changed tire/chassis behavior
```

### Tire state
```
slip + rolling dissipation
→ surface heat
→ carcass heat
→ internal-air temperature
→ pressure
→ grip change
→ wear/tread loss
→ wet/aquaplaning behavior change
```

### Cooling damage
```
radiator impact/crush
→ puncture/leak
→ coolant loss
→ cooling efficiency loss
→ coolant temperature rise
→ engine torque derate
→ cumulative thermal engine damage
```

### Engine restart
```
Running below stall RPM
→ Stalled
→ starter torque
→ Cranking
→ combustion-start RPM
→ Running
```

## Major implemented systems

### TA_Tire
- slip ratio/angle;
- load sensitivity;
- combined slip;
- camber force;
- pneumatic-trail aligning moment;
- aligning moment applied to chassis torque;
- rolling resistance;
- continuous per-wheel aquaplaning;
- temperature-dependent grip;
- pressure-dependent grip;
- surface/carcass thermal model;
- internal-air temperature and pressure;
- energy-based wear;
- tread-depth loss;
- irreversible thermal degradation.

### Front double wishbone
- upper inner A/B;
- lower inner A/B;
- upper/lower ball joints;
- tie-rod inner/outer;
- wheel center;
- wheel orientation reconstruction;
- steering through tie-rod/rack displacement;
- structural pickup offsets;
- damper chassis mount;
- lower-arm damper mount;
- solved damper length;
- damaged-mode motion-ratio estimate;
- convergence residual/tolerance reporting.

### Suspension/contact
- road-plane contact reach;
- bisection suspension-travel solve;
- full-droop airborne state;
- full-bump penetration reporting;
- spring/damper/bump/droop forces;
- static spring compression;
- anti-roll equal/opposite load transfer;
- contact-point velocity from rigid-body motion;
- longitudinal/lateral contact speed derivation;
- self-derived tire normal load;
- road surface propagation to tire solver.

### Structure → suspension
- structure nodes now retain reference positions;
- current-minus-reference displacement helper;
- weighted node bindings per suspension pickup;
- direct conversion to double-wishbone damage offsets.

### Chassis / vehicle
- 6-DOF chassis integration;
- physical force-at-point torque generation;
- drivetrain/tire/wheel integration;
- brake-to-zero wheel integration;
- tire thermal/wear advancement every vehicle step;
- resolved contact can feed `FTAWheelContactInput`;
- integrated resolved-contact → tire → chassis regression source exists.

## Documentation added this session
- `20-DOUBLE-WISHBONE-GEOMETRY-V02.md`
- `21-CONTACT-LOAD-PIPELINE-V01.md`
- `22-TIRE-THERMAL-WEAR-V01.md`

## Tests written this session
Automation source now additionally covers:
- double-wishbone reference geometry;
- bump travel;
- rack steering changing toe;
- pickup displacement changing camber;
- degenerate wishbone rejection;
- reference road contact;
- self-derived normal load;
- chassis velocity → contact velocity;
- full-droop contact loss;
- damaged-contact alignment;
- structural-node → pickup binding;
- weighted structural binding;
- anti-roll transfer;
- resolved suspension contact → tire → chassis acceleration;
- tire surface heating;
- carcass/internal-air warming;
- pressure rise;
- wear/tread loss;
- cold/optimal/hot grip behavior.

All tests remain **written but not executed**.

## Source/API review performed
Current Epic Unreal Engine 5.8 documentation confirms the TVector functions used by this code, including:
- `Length()`;
- `Equals()`;
- `GetSafeNormal()`;
- `SquaredLength()`;
- `GetClampedToMaxSize()`.

## Important corrections made
1. Damaged suspension no longer assumes motion ratio = 1.0.
2. 1D travel cache is invalid while steering or structural pickup displacement is active.
3. Road surface state now survives the contact-resolution path into the tire solver.
4. Tire aligning moment now affects chassis torque rather than telemetry only.
5. Tire temperature/pressure/wear state now affects actual force capacity.
6. High-fidelity front contact no longer needs manually supplied load or patch velocity.

## Still build-unverified
- UnrealHeaderTool;
- UnrealBuildTool;
- MSVC/Clang compilation;
- Editor module loading;
- Automation tests;
- performance profiling;
- cross-machine determinism;
- real driving calibration.

No build/test-pass claim may be made before those operations actually run.

## Highest remaining technical risks
1. Rear multi-link geometry/contact is still absent, so the whole car cannot yet use self-derived four-wheel loads.
2. Driver steering input is not yet mapped to rack displacement in the integrated vehicle control path.
3. Tire vertical compliance/unsprung mass are absent.
4. Structure/contact crash impulses are not yet distributed from real collision manifolds.
5. Whole-vehicle energy stability must be checked after four independent suspension corners are active.
6. Tire and suspension calibrations are engineering seeds rather than measured data.
7. First real UE 5.8 build may expose UHT/UBT/API issues.

## Immediate next work — no user input required

### 1. Complete front axle
Implement:
- mirrored front-left TA-P01 hardpoints;
- steering rack runtime config;
- normalized steering input → physical rack displacement;
- left/right wishbone solve from one rack position;
- axle anti-roll application;
- Ackermann/bump-steer telemetry and regression tests.

### 2. Rear multi-link geometry v0.1
Implement a true link-based rear upright solver rather than disguising the rear as a double wishbone:
- individual chassis link pickups;
- individual upright link pickups;
- rigid upright/wheel center;
- bump/toe/camber behavior;
- structural pickup displacement;
- damper/spring mount geometry.

### 3. Full four-wheel self-support fixture
Replace all manually supplied proving-ground loads/velocities:
- four suspension corners;
- four road contacts;
- gravity;
- static settle;
- accelerate;
- brake;
- steer;
- roll/load transfer.

### 4. Tire vertical compliance / unsprung mass
After four-wheel geometry is stable:
- tire radial stiffness/damping;
- unsprung wheel/upright mass;
- wheel hop;
- curb/rough-road normal dynamics.

### 5. Structure collision coupling
Add:
- collision impulse input;
- spatial distribution to structural nodes;
- energy accounting;
- suspension pickup displacement from the same crash;
- avoid double-counting rigid/structural impact energy.

### 6. Telemetry / regression export
Add:
- config hash/build metadata;
- CSV trace export;
- regression envelopes;
- static-settle and handling report outputs.

### 7. First Unreal build gate
As soon as UE 5.8 build access exists:
1. generate project files;
2. compile Development Editor;
3. fix UHT/UBT/compiler errors;
4. launch Editor;
5. run all `TorqueAtlas.*` Automation tests;
6. record engine/toolchain/commit;
7. profile fixed-step vehicle and damaged-suspension cases.

## Exact continuation point
Resume with **front-left mirror + steering rack runtime**, then create a complete two-wheel front axle contact/anti-roll solve. After that start the true rear multi-link solver.

Do not expand world size, vehicle roster, career, economy or art production until the prototype vehicle supports its own weight on all four wheels and can accelerate, brake and steer from self-derived contact physics.

## Checkpoint rule
Update this file before ending every substantial work session and before switching to a new major subsystem.
