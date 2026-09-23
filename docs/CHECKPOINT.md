# Active Development Checkpoint

Updated: 2026-09-23

## Current phase
**Early Unreal/C++ physics prototype foundation.**

The project has moved beyond pure specification. Core simulation modules now contain first executable algorithms and Automation tests, but the repository remains **build-unverified** until compiled against an installed Unreal Engine 5.8 toolchain.

## Canonical repository
`latektigroo0904/racegame`

## Completed design/specification
- master game vision and GDD;
- technical architecture;
- hybrid real-geography world strategy;
- GeoForge normalized data schema;
- numerical tire solver v0.1;
- wetness/aquaplaning model;
- steering/FFB direction;
- structural solver v0.2 design;
- vehicle data/runtime schema;
- powertrain solver specification;
- suspension kinematics specification;
- Proof-of-Physics acceptance matrix;
- Unreal/C++ repository skeleton;
- functional damage graph v0.1;
- roadmap/MVP;
- 60-brand concept pool (100 maximum).

## Code present

### Root Unreal project
- `TorqueAtlas.uproject`;
- game/editor targets;
- `TorqueAtlas` game module;
- Unreal `.gitignore`.

### TorqueVehicleSimulation plugin
Enabled runtime modules:
- `TA_Core`;
- `TA_Surface`;
- `TA_Tire`;
- `TA_Powertrain`;
- `TA_Structure`;
- `TA_Damage`;
- `TA_Vehicle`.

### TA_Core
Implemented:
- simulation version types;
- validation types;
- SI/Unreal conversion helpers.

### TA_Vehicle
Implemented:
- `UTAVehicleDefinition : UPrimaryDataAsset`;
- native `FTAVehicleCompiledConfig`;
- data validation;
- baseline stable physics-config hash.

### TA_Surface
Implemented:
- typed surface sample;
- dry/wet baseline friction blending;
- ice blending.

### TA_Tire
Implemented first native solver:
- slip ratio;
- slip angle;
- load sensitivity;
- smooth pure-slip force saturation;
- combined-slip limiting;
- camber force contribution;
- pneumatic-trail aligning moment;
- rolling resistance;
- continuous standing-water hydro support loss;
- tread/pressure/load factors in aquaplaning seed model;
- automation tests.

### TA_Powertrain
Implemented:
- engine rotational integration;
- engine friction model;
- clutch torque capacity;
- clutch thermal/wear integration;
- conventional gearbox speed/torque transforms;
- separate final-drive transforms;
- driveline torsional-compliance helpers;
- quasi-static open-differential torque limiter;
- automation tests.

### TA_Structure
Implemented first XPBD-style baseline:
- node state;
- compliant distance constraint;
- semi-implicit prediction;
- iterative position correction;
- plastic rest-length update;
- fracture state;
- velocity reconstruction;
- automation tests for constraint correction, plasticity and fracture.

### TA_Damage
Implemented:
- typed damage signals;
- deterministic fixed-capacity event queue;
- first component-specific model: radiator;
- impact/crush → puncture/leak;
- coolant mass loss;
- airflow/cooling-efficiency degradation;
- automation tests.

## Corrections made
An earlier powertrain draft used the wrong angular-speed direction for a conventional reduction ratio.

Canonical convention is now:
```
G = omega_input / omega_output
omega_output = omega_input / G
T_output ≈ T_input * G * efficiency
```

Gearbox and final drive are solved as separate transforms.

## Verification status
### Verified
- repository files and module structure exist;
- architecture/documentation are synchronized;
- code has received static review during authoring;
- automation test source exists.

### Not yet verified
- UnrealHeaderTool;
- UnrealBuildTool;
- MSVC/Clang compile;
- editor startup;
- module load;
- runtime test execution;
- actual CPU timings;
- deterministic behaviour across machines.

No build-success claim may be made until those checks run.

## Provisional assumptions still requiring calibration
- tire stiffness/peak parameters;
- wet friction coefficients;
- hydro calibration;
- clutch thermal constants;
- driveline compliance;
- radiator leak calibration;
- structure compliance/yield/fracture values;
- structural iteration counts;
- suspension hardpoints;
- timestep/performance budgets.

## Highest technical risks
1. Structural solver stability and CPU cost during severe impacts.
2. Correct coupling of structural displacement into suspension/steering/drivetrain.
3. Tire calibration without proprietary measured datasets.
4. Whole-vehicle solver ordering and energy stability.
5. High-speed multiplayer reconciliation after deformation.
6. GeoForge cleanup cost at complex infrastructure.
7. First real UE 5.8 compile may expose UHT/UBT/API adjustments.

## Immediate next work — no user input required

### 1. Integrated vehicle runtime
Create the first `TA_Vehicle` simulation orchestrator:
- compiled config ownership;
- live sim state;
- fixed-step entry point;
- deterministic subsystem order;
- no allocations in inner step.

### 2. Suspension runtime v0.1
Add:
- hardpoint runtime structs;
- per-corner state;
- static setup/ride-height path;
- first double-wishbone geometric solve;
- damaged pickup offsets.

### 3. Wheel rotational dynamics and brakes
Add:
- wheel inertia;
- drive torque;
- brake torque;
- tire reaction torque;
- angular-speed integration.

This is required to couple powertrain and tire solvers physically.

### 4. Powertrain completion
Add:
- torque-map interface;
- throttle/load response;
- idle/stall state;
- starter;
- turbo state;
- over-rev damage hook.

### 5. Integrated damage chain
Wire:
```
structure event
→ damage binding
→ radiator state
→ coolant/thermal system
→ engine torque derate/failure
```

### 6. Telemetry module
Add deterministic trace samples for:
- wheel forces;
- slip;
- powertrain;
- structural events;
- damage state;
- solver timings.

### 7. Integration tests
Build a minimal deterministic vehicle fixture that exercises:
- engine → clutch → gearbox → diff → wheel;
- wheel → tire reaction;
- surface wetness;
- first structural/damage event.

## Exact continuation point
Resume with **TA_Vehicle integrated runtime + wheel rotational dynamics**, then implement the first suspension runtime geometry.

Do not expand world size, brand roster, career or business systems until the integrated vehicle loop exists.

## Build gate
As soon as access to an Unreal Engine 5.8 build environment is available:
1. generate project files;
2. compile Development Editor;
3. fix UHT/UBT/compiler errors;
4. launch editor;
5. run Automation tests;
6. record toolchain + commit;
7. only then mark the skeleton build-verified.

## Checkpoint rule
Update this file before ending every substantial work session and before switching to a new major subsystem.
