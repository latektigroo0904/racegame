# Active Development Checkpoint

Updated: 2026-09-24

## Current phase
**Proof-of-Physics v2: aerodynamic solver, force application, applied telemetry and machine-readable regression reporting are source-level closed. The only remaining canonical aero ownership edit is the authored `UTAVehicleDefinition` property plus its asset compile/hash call site. An exact patch and a static source-closure gate are now versioned. UE 5.8 executable verification remains the external acceptance gate.**

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
- exact vehicle-definition closure patch versioned as `patches/41-aero-vehicle-definition-callsite.patch`;
- `scripts/verify_aero_asset_closure.py`, a static source gate that enforces the property/include/validation/compiler-call presence, exactly-one canonical compiler call, COM/runtime destination and `structure hash -> aero hash -> final PhysicsConfigHash` ordering.

Important invariant: **no arcade speed-dependent tire-grip multiplier**. Aero grip gain must emerge from physical force application, chassis attitude/load transfer and changed tire normal loads.

## Telemetry ownership decision
`docs/41-TELEMETRY-OWNERSHIP-ADR-V01.md` remains accepted for Proof-of-Physics v2:
- `FTATelemetrySample` is the canonical compact applied-step physics evidence stream, including exact applied aero;
- `FTAVehicleTelemetrySample` remains the broad diagnostic/state-correlation stream;
- aero is not duplicated into the broad stream during this phase;
- telemetry never re-solves physics;
- structural consolidation is deferred until UE 5.8 executable evidence exists and consumers/export schemas can be migrated with golden-output tests.

## Work completed this session
1. Re-read the active checkpoint and exact vehicle-definition closure patch.
2. Re-audited the canonical `UTAVehicleDefinition` header and confirmed the authored aero property is still absent.
3. Re-audited `BuildCompiledConfig` from validation through its final hash assignment. Confirmed the patch's `VehicleRuntime` target is valid because `FTAVehicleRuntimeConfig& VehicleRuntime = OutConfig.VehicleRuntime` is established earlier in the function.
4. Confirmed the canonical final ordering is currently `HashStructureRuntime(...) -> OutConfig.PhysicsConfigHash = Hash`, giving the aero adapter one unambiguous insertion point.
5. Re-checked `FTAVehicleCompiledConfig`: COM is owned as `CenterOfMassMeters` and runtime physics is owned as `VehicleRuntime`, matching the patch contract.
6. Added `scripts/verify_aero_asset_closure.py` as a deterministic static acceptance gate. It is intentionally expected to fail until patch 41 is applied; after application it must pass before UE build verification.
7. Refreshed this checkpoint.

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
12. Static source verification is a pre-build guard only; it cannot substitute for UHT, C++ compilation or Automation execution.

## Risks
Highest general risks remain UHT/UBT/compiler errors, unexecuted numerical Automation assertions, unmeasured coupled-contact convergence, unmeasured dynamic-unsprung behavior, incomplete real collision-manifold persistence, deferred topology-changing suspension fracture, incomplete hydraulic/ABS/fluid-boil behavior, and provisional real-world calibration.

Aero/telemetry-specific risks:
- runtime defaults still mask the missing `UTAVehicleDefinition` property/call site until patch 41 is applied;
- canonical `PhysicsConfigHash` still excludes aero until that patch is applied;
- source-level report/tests have not executed under UE 5.8;
- the call site must not hash aero twice: the adapter already advances the supplied hash on success;
- double COM subtraction or raw-coordinate hashing would corrupt reproducibility;
- two telemetry families intentionally remain, so ownership discipline is required to prevent field drift;
- the GitHub connector used in this run exposes whole-file replacement rather than a line-patch primitive. The canonical C++ files are large, so applying a partial/truncated replacement would be destructive; patch 41 remains the safe versioned edit artifact.

## Deliverables completed this session
- `scripts/verify_aero_asset_closure.py`;
- direct source audit proving patch 41's target variable and final hash insertion point are correct;
- refreshed active checkpoint.

## Exact continuation point
Resume with the **canonical vehicle-definition source edit**:
1. apply `patches/41-aero-vehicle-definition-callsite.patch` using a patch-capable workspace/tool;
2. run `scripts/verify_aero_asset_closure.py` and require PASS;
3. add/extend asset-level tests for non-default propagation, invalid validation, hash sensitivity, coordinate invariance and deterministic no-double-hash behavior;
4. inspect includes/module dependencies and ensure UHT can reflect the new Blueprint property;
5. run UE 5.8 UHT/UBT and `Automation RunTest TorqueAtlas.` when an engine environment is available;
6. only after source closure passes, begin the next major Proof-of-Physics subsystem audit.

After aero asset closure is applied, keep the telemetry split defined by ADR 41 until executable verification. Do not start map-based aero, active aero or another large physics subsystem before the canonical asset property/call site and asset tests are source-level closed.

## Checkpoint rule
Update this file before ending every substantial work session and before switching to a new major subsystem.
