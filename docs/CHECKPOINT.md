# Active Development Checkpoint

Updated: 2026-09-23

## Current phase
**Proof-of-Physics: authored handling coefficients + first executable UE 5.8 verification gate.**

The source path now includes authored vehicle/structure compilation, four-wheel suspension/contact/tire/chassis simulation, crash-to-structural and typed functional damage, telemetry/regression reporting, and an isolated dynamic-unsprung corner experiment. The source remains **build-unverified** until Unreal Engine 5.8 UHT/UBT/C++ compilation and Automation execution actually run.

## Canonical repository
`latektigroo0904/racegame`

## Runtime modules
`TA_Core`, `TA_Surface`, `TA_Tire`, `TA_Powertrain`, `TA_Structure`, `TA_Damage`, `TA_Vehicle`, `TA_Telemetry`.

## Canonical production driving path
```
UTAVehicleDefinition
→ validation / FTAVehicleCompiledConfig
→ driver controls
→ drivetrain + wheel angular dynamics
→ front shared-rack double wishbone + rear five-link
→ quasi-static compliant suspension/anti-roll/tire radial equilibrium
→ tire forces at four physical contact patches
→ 6-DOF chassis integration
```

## Crash and functional-damage closure
Continuous geometry:
```
collision impulse → structure deformation → persistent pickup displacement
→ camber/toe/load change → tire input → tire-force change
```

Typed functional damage:
```
structure/impact signal → radiator / steering rack / wheel hub
→ persistent subsystem degradation → runtime behavior change
```

Invariant: typed damage must not create a second generic alignment-damage path. Collision external impulse belongs to the chassis exactly once.

## Dynamic-unsprung experiment
`TAExperimentalUnsprungCorner` is now present as an isolated experiment and must **not** replace canonical contact yet.

Implemented source-level coverage includes:
- quasi-static initialization;
- 20 mm road-step transient;
- convergence toward raised-road quasi-static equilibrium;
- gravity/unsprung-weight force accounting;
- explicit no-normal-force-double-count test;
- chassis suspension reaction at the chassis-side damper mount;
- fixed-input determinism;
- config/travel-limit validation.

Force ownership:
```
road/tire normal force → unsprung mass
suspension reaction → chassis
```

`VerticalLoadN` remains tire-road load; `SuspensionForceWorldN` is chassis reaction. They are intentionally not forced equal for the dynamic-unsprung path.

Promotion remains blocked on real UE execution, CPU profiling and energy/stability evidence. See `docs/32-DYNAMIC-UNSPRUNG-EXPERIMENT-V01.md`.

## Telemetry and regression
Canonical telemetry uses `FTATelemetryRingBuffer`; lightweight legacy capture is `FTACompactTelemetryRingBuffer`.

Regression infrastructure includes scalar envelopes, scenario evaluation, physics-hash consistency, CSV/JSONL reports and observed min/max/mean/steady-state profiles.

Provisional scenarios remain:
1. `TA.StaticSettle.V1`
2. `TA.Acceleration.V1`
3. `TA.Braking15Mps.V1`
4. `TA.ConstantSteer15Mps.V1`
5. `TA.AsymmetricRoad20mm.V1`
6. `TA.SyntheticFrontRightCrash.V1`

These ranges are diagnostics, not trusted calibration, until UE execution.

## Verification harness
Repository-side UE 5.8 verification runners now exist for Windows and Unix-like hosts, with Automation JSON report validation and documented `RunTest` invocation. This reduces the first-build gate to supplying a usable UE 5.8 installation/toolchain and executing the harness; it does not itself constitute a successful build.

## Important assumptions
1. TA-P01 remains the only proving-ground vehicle.
2. Current four-wheel topology is intentionally constrained.
3. Quasi-static compliant contact remains canonical.
4. Dynamic unsprung mass remains experimental until measured evidence clears the acceptance gate.
5. Typed functional damage is monotonic until an explicit repair operation exists.
6. Provisional regression ranges are diagnostics only.

## Highest current risks
1. First UE 5.8 build may expose UHT/include/API/compiler errors.
2. Automation thresholds have not been numerically executed in UE.
3. Dynamic-unsprung CPU cost and mechanical-energy behavior are not measured.
4. Coupled axle/five-link convergence is source-reviewed but not runtime-profiled.
5. Real collision manifolds/contact persistence remain unconnected; crash tests use synthetic impulses.
6. Suspension discrete component fracture severity remains incomplete.
7. Fluid/electrical damage consumers remain incomplete.
8. Brake thermal dynamics and aero remain incomplete.
9. Tire/powertrain authoring still contains prototype defaults that must become versioned content.

## Build-verification status
Still unverified: UnrealHeaderTool, UnrealBuildTool, MSVC/Clang compile, Editor module load, all `TorqueAtlas.*` Automation tests, runtime convergence/profiling, replay/cross-machine determinism and driving feel/calibration.

No build/test-pass claim may be made before those operations run.

## Immediate next work — no user input required

### 1. Audit handling-critical defaults
Inspect `UTAVehicleDefinition`, `FTAVehicleCompiledConfig`, tire runtime and powertrain runtime. Classify every coefficient as authored, derived or solver-internal. Handling-critical calibration must not silently live as a C++ default.

### 2. Promote tire/powertrain authoring
Move remaining vehicle-specific tire and powertrain coefficients into versioned authored content, validate ranges, compile them into runtime config and include them in `PhysicsConfigHash`.

### 3. Add authoring/hash regressions
Tests must prove that changing each promoted coefficient changes the compiled config/hash and that invalid values fail validation rather than being silently clamped into a different vehicle.

### 4. First UE 5.8 build gate
When a usable UE environment exists: generate project files; compile Development Editor; repair UHT/UBT/compiler errors; run all `TorqueAtlas.*` tests; record toolchain + commit + physics hash; capture trusted telemetry; then review provisional envelopes.

### 5. Dynamic-unsprung evidence after build gate
Measure canonical-vs-experimental CPU cost and add a mechanical-energy diagnostic before considering four-wheel integration.

## Exact continuation point
Resume with the **handling-critical default audit**. Start at `UTAVehicleDefinition` and `FTAVehicleCompiledConfig`, trace tire and powertrain values into their runtime configs, and produce a coefficient ownership matrix. Promote only values that describe the vehicle/setup; keep numerical solver controls solver-owned.

## Checkpoint rule
Update this file before ending every substantial work session and before switching to a new major subsystem.
