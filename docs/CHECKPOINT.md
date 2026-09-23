# Active Development Checkpoint

Updated: 2026-09-23

## Current phase
**Proof-of-Physics: dynamic-unsprung experiment after crash-to-functional-damage closure.**

The canonical TA-P01 source path now includes:
- authored vehicle + structural content compilation;
- complete four-wheel suspension/contact/tire/chassis path;
- collision → structure coupling;
- persistent structural geometry deformation;
- direct structure → suspension pickup displacement;
- typed radiator, steering-rack and wheel-hub functional damage consumers;
- functional damage telemetry;
- scenario regression/report infrastructure.

The source remains **build-unverified** until Unreal Engine 5.8 UHT/UBT/C++ compilation and Automation execution actually run.

## Canonical repository
`latektigroo0904/racegame`

## Runtime modules
`TA_Core`, `TA_Surface`, `TA_Tire`, `TA_Powertrain`, `TA_Structure`, `TA_Damage`, `TA_Vehicle`, `TA_Telemetry`.

## Canonical driving path
```
UTAVehicleDefinition
→ validation / FTAVehicleCompiledConfig
→ driver controls
→ drivetrain + wheel angular dynamics
→ front shared-rack double wishbone + rear five-link
→ compliant suspension/anti-roll/tire radial equilibrium
→ tire forces at four physical contact patches
→ 6-DOF chassis integration
```

## Canonical crash paths

### Rigid + internal deformation
```
collision impulse
→ chassis Δv + Δω exactly once
→ rigid translation/rotation removed from structural excitation
→ internal structure deformation
→ plasticity / fracture
```

### Continuous geometric damage
```
persistent structure-node displacement
→ weighted suspension bindings
→ pickup displacement
→ changed camber/toe/load
→ changed tire input
→ changed tire force
```

### Typed functional damage
```
structure/impact damage signal
→ target component route
→ radiator / steering rack / wheel hub
→ persistent subsystem degradation
→ runtime behavior change
```

Important invariant:
**typed damage must not create a second generic alignment-damage path.**

## Implemented baseline

### Four-wheel vehicle
- fixed FL/FR/RL/RR order;
- front mirrored double wishbone with one physical shared rack;
- Ackermann and bump steer;
- rear true five-link;
- front/rear anti-roll coupling inside compliant contact equilibrium;
- pressure-dependent tire radial compliance;
- tire thermal/pressure/wear state;
- RWD prototype drivetrain, clutch, open differential and wheel inertia;
- force-at-point 6-DOF chassis dynamics;
- radiator → coolant loss → thermal derate.

### Structure and damage
- spatial impact distribution;
- momentum-neutral internal deformation excitation;
- XPBD-style distance constraints;
- plasticity and fracture;
- deterministic damage signals;
- authored nodes/constraints/mount bindings/routes;
- persistent compiled damage runtime;
- direct structural suspension bindings;
- radiator routing;
- steering-rack functional damage:
  - severity;
  - command authority;
  - free play;
- per-wheel hub functional damage:
  - severity;
  - brake efficiency;
  - drive efficiency;
  - bearing drag.

### Functional damage runtime effects
Steering:
```
driver command
→ rack mapping
→ authority reduction
→ free-play deadband
→ physical rack displacement
→ tie-rod geometry
```

Wheel/hub:
```
drive torque × drive efficiency
brake torque × brake efficiency
bearing drag opposes wheel rotation
```

### Physics config identity
The structure/damage portion of `PhysicsConfigHash` now includes:
- solver config;
- impact distribution;
- nodes;
- constraints;
- damage-bridge target maps;
- mount weights and thresholds;
- route consumer type;
- accepted signal flags;
- functional damage calibration;
- suspension structure bindings.

### Telemetry
Canonical full telemetry buffer:
`FTATelemetryRingBuffer`

Legacy/lightweight buffer renamed:
`FTACompactTelemetryRingBuffer`

Telemetry includes:
- config identity;
- chassis;
- powertrain;
- wheel loads;
- suspension travel;
- camber/toe;
- tire slip/force/temp/pressure/wear/deflection;
- steering rack position;
- steering-rack damage/authority/free play;
- per-wheel hub damage/brake efficiency/drive efficiency/bearing drag.

### Regression reporting
Implemented:
- scalar `TARegressionEnvelope`;
- scenario-level envelope evaluation;
- physics-hash consistency check;
- CSV report;
- JSON Lines report;
- rich profile:
  - observed min;
  - observed max;
  - observed mean;
  - observed steady-state mean;
  - expected min/max/steady ranges.

Six provisional scenarios:
1. `TA.StaticSettle.V1`
2. `TA.Acceleration.V1`
3. `TA.Braking15Mps.V1`
4. `TA.ConstantSteer15Mps.V1`
5. `TA.AsymmetricRoad20mm.V1`
6. `TA.SyntheticFrontRightCrash.V1`

All remain **provisional**, not trusted.

## Source-level regression closure

Coverage now includes:
- vehicle definition → runtime;
- front/rear suspension geometry;
- tire/suspension compliance;
- anti-roll equilibrium;
- crash → structural deformation;
- structure → pickup offsets;
- pickup offsets → alignment change;
- alignment → tire-force change;
- crash → radiator consequence;
- crash → steering-rack functional damage;
- crash → wheel-hub functional damage;
- steering authority/free-play → physical rack change;
- hub drive-efficiency → delivered drive torque;
- hub brake-efficiency → wheel braking;
- hub bearing drag → wheel coast-down;
- telemetry capture/export;
- regression metrics/profiles.

These are **source-level assertions only** until UE Automation executes them.

## Important assumptions
1. TA-P01 remains the only proving-ground vehicle.
2. Current four-wheel topology is intentionally constrained.
3. Collision external impulse belongs to chassis exactly once.
4. Continuous structural displacement owns alignment geometry truth.
5. Typed functional damage is monotonic until a future explicit repair operation.
6. Quasi-static compliant contact remains canonical.
7. Dynamic unsprung mass remains experimental until it demonstrates stable coupling without force double counting.
8. Provisional regression ranges are diagnostics, not validated calibration.

## Highest current risks
1. First UE 5.8 build may expose UHT/include/API/compiler errors.
2. Automation thresholds have not been numerically executed in UE.
3. Coupled axle/five-link convergence is source-reviewed but not runtime-profiled.
4. Real collision manifolds/contact persistence remain unconnected; crash tests use synthetic impulses.
5. Dynamic unsprung coupling is not yet integrated into canonical contact.
6. Suspension discrete component fracture severity remains incomplete.
7. Fluid/electrical damage consumers remain incomplete.
8. Brake thermal dynamics remain incomplete.
9. Aero remains incomplete.
10. Tire/powertrain authoring still contains prototype defaults that should become versioned content.

## Build-verification status
Still unverified:
- UnrealHeaderTool;
- UnrealBuildTool;
- MSVC/Clang compile;
- Editor module load;
- all `TorqueAtlas.*` Automation tests;
- runtime convergence/profiling;
- replay/cross-machine determinism;
- driving feel/calibration.

No build/test-pass claim may be made before those operations run.

## Immediate next work — no user input required

### 1. Experimental dynamic-unsprung corner
Integrate one isolated proving-ground corner using `TAUnsprungVerticalDynamics`.

Required force ownership:
```
road/tire radial normal force
→ unsprung mass

suspension spring/damper/stop reaction
→ equal/opposite on unsprung mass and chassis

DO NOT:
road normal force → unsprung mass
AND road normal force → chassis directly
```

Required comparison:
- same geometry/road/tire/suspension;
- quasi-static canonical contact vs dynamic unsprung corner;
- static equilibrium;
- 20 mm road step;
- damping transient;
- wheel-hop response;
- energy/stability diagnostics.

### 2. Dynamic-unsprung acceptance gate
Do not make it canonical unless:
- static equilibrium agrees with quasi-static baseline within a defined tolerance;
- road-step transient remains bounded;
- no duplicated normal force reaches chassis;
- travel limit handling is stable;
- fixed inputs are deterministic;
- CPU cost is measured.

### 3. Complete authored tire/powertrain coefficients
Move remaining handling-critical defaults into `UTAVehicleDefinition` and hash them.

### 4. First UE 5.8 build gate
When a usable UE environment exists:
1. generate project files;
2. compile Development Editor;
3. repair UHT/UBT/compiler errors;
4. run all `TorqueAtlas.*` Automation tests;
5. record toolchain + commit + physics hash;
6. capture first trusted telemetry traces;
7. replace provisional ranges with reviewed baselines.

## Exact continuation point
Resume with the **isolated dynamic-unsprung corner adapter**.

First inspect:
- existing `TAUnsprungVerticalDynamics` primitive;
- suspension force conventions;
- tire radial force API;
- current compliant contact resolver.

Then implement a one-corner experimental adapter that reports both:
- force applied to unsprung mass;
- reaction force applied to chassis.

Do not change the canonical four-wheel runtime until comparison tests justify it.

## Checkpoint rule
Update this file before ending every substantial work session and before switching to a new major subsystem.
