# Active Development Checkpoint

Updated: 2026-09-23

## Current phase
**Proof-of-Physics v2: canonical aerodynamic force application and machine-readable aero regression reporting are source-level closed. The remaining primary closure is authored-aero ownership in `UTAVehicleDefinition` plus the canonical asset compile/hash call site. UE 5.8 executable verification remains the external acceptance gate.**

Canonical repository: `latektigroo0904/racegame`. Current content/runtime versions remain SchemaVersion 2, PhysicsVersion 2 and DamageModelVersion 2.

## Established source-level foundation
The repository contains four-wheel compliant contact; front shared-rack double wishbone; rear five-link geometry; tire force/thermal/wear/radial compliance; engine/clutch/gearbox/final-drive/differential dynamics; 6-DOF chassis; structural crash deformation and suspension pickup coupling; typed functional damage; brake thermal/fade/wear; telemetry/regression reporting; vehicle calibration authoring; isolated dynamic-unsprung experimentation; deterministic aerodynamic solver/bridge/authoring/compiler helpers; and UE 5.8 verification runners.

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
- `TAAeroRegressionReport` machine-readable summary over exact applied compact telemetry, with JSON-lines and CSV export.

Important invariant: **no arcade speed-dependent tire-grip multiplier**. Aero grip gain must emerge from physical force application, chassis attitude/load transfer and changed tire normal loads.

## Work completed this session
1. Re-audited the canonical checkpoint and current `UTAVehicleDefinition`/`BuildCompiledConfig` source.
2. Confirmed the primary gap is still concrete: the asset class ends at `Structure`, while final `PhysicsConfigHash` is assigned after structure hashing with no aero adapter call.
3. Confirmed the existing `TAVehicleAerodynamicsAssetCompiler` already owns the correct transaction boundary: validate/compile effective COM-local runtime/hash without mutating the hash on invalid authoring.
4. Added `docs/40-AERO-ASSET-CALLSITE-PATCH-MANIFEST-V01.md`, freezing the exact minimal call-site patch, validation code, ordering, frozen non-default regression fixture and acceptance criteria.
5. Explicitly prohibited duplicate raw-coordinate hashing and duplicate COM subtraction at the future call site.

## Decisions and assumptions
1. Vehicle asset aero authoring uses vehicle-origin-local coordinates; runtime application point is COM-local.
2. COM subtraction happens exactly once inside the established aero compile path.
3. Physics hashing follows effective compiled aero values, never raw authored coordinates.
4. Air density and wind remain transient environment state and are not asset-hashed.
5. Negative lift coefficient means downforce under the current solver convention.
6. Telemetry/regression consume applied step output and never independently recompute aero.
7. Asset integration regressions must use non-default values and non-zero COM to prevent default-value masking.
8. `TAVehicleAerodynamicsAssetCompiler` is the single intended asset call site for validate/compile/hash.
9. Failed asset aero compilation is transactional.
10. Single-resultant aero remains Proof-of-Physics scope; map-based front/rear balance and active aero remain deferred pending executable evidence.

## Risks
Highest general risks remain UHT/UBT/compiler errors, unexecuted numerical Automation assertions, unmeasured coupled-contact convergence, unmeasured dynamic-unsprung behavior, incomplete real collision-manifold persistence, deferred topology-changing suspension fracture, incomplete hydraulic/ABS/fluid-boil behavior, and provisional real-world calibration.

Aero-specific risks:
- runtime defaults still mask the missing `UTAVehicleDefinition` property/call site until canonical integration is completed;
- canonical `PhysicsConfigHash` still excludes aero at the vehicle-definition call site;
- source-level report/tests have not executed under UE 5.8;
- a future call site must not hash aero twice: the adapter already advances the supplied hash on success;
- double COM subtraction or raw-coordinate hashing would corrupt reproducibility;
- there are two telemetry sample families; future consolidation should be deliberate rather than silently duplicating channels.

## Deliverables completed this session
- `docs/40-AERO-ASSET-CALLSITE-PATCH-MANIFEST-V01.md`;
- frozen exact property/validation/compile/hash ordering;
- frozen asset-level acceptance fixture and regression matrix;
- refreshed active checkpoint.

## Exact continuation point
Resume with the **canonical vehicle-definition call-site closure**, using `docs/40-AERO-ASSET-CALLSITE-PATCH-MANIFEST-V01.md` as the patch contract:
1. add `TAAerodynamicsDefinition.h` and `FTAAerodynamicsDefinition Aerodynamics` to `UTAVehicleDefinition`;
2. add `Vehicle.InvalidAerodynamics` validation before the early validation return;
3. include `TAVehicleAerodynamicsAssetCompiler.h` in the implementation;
4. call `CompileValidatedAndHash` exactly once after the base hash is assembled and before assigning `OutConfig.PhysicsConfigHash`;
5. extend asset-level tests for non-default propagation, invalid validation, hash sensitivity, coordinate invariance and deterministic no-double-hash behavior;
6. run source-sanity, then UE 5.8 verification when an engine environment is available.

After that closure, audit whether the two telemetry sample families should be consolidated or retained as explicit compact-vs-broad layers. Do not start a new major physics subsystem until the canonical vehicle-definition aero property/call site and tests are source-level closed. Do not claim UE build success until the verification harness actually runs against UE 5.8.

## Checkpoint rule
Update this file before ending every substantial work session and before switching to a new major subsystem.
