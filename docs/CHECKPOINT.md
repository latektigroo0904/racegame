# Active Development Checkpoint

Updated: 2026-09-23

## Current phase
**Proof-of-Physics v2: canonical aerodynamic force application is wired into the ordinary vehicle step. Aero machine-readable regression reporting is now source-level closed over the exact applied compact telemetry. The remaining primary closure is UTAVehicleDefinition authored-aero ownership and asset call-site integration. UE 5.8 executable verification remains the external acceptance gate.**

Canonical repository: `latektigroo0904/racegame`. Current content/runtime versions remain SchemaVersion 2, PhysicsVersion 2 and DamageModelVersion 2.

## Established source-level foundation
The repository contains four-wheel compliant contact; front shared-rack double wishbone; rear five-link geometry; tire force/thermal/wear/radial compliance; engine/clutch/gearbox/final-drive/differential dynamics; 6-DOF chassis; structural crash deformation and suspension pickup coupling; typed functional damage; brake thermal/fade/wear; telemetry/regression reporting; vehicle calibration authoring; isolated dynamic-unsprung experimentation; aerodynamic solver/bridge/authoring definition; and UE 5.8 verification runners.

The project remains **UE-build-unverified** until UnrealHeaderTool, UnrealBuildTool/C++ compilation, Editor module loading and `Automation RunTest TorqueAtlas.` complete successfully under Unreal Engine 5.8.

## Aerodynamics status
Established:
- deterministic `TAAerodynamics` solver;
- `FTAVehicleRuntimeConfig::Aerodynamics` ownership;
- transient `FTAVehicleStepInput::AerodynamicsEnvironment` ownership;
- exact result slot in `FTAVehicleStepOutput::Aerodynamics`;
- `TAVehicleAerodynamicsBridge` as the single adapter into the shared chassis force accumulator;
- Blueprint-authorable `FTAAerodynamicsDefinition` with validation, vehicle-origin-local -> COM-local compilation and effective-runtime hashing helper;
- canonical `TAVehicleSimulation::Step` aero call immediately before the single chassis integration;
- exact applied-result aero telemetry and scalar regression extraction;
- full-step source regressions for moving-air response, zero-density neutrality and headwind amplification;
- effective aero hash coordinate-invariance regression;
- `TAVehicleAerodynamicsAssetCompiler::CompileValidatedAndHash` transactional asset adapter;
- asset-compiler regressions using frozen non-default values and non-zero COM;
- `TAAeroRegressionReport` machine-readable summary layer over the exact applied compact telemetry, with JSON-lines and CSV export;
- report regressions for scalar sign conventions, steady-state summaries, serialization presence and invalid scenario-frame rejection.

Important invariant: **no arcade speed-dependent tire-grip multiplier**. Aero grip gain must emerge from physical force application, chassis attitude/load transfer and changed tire normal loads.

## Work completed this session
1. Re-audited the repository tree and active checkpoint before editing.
2. Confirmed the canonical asset-side gap remains: `UTAVehicleDefinition` still has no authored `Aerodynamics` property and `BuildCompiledConfig` still finalizes `PhysicsConfigHash` without aero.
3. Audited telemetry ownership and found two intentional-but-divergent telemetry surfaces: `FTATelemetrySample` is the compact exact-step sample and already contains applied aero, while `FTAVehicleTelemetrySample` is the older broad ring-buffer sample used by the generic scenario regression layer and currently contains no aero channels.
4. Avoided copying/recomputing aero into the older broad sample just to satisfy reporting. Added `TAAeroRegressionReport` directly over `FTACompactTelemetryRingBuffer`, preserving the single-source-of-truth applied-aero path.
5. Added summaries for relative air speed, dynamic pressure, force magnitude, drag-axis force, vertical force and pitch torque using `TARegressionEnvelope::Summarize`.
6. Added deterministic JSON-lines and CSV exporters suitable for CI artifact capture.
7. Added Automation regressions proving sample count, min/max/mean/trailing mean, drag/downforce/pitch sign conventions, serialization fields and invalid frame rejection.

## Decisions and assumptions
1. Vehicle asset authoring uses vehicle-origin-local coordinates; runtime aero application point is COM-local.
2. COM subtraction happens exactly once during compilation.
3. Physics hashing follows effective compiled aero values, never raw authored coordinates.
4. Air density and wind remain transient environment state and are not asset-hashed.
5. Negative lift coefficient means downforce under the current solver convention.
6. Telemetry/regression consume applied step output and never independently recompute aero.
7. Asset integration regressions use deliberately non-default values and non-zero COM to prevent default-value masking.
8. `TAVehicleAerodynamicsAssetCompiler` remains the intended single asset call-site for validate/compile/hash.
9. Failed asset aero compilation must be transactional.
10. Machine-readable aero reporting is built from `FTACompactTelemetryRingBuffer`, because that path already owns the exact applied aero result. The older broad telemetry buffer is not expanded merely to duplicate those values.
11. Single-resultant aero remains Proof-of-Physics scope; map-based front/rear balance and active aero remain deferred pending executable evidence.

## Risks
Highest general risks remain UHT/UBT/compiler errors, unexecuted numerical Automation assertions, unmeasured coupled-contact convergence, unmeasured dynamic-unsprung behavior, incomplete real collision-manifold persistence, deferred topology-changing suspension fracture, incomplete hydraulic/ABS/fluid-boil behavior, and provisional real-world calibration.

Aero-specific risks:
- runtime defaults still mask the missing `UTAVehicleDefinition` property/call-site until canonical integration is completed;
- canonical `PhysicsConfigHash` still excludes aero at the vehicle-definition call site;
- source-level report/tests have not executed under UE 5.8;
- a future call site must not hash aero twice: the adapter already advances the supplied hash on success;
- double COM subtraction or raw-coordinate hashing would corrupt reproducibility;
- there are two telemetry sample families; future consolidation should be deliberate rather than silently duplicating channels.

## Deliverables completed this session
- `Public/TAAeroRegressionReport.h`;
- `Private/TAAeroRegressionReport.cpp`;
- `Private/Tests/TAAeroRegressionReportTests.cpp`;
- machine-readable JSON-lines/CSV aero summaries sourced from exact applied telemetry;
- refreshed active checkpoint.

## Exact continuation point
Resume with the **canonical vehicle-definition call-site closure**:
1. add `#include "TAAerodynamicsDefinition.h"` and `FTAAerodynamicsDefinition Aerodynamics` to `UTAVehicleDefinition`;
2. add `Vehicle.InvalidAerodynamics` validation before the early validation return;
3. include `TAVehicleAerodynamicsAssetCompiler.h` in the implementation;
4. after the normal base hash has been assembled and COM is known, call `TAVehicleAerodynamicsAssetCompiler::CompileValidatedAndHash(Aerodynamics, CenterOfMassVehicleLocalM, Hash, VehicleRuntime.Aerodynamics)` exactly once before assigning `OutConfig.PhysicsConfigHash`;
5. extend `TAVehicleDefinitionTests.cpp` to prove non-default propagation, invalid validation code and physics-hash sensitivity;
6. run source-sanity, then UE 5.8 verification when an engine environment is available.

After that closure, audit whether the two telemetry sample families should be consolidated or kept as compact-vs-broad layers with an explicit documented boundary. Do not start a new major physics subsystem until the canonical vehicle-definition aero property/call-site and tests are source-level closed. Do not claim UE build success until the verification harness actually runs against UE 5.8.

## Checkpoint rule
Update this file before ending every substantial work session and before switching to a new major subsystem.
