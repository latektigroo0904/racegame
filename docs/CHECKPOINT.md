# Active Development Checkpoint

Updated: 2026-09-23

## Current phase
**Early integrated Unreal/C++ vehicle physics prototype.**

The repository now contains the first closed native subsystem chain from engine torque to driven-wheel torque and tire reaction, plus the first functional cooling-damage-to-engine-derate chain.

The code remains **build-unverified** until compiled and executed against Unreal Engine 5.8.

## Canonical repository
`latektigroo0904/racegame`

## Completed specifications
- master GDD;
- technical architecture;
- tire/surface/steering numerical design;
- structural solver v0.2;
- GeoForge normalized schema;
- vehicle data/runtime schema;
- powertrain v0.1;
- suspension kinematics v0.1;
- Proof-of-Physics test matrix;
- Unreal/C++ skeleton;
- functional damage graph;
- integrated vehicle runtime v0.1.

## Enabled runtime modules
```
TA_Core
TA_Surface
TA_Tire
TA_Powertrain
TA_Structure
TA_Damage
TA_Vehicle
```

## Implemented code

### TA_Core
- simulation version types;
- validation messages/results;
- SI/Unreal conversion helpers.

### TA_Surface
- surface material/sample state;
- dry/wet friction blending;
- ice blending.

### TA_Tire
- slip ratio/angle;
- load sensitivity;
- smooth saturation;
- combined slip;
- camber contribution;
- aligning moment;
- rolling resistance;
- continuous per-wheel aquaplaning;
- longitudinal reaction-capacity estimate for drivetrain coupling;
- tests.

### TA_Powertrain
- data-driven torque curve;
- throttle/idle torque;
- redline/limiter fade;
- engine friction/inertia;
- finite starter torque;
- Running/Stalled/Cranking transitions;
- clutch torque;
- clutch heat/wear/fade;
- gearbox transforms;
- separate final drive;
- torsional compliance helpers;
- quasi-static open differential;
- engine thermal state;
- temperature torque derate;
- thermal engine damage;
- tests.

### TA_Structure
- structural nodes;
- compliant distance constraints;
- iterative XPBD-style correction;
- plastic rest-length change;
- fracture;
- velocity reconstruction;
- tests.

### TA_Damage
- deterministic typed damage events;
- fixed-capacity event queue;
- radiator crush/puncture;
- leak area;
- coolant loss;
- cooling efficiency degradation;
- tests.

### TA_Vehicle
- fixed-step integrated runtime;
- two-driven-wheel TA-P01 prototype path;
- engine → clutch → gearbox → final drive → open diff;
- wheel inertia;
- brake-to-zero angular integration;
- tire reaction torque;
- total tire-force aggregation;
- radiator/cooling → engine thermal update;
- integrated tests;
- suspension kinematic cache runtime;
- spring/damper/stop forces;
- static spring compression/preload;
- suspension tests.

## Canonical physical chains now represented in code

### Drive chain
```
throttle
→ engine torque
→ clutch
→ gearbox
→ final drive
→ open differential
→ wheel angular acceleration
→ tire slip/force
→ tire reaction torque
```

### Cooling damage chain
```
radiator impact/crush
→ puncture/leak
→ coolant mass loss
→ cooling efficiency loss
→ coolant temperature increase
→ thermal torque derate
→ cumulative engine thermal damage
```

### Engine restart chain
```
running RPM below stall threshold
→ Stalled
→ starter engagement
→ finite starter torque
→ Cranking
→ combustion-start RPM
→ Running
```

## Important corrections made
1. Conventional gearbox reduction:
   `omega_out = omega_in / G`.
2. Reference suspension ride height includes static spring compression.
3. A zero-rpm engine can no longer self-start from the torque map.
4. Brake integration cannot reverse a wheel merely through timestep overshoot.

## Verification status

### Source-level verified
- files/module boundaries are consistent;
- specs, ADRs and implementation have been synchronized;
- official UE 5.8 docs confirm the `TVector::GetClampedToMaxSize` API used by the structural solver;
- tests exist for major implemented primitives.

### Still not executed
- UnrealHeaderTool;
- UnrealBuildTool;
- C++ compiler;
- Editor startup/module loading;
- Automation tests;
- CPU profiling;
- cross-machine deterministic comparison.

No build/test-pass claim is permitted until actually executed.

## Remaining highest risks
1. First UE 5.8 compile/API cleanup.
2. Full chassis 6-DOF integration and stable force application.
3. Runtime damaged-hardpoint suspension geometry.
4. Structural collision impulse distribution from actual contacts.
5. Tire calibration against trustworthy measurement/reference data.
6. Whole-vehicle energy stability under drivetrain/tire iteration.
7. Multiplayer correction of deformed high-speed vehicles.
8. GeoForge cleanup of complex road infrastructure.

## Immediate next work — no user input required

### 1. Chassis dynamics v0.1
Implement native 6-DOF chassis state:
- position;
- orientation quaternion;
- linear velocity;
- angular velocity;
- mass/inertia;
- force/torque accumulation;
- gravity;
- tire-force application at contact points.

### 2. Suspension geometry runtime v0.2
Implement:
- hardpoint structs;
- validation;
- first front double-wishbone geometric solution;
- damaged pickup offsets;
- cache invalidation/switching.

### 3. Vehicle/suspension/chassis coupling
Replace externally supplied vertical load with suspension/contact-derived load in the first proving-ground fixture.

### 4. Telemetry module
Implement fixed-capacity/ring telemetry:
- simulation tick;
- RPM/gear;
- wheel slips/loads;
- tire forces;
- temperatures;
- damage state;
- solver timing fields.

### 5. Engine/powertrain refinement
Add:
- turbo spool state;
- explicit combustion/load energy input to thermal model;
- over-rev damage accumulator;
- starter battery/electrical hook later.

### 6. Structural-damage integration
Connect:
- collision impulse input;
- node-load distribution;
- damage binding;
- radiator target events;
- suspension pickup displacement.

### 7. Build gate
As soon as an Unreal Engine 5.8 build environment is accessible:
1. generate project files;
2. compile Development Editor;
3. fix UHT/UBT/compiler errors;
4. launch editor;
5. run Automation tests;
6. save exact engine/toolchain/commit metadata;
7. profile first fixed-step fixture.

## Exact continuation point
Resume with **native chassis dynamics v0.1**, then connect tire forces at wheel/contact positions. After that, implement the first damaged-hardpoint double-wishbone runtime solve.

Do not expand world size, brands, career, economy or visual content until a chassis can physically move under the integrated solver.

## Checkpoint rule
Update this file before ending every substantial work session and before switching to a new major subsystem.
