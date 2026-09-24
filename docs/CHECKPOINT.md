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
- `.github/workflows/source-sanity.yml` executes the applicator safety suite and exercises applicator + verifier against the real canonical source tree in an ephemeral CI checkout;
- the ephemeral closure gate additionally requires `git diff --check` and exactly the two intended `TAVehicleDefinition` source files to change;
- Source sanity run 140 for checkpoint commit `47c92b6f1af1d10bc0bd29c471a393ce20a09669` completed successfully on 2026-09-24, proving the real-source preflight/applicator/verifier contract is green in CI.

Important invariant: **no arcade speed-dependent tire-grip multiplier**. Aero grip gain must emerge from physical force application, chassis attitude/load transfer and changed tire normal loads.

## Telemetry ownership
ADR 41 remains accepted:
- compact `FTATelemetrySample` owns exact applied-step physics evidence including aero;
- broad `FTAVehicleTelemetrySample` owns diagnostic/state correlation;
- telemetry never re-solves physics;
- structural consolidation remains deferred until executable evidence and schema migration tests exist.

## P1 static mass-balance preparation
`docs/42-STATIC-MASS-BALANCE-V01.md` now defines the independent analytical equilibrium target for Proof-of-Physics P1.1:
- +X forward, +Y right, +Z up;
- total weight and front/rear axle equilibrium;
- per-axle lateral split using each axle's own track width;
- fail-closed validation that prevents negative static corner targets;
- Class-A invariants for conservation, symmetry, monotonic longitudinal transfer and lateral sign;
- strict separation between analytical reference and dynamic suspension/contact state;
- telemetry contract for later settled-world comparison.

This is deliberately preparation only. It must not distract from landing and executable-verifying canonical aero ownership.

## Work completed this session
1. Checked the latest Source sanity result. Run 140 completed successfully for the previous checkpoint commit; the real-source aero closure preflight is therefore green.
2. Reconfirmed canonical main still has no authored `Aerodynamics` property after `Structure` and still ends the hash path at `HashStructureRuntime -> PhysicsConfigHash`.
3. Confirmed patch 41 still matches the exact intended header/include/property, validation and transactional compile/hash call-site contract.
4. Attempted to obtain a patch-capable local checkout; the local execution environment has no outbound DNS/network access, so no false claim of canonical source mutation was made.
5. Started the next independent Proof-of-Physics preparation slice by specifying P1.1 static mass balance in `docs/42-STATIC-MASS-BALANCE-V01.md`.

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
14. P1.1 analytical mass balance is an oracle only; it must never inject forces or corner loads into the dynamic solver to make the regression pass.
15. Static lateral reference split uses each axle's own track width and rejects COM positions that imply negative corner support.

## Risks
General risks remain UHT/UBT/compiler errors, unexecuted numerical Automation assertions, unmeasured coupled-contact convergence, dynamic-unsprung uncertainty, incomplete collision-manifold persistence, deferred topology-changing suspension fracture, incomplete hydraulic/ABS/fluid-boil behavior and provisional real-world calibration.

Aero-specific risks:
- runtime defaults can still mask the missing canonical asset property until closure lands;
- canonical `PhysicsConfigHash` still excludes aero until closure lands;
- the asset-level definition tests intentionally require the missing property, so UE compilation is expected to fail before closure;
- static gates and Python unit tests do not substitute for UHT/UBT/Automation;
- the adapter must not be called twice or followed by a second aero hash;
- double COM subtraction/raw-coordinate hashing would break reproducibility;
- compact/broad telemetry ownership must remain disciplined.

P1.1 risks:
- authored suspension preload/static compression may not agree with COM-derived equilibrium;
- tire vertical compliance can alter settle dynamics but not final force equilibrium;
- transient roll stiffness distribution must not be conflated with the static analytical oracle.

## Deliverables completed this session
- confirmed Source sanity run 140: success;
- `docs/42-STATIC-MASS-BALANCE-V01.md` analytical P1.1 contract;
- refreshed active checkpoint with green preflight evidence and next implementation slice.

## Exact continuation point
Resume with the **canonical vehicle-definition source edit**:
1. apply `scripts/apply_aero_asset_closure.py` in a patch-capable checkout or equivalent exact two-file edit path;
2. immediately require PASS from `scripts/verify_aero_asset_closure.py`;
3. inspect and commit only `TAVehicleDefinition.h` and `TAVehicleDefinition.cpp`;
4. run the four `TorqueAtlas.Vehicle.Definition.Aerodynamics.*` regressions plus effective-hash coordinate-invariance;
5. audit UHT reflection for the new Blueprint property;
6. run UE 5.8 UHT/UBT and `Automation RunTest TorqueAtlas.` when an engine runner is available.

If executable Unreal verification remains externally unavailable after source closure, implement the pure `TAStaticMassBalance` helper and its Class-A Automation tests from doc 42; do not begin map-based/active aero or broad content production.

## Checkpoint rule
Update this file before ending every substantial work session and before switching to a new major subsystem.
