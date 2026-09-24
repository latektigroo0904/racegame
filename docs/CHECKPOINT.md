# Active Development Checkpoint

Updated: 2026-09-24

## Current phase
**Proof-of-Physics v2.** Aerodynamic solver, force application, applied telemetry, regression reporting, asset compiler and asset-level regression source are established. The remaining canonical ownership edit is `UTAVehicleDefinition.Aerodynamics` plus its validate/compile/hash call site. UE 5.8 executable verification remains the acceptance gate.

Canonical repository: `latektigroo0904/racegame`. Content/runtime versions remain SchemaVersion 2, PhysicsVersion 2 and DamageModelVersion 2.

The project remains **UE-build-unverified** until UnrealHeaderTool, UnrealBuildTool/C++ compilation, Editor module loading and `Automation RunTest TorqueAtlas.` complete successfully under Unreal Engine 5.8.

## Aerodynamics status
Established:
- deterministic `TAAerodynamics` solver and `TAVehicleAerodynamicsBridge`;
- `FTAVehicleRuntimeConfig::Aerodynamics`, transient environment input and exact step output;
- canonical step wiring before the single chassis integration;
- `FTAAerodynamicsDefinition` validation, vehicle-origin-local -> COM-local compilation and effective-runtime hashing;
- `TAVehicleAerodynamicsAssetCompiler::CompileValidatedAndHash` as the transactional asset adapter;
- compact applied-aero telemetry, scalar extraction and machine-readable `TAAeroRegressionReport` JSON-lines/CSV output;
- solver, bridge, full-step, effective-hash and asset-compiler regressions;
- `TAVehicleDefinitionAerodynamicsTests.cpp` for non-default propagation, invalid authoring, full-hash sensitivity and deterministic repeated compile/hash;
- `patches/41-aero-vehicle-definition-callsite.patch` as the exact canonical closure patch;
- `scripts/verify_aero_asset_closure.py` as the static post-edit gate;
- `scripts/apply_aero_asset_closure.py` as the idempotent fail-closed applicator;
- `tests/tools/test_apply_aero_asset_closure.py` covering insertion, ordering, idempotence, source drift and ambiguous-anchor refusal;
- `.github/workflows/source-sanity.yml` executes the applicator safety suite and now also exercises the applicator + verifier against the real canonical source tree in an ephemeral CI checkout;
- the ephemeral closure gate additionally requires `git diff --check` and exactly the two intended `TAVehicleDefinition` source files to change.

Important invariant: **no arcade speed-dependent tire-grip multiplier**. Aero grip gain must emerge from physical force application, chassis attitude/load transfer and changed tire normal loads.

## Telemetry ownership
ADR 41 remains accepted:
- compact `FTATelemetrySample` owns exact applied-step physics evidence including aero;
- broad `FTAVehicleTelemetrySample` owns diagnostic/state correlation;
- telemetry never re-solves physics;
- structural consolidation remains deferred until executable evidence and schema migration tests exist.

## Work completed this session
1. Re-audited the canonical `UTAVehicleDefinition` header and confirmed authored aero is still absent after `Structure`.
2. Re-audited the canonical `BuildCompiledConfig` tail and confirmed the final hash still ends `HashStructureRuntime -> PhysicsConfigHash`.
3. Audited `TA_Vehicle.Build.cs` and the aero headers: the aero definition/solver live in `TA_Vehicle`; no new external module dependency is introduced by the planned property/include.
4. Strengthened Source sanity so the closure applicator is executed against the actual repository sources, immediately verified, checked for whitespace errors, and constrained to exactly the two intended canonical files.
5. Confirmed GitHub Actions run 139 started for commit `09bc60aa3373619d7154e75cab27aef915068163`; it was still in progress at checkpoint time, so no pass is claimed yet.
6. Preserved main-source immutability: the CI exercise occurs only in the ephemeral Actions checkout and does not pretend the canonical edit has landed.

## Decisions and assumptions
1. Authored aero application point is vehicle-origin-local; runtime is COM-local.
2. COM subtraction occurs exactly once through the established compile path.
3. Aero hash contribution follows effective compiled values, not raw authored coordinates.
4. Full `PhysicsConfigHash` includes physical COM; only the isolated effective-aero contribution is coordinate-invariant.
5. Air density and wind are transient environment state and are not asset-hashed.
6. Negative lift coefficient means downforce under the current convention.
7. Applied telemetry is evidence; it never recomputes aero.
8. Asset regressions use non-default values and non-zero COM to prevent default masking.
9. `TAVehicleAerodynamicsAssetCompiler` is the single asset validate/compile/hash call site.
10. Failed aero compilation is transactional.
11. Automated source editing must fail closed on source drift or ambiguous anchors.
12. Same-module aero includes require no additional `TA_Vehicle.Build.cs` dependency; UE/UHT remains the executable authority.
13. Single-resultant aero remains current scope; map-based balance and active aero remain deferred.

## Risks
General risks remain UHT/UBT/compiler errors, unexecuted numerical Automation assertions, unmeasured coupled-contact convergence, dynamic-unsprung uncertainty, incomplete collision-manifold persistence, deferred topology-changing suspension fracture, incomplete hydraulic/ABS/fluid-boil behavior and provisional real-world calibration.

Aero-specific risks:
- runtime defaults can still mask the missing canonical asset property until closure lands;
- canonical `PhysicsConfigHash` still excludes aero until closure lands;
- the asset-level definition tests intentionally require the missing property, so UE compilation is expected to fail before closure;
- static gates and Python unit tests do not substitute for UHT/UBT/Automation;
- the adapter must not be called twice or followed by a second aero hash;
- double COM subtraction/raw-coordinate hashing would break reproducibility;
- compact/broad telemetry ownership must remain disciplined;
- the new real-source CI preflight validates patch applicability but does not persist its ephemeral edits.

## Deliverables completed this session
- strengthened `.github/workflows/source-sanity.yml` with real-source aero closure preflight;
- module/UHT dependency audit result recorded;
- refreshed active checkpoint.

## Exact continuation point
Resume with the **canonical vehicle-definition source edit**:
1. inspect Source sanity run 139 and require the real-source closure preflight to be green; if red, fix the applicator/verifier/anchor contract before touching canonical C++;
2. execute `scripts/apply_aero_asset_closure.py` in a patch-capable checkout;
3. immediately require PASS from `scripts/verify_aero_asset_closure.py`;
4. inspect and commit only the two intended canonical source edits;
5. run the four `TorqueAtlas.Vehicle.Definition.Aerodynamics.*` regressions plus effective-hash coordinate-invariance;
6. audit UHT reflection for the new Blueprint property;
7. run UE 5.8 UHT/UBT and `Automation RunTest TorqueAtlas.` when an engine runner is available;
8. only after that closure, start the next major Proof-of-Physics subsystem audit.

Do not start map-based aero, active aero or another large physics subsystem before canonical asset ownership is source-level closed.

## Checkpoint rule
Update this file before ending every substantial work session and before switching to a new major subsystem.
