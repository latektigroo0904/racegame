# Active Development Checkpoint

Updated: 2026-09-23

## Current phase
**Proof-of-Physics v2: source-sanity closure and first executable Unreal Engine 5.8 build gate.**

The repository now has a substantially closed source-level physics foundation:
- four-wheel compliant contact;
- front shared-rack double wishbone;
- rear five-link geometry;
- tire force/thermal/wear/radial compliance;
- drivetrain, clutch and thermal runtime;
- 6-DOF chassis;
- collision → internal structure coupling;
- structure → suspension pickup deformation;
- radiator, steering-rack and wheel-hub functional damage;
- telemetry and scenario regression reporting;
- complete vehicle calibration authoring v2;
- isolated dynamic-unsprung experiment;
- reproducible UE 5.8 verification harness.

The project is still **UE-build-unverified** until UnrealHeaderTool, UnrealBuildTool/C++ compilation, Editor module loading and the full `TorqueAtlas.*` Automation suite actually execute successfully.

## Canonical repository
`latektigroo0904/racegame`

## Version baseline
Current content/runtime defaults:
```
SchemaVersion      = 2
PhysicsVersion     = 2
DamageModelVersion = 2
```

Legacy positive versions may compile with warning. Unknown future versions are rejected.

## Canonical production driving path
```
UTAVehicleDefinition
→ validation / FTAVehicleCompiledConfig
→ driver controls
→ engine / clutch / gearbox / final drive / differential
→ wheel angular dynamics
→ front shared-rack double wishbone + rear five-link
→ quasi-static compliant suspension/anti-roll/tire radial equilibrium
→ tire longitudinal/lateral/alignment forces
→ independent tire-contact and suspension-reaction force application
→ 6-DOF chassis integration
```

The quasi-static compliant path remains canonical.

## Vehicle calibration ownership

Vehicle-specific effective physics is now explicit/versioned for:
- tire force, thermal, pressure, wear, hydro and radial coefficients;
- engine torque curve, friction, run-state, starter and idle control;
- clutch coupling, thermal and wear calibration;
- gearbox/final-drive/mechanical efficiency;
- driveline compliance;
- engine thermal/derate;
- radiator/coolant damage and leak calibration;
- suspension spring/damper/stop calibration;
- steering geometry;
- hardpoint geometry;
- structural solver/impact/damage routing content.

Suspension `KinematicSamples` are **derived**, not authored:
```
authored hardpoints
→ exact front/rear geometry solver
→ deterministic 17-point LUT
```

Global chassis gravity remains scenario/world physics. Vehicle-attached internal structure uses zero local gravity because its nodes represent chassis-relative deformation.

## Physics configuration identity

`PhysicsConfigHash` now covers effective current:
- tire;
- wheel;
- engine and torque curve;
- engine thermal;
- cooling/radiator;
- clutch;
- gearbox;
- driveline;
- front/rear suspension force parameters;
- derived kinematic LUTs;
- steering;
- exact suspension geometry;
- structure solver/impact data;
- structure nodes/constraints;
- damage bridge/routes;
- structure-to-suspension bindings.

A handling/crash-relevant authored change must alter physics identity.

## Crash and functional damage

Continuous geometry path:
```
collision impulse
→ internal structure deformation
→ persistent node displacement
→ suspension pickup displacement
→ camber/toe/load change
→ tire-force consequence
```

Typed functional path:
```
impact/structure signal
→ radiator / steering rack / wheel hub
→ persistent subsystem degradation
→ runtime behavior consequence
```

Key invariant: typed damage does not add a second generic alignment-damage scalar.

Implemented functional consequences:
- radiator puncture/leak/coolant loss;
- steering command authority loss;
- steering rack free play;
- hub brake-efficiency loss;
- hub drive-efficiency loss;
- hub bearing drag.

Functional damage is monotonic until a future explicit repair operation exists.

## Dynamic-unsprung experiment

`TAExperimentalUnsprungCorner` remains isolated from the canonical four-wheel runtime.

Implemented source-level gates:
- quasi-static seed;
- 20 mm road-step transient;
- settled comparison with quasi-static raised-road equilibrium;
- explicit unsprung-weight force accounting;
- no tire-normal double counting;
- separate chassis-side suspension force application point;
- generalized force-balance diagnostics;
- deterministic fixed inputs;
- travel/config validation.

Force ownership:
```
road/tire normal force → unsprung generalized mass
suspension/link reaction → chassis
```

Promotion is blocked until real UE execution and CPU/energy/stability evidence exist.

## Telemetry/regression

Canonical full telemetry buffer:
`FTATelemetryRingBuffer`

Legacy compact buffer:
`FTACompactTelemetryRingBuffer`

Telemetry includes:
- physics config hash;
- chassis/powertrain;
- wheel loads and suspension;
- camber/toe;
- tire slip/force/temp/pressure/wear/deflection;
- rack position/Ackermann/bump steer;
- steering damage/authority/free play;
- per-wheel hub damage/brake/drive/bearing drag.

Regression infrastructure includes:
- scalar envelopes;
- min/max/mean/final/absolute-maximum statistics;
- rich min/max/mean/steady-state profiles;
- physics-hash consistency;
- CSV reports;
- JSON Lines reports.

Six provisional scenarios remain:
1. `TA.StaticSettle.V1`
2. `TA.Acceleration.V1`
3. `TA.Braking15Mps.V1`
4. `TA.ConstantSteer15Mps.V1`
5. `TA.AsymmetricRoad20mm.V1`
6. `TA.SyntheticFrontRightCrash.V1`

They are not trusted baselines until real UE-generated traces are reviewed.

## Verification harness

Source-only CI:
`.github/workflows/source-sanity.yml`

It validates:
- Bash verification-runner syntax;
- PowerShell verification-runner syntax;
- Automation-report validator selftests;
- merge conflict markers;
- balanced C++ delimiters after literal/comment stripping;
- generated-header include ordering;
- duplicate Automation symbols/names;
- project/plugin module descriptors.

Local full UE verification:

Windows:
```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\Verify-Unreal.ps1
```

Linux/macOS:
```bash
bash Scripts/verify-unreal.sh --ue-root /path/to/UE_5.8
```

The runners require UE 5.8, build `TorqueAtlasEditor` Development, execute:
`Automation RunTest TorqueAtlas.`

and then validate `AutomationReport/index.json`.

Artifacts are stored under:
`Saved/Verification/<timestamp>/`

A zero process exit alone is not enough; the report must contain successful Torque Atlas tests and no failed/not-run/in-process results.

## External source-sanity feedback

GitHub Actions is now being used as an actual source feedback loop.

The first enhanced run successfully passed:
- Bash syntax;
- PowerShell syntax;
- Automation-report validator selftests.

It then identified a malformed C++ JSON quote-escape in `TATelemetryRegression.cpp`; that source error was repaired on `main`.

The next source-sanity run determines whether further static issues remain.

This is still not equivalent to an Unreal build.

## Highest current risks
1. UHT/UBT/C++ compiler errors can still exist beyond static delimiter checks.
2. Automation numerical assertions have not run in UE.
3. Dynamic-unsprung stability/energy/CPU cost is unmeasured.
4. Coupled contact and five-link convergence has not been profiled.
5. Real collision manifold/contact persistence is not yet connected.
6. Suspension discrete fracture consumers remain incomplete.
7. Fluid/electrical damage consumers remain incomplete.
8. Brake thermal dynamics remain incomplete.
9. Aero remains incomplete.
10. Real-world calibration remains provisional.

## Immediate next work — no user input required

### 1. Close source-sanity to green
Use GitHub Actions failures as evidence. Repair only real source/harness issues until the latest `main` run is green.

### 2. First UE 5.8 build gate
On any suitable machine with UE 5.8:
1. run the verification harness;
2. preserve the generated `Saved/Verification/<timestamp>` directory;
3. repair UHT/UBT/compiler errors;
4. rerun on a new Git SHA;
5. repair Automation failures;
6. record the first full passing commit/toolchain.

### 3. Establish trusted baselines
Only after full UE pass:
- capture real scenario traces;
- inspect numerical stability and expected steady states;
- replace provisional broad envelopes with reviewed ranges;
- stamp the exact non-zero physics-config hash.

### 4. Dynamic-unsprung measured comparison
After canonical UE baseline exists:
- compare static equilibrium;
- compare 20 mm road-step response;
- inspect mechanical-energy behavior;
- measure per-corner CPU cost;
- test four-corner coupling only if isolated acceptance gates pass.

### 5. Next damage systems
Without weakening the build gate:
- discrete suspension/link fracture consumers;
- brake thermal/fade/wear;
- fluid/electrical consumers.

## Exact continuation point
Resume from the **latest GitHub source-sanity run**.

If green:
- do not claim UE build success;
- proceed to first UE 5.8 executable verification when a toolchain is available;
- otherwise continue source-level work on discrete suspension-component damage behind the same regression/hash/version discipline.

If red:
- inspect the exact failing job step/log;
- repair the smallest root cause;
- rerun through the normal push-triggered workflow.

## Checkpoint rule
Update this file before ending every substantial work session and before switching to a new major subsystem.
