# Active Development Checkpoint

Updated: 2026-09-24

## Current phase
**Proof-of-Physics v2: canonical aerodynamic force application and machine-readable aero regression reporting are source-level closed. The remaining primary source edit is authored-aero ownership in `UTAVehicleDefinition` plus the canonical asset compile/hash call site; an exact ready-to-apply patch is now versioned. Telemetry ownership has been explicitly resolved for the current phase. UE 5.8 executable verification remains the external acceptance gate.**

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
- `TAAeroRegressionReport` machine-readable summary over exact applied compact telemetry, with JSON-lines and CSV export;
- exact vehicle-definition closure patch versioned as `patches/41-aero-vehicle-definition-callsite.patch`.

Important invariant: **no arcade speed-dependent tire-grip multiplier**. Aero grip gain must emerge from physical force application, chassis attitude/load transfer and changed tire normal loads.

## Telemetry ownership decision
`docs/41-TELEMETRY-OWNERSHIP-ADR-V01.md` is accepted for Proof-of-Physics v2:
- `FTATelemetrySample` is the canonical compact applied-step physics evidence stream, including exact applied aero;
- `FTAVehicleTelemetrySample` remains the broad diagnostic/state-correlation stream;
- aero is not duplicated into the broad stream during this phase;
- telemetry never re-solves physics;
- structural consolidation is deferred until UE 5.8 executable evidence exists and consumers/export schemas can be migrated with golden-output tests.

## Work completed this session
1. Re-audited the real repository tree and located the canonical vehicle source under `Plugins/TorqueVehicleSimulation/Source/TA_Vehicle`.
2. Re-read `UTAVehicleDefinition`, `BuildCompiledConfig`, the aero asset compiler and the final hash assignment; confirmed the missing property/call site is still the only canonical aero ownership gap.
3. Versioned `patches/41-aero-vehicle-definition-callsite.patch`, containing the exact header include/property, `Vehicle.InvalidAerodynamics` validation, asset-compiler include and single post-base-hash `CompileValidatedAndHash` call.
4. Audited both telemetry families directly from source.
5. Accepted `docs/41-TELEMETRY-OWNERSHIP-ADR-V01.md`: retain compact applied-physics telemetry and broad diagnostic telemetry as explicit separate layers for Proof-of-Physics v2 rather than duplicating aero or performing a risky pre-build schema merge.
6. Refreshed this checkpoint.

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
11. Compact telemetry owns exact applied-step physics evidence; broad telemetry owns high-dimensional diagnostic/state correlation for this phase.
12. No aero-field duplication into broad telemetry until a deliberate adapter/schema migration is justified by verified consumers.

## Risks
Highest general risks remain UHT/UBT/compiler errors, unexecuted numerical Automation assertions, unmeasured coupled-contact convergence, unmeasured dynamic-unsprung behavior, incomplete real collision-manifold persistence, deferred topology-changing suspension fracture, incomplete hydraulic/ABS/fluid-boil behavior, and provisional real-world calibration.

Aero/telemetry-specific risks:
- runtime defaults still mask the missing `UTAVehicleDefinition` property/call site until the versioned patch is applied to the canonical source files;
- canonical `PhysicsConfigHash` still excludes aero until that patch is applied;
- source-level report/tests have not executed under UE 5.8;
- the call site must not hash aero twice: the adapter already advances the supplied hash on success;
- double COM subtraction or raw-coordinate hashing would corrupt reproducibility;
- two telemetry families intentionally remain, so ownership discipline is required to prevent field drift;
- the current GitHub connector exposes whole-file replacement rather than a line-patch primitive; the canonical vehicle files are large enough that safe in-place replacement was not performed from a truncated retrieval. The exact patch is committed instead of risking destructive replacement.

## Deliverables completed this session
- `patches/41-aero-vehicle-definition-callsite.patch`;
- `docs/41-TELEMETRY-OWNERSHIP-ADR-V01.md`;
- direct source audit of the canonical vehicle-definition/hash path and both telemetry families;
- refreshed active checkpoint.

## Exact continuation point
Resume with the **canonical vehicle-definition source edit**:
1. apply `patches/41-aero-vehicle-definition-callsite.patch` to the canonical source using a safe patch-capable workspace/tool;
2. verify `FTAAerodynamicsDefinition Aerodynamics` is Blueprint-authorable and `Vehicle.InvalidAerodynamics` is emitted before the early validation return;
3. verify `CompileValidatedAndHash` is called exactly once after the existing base/structure hash and before `OutConfig.PhysicsConfigHash = Hash`;
4. add/extend asset-level tests for non-default propagation, invalid validation, hash sensitivity, coordinate invariance and deterministic no-double-hash behavior;
5. run source-sanity and inspect includes/module dependencies;
6. run UE 5.8 UHT/UBT and `Automation RunTest TorqueAtlas.` when an engine environment is available.

After the aero asset closure is actually applied, keep the telemetry split defined by ADR 41 until executable verification. The next major physics subsystem should not start before the canonical vehicle-definition aero property/call site and asset tests are source-level closed.

## Checkpoint rule
Update this file before ending every substantial work session and before switching to a new major subsystem.
