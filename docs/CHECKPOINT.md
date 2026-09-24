# Active Development Checkpoint

Updated: 2026-09-24

## Current phase
**Proof-of-Physics v2.** Aerodynamics is functionally established but still lacks the final canonical `UTAVehicleDefinition.Aerodynamics` ownership edit. Source sanity run 140 proved the exact applicator/verifier contract against the real source tree. UE 5.8 executable verification remains the acceptance gate.

Canonical repository: `latektigroo0904/racegame`. Content/runtime versions remain SchemaVersion 2, PhysicsVersion 2 and DamageModelVersion 2.

The project remains **UE-build-unverified** until UnrealHeaderTool, UnrealBuildTool/C++ compilation, Editor module loading and `Automation RunTest TorqueAtlas.` complete successfully under Unreal Engine 5.8.

## Aerodynamics status
Established: deterministic solver/bridge, runtime config, canonical step wiring, authored definition validation and COM-local compilation, transactional `TAVehicleAerodynamicsAssetCompiler`, compact applied-aero telemetry, regression reporting, solver/bridge/full-step/effective-hash/asset-compiler regressions, asset-level vehicle-definition regression source, exact patch 41, fail-closed applicator/verifier tooling, applicator unit tests and the real-source CI preflight.

Source sanity run 140 completed successfully for checkpoint commit `47c92b6f1af1d10bc0bd29c471a393ce20a09669`.

Canonical main still lacks the authored `Aerodynamics` property and still finalizes the physics hash after structure. The GitHub connector available to this automation can replace whole files but cannot safely line-patch the large canonical definition files; partial retrieval is truncated. Therefore the source edit is intentionally not faked or applied via unsafe whole-file reconstruction.

Important invariant: **no arcade speed-dependent tire-grip multiplier**. Aero grip gain must emerge from physical force application, chassis attitude/load transfer and changed tire normal loads.

## Telemetry ownership
ADR 41 remains accepted: compact `FTATelemetrySample` owns exact applied-step physics evidence; broad `FTAVehicleTelemetrySample` owns diagnostic/state correlation; telemetry never re-solves physics; structural consolidation remains deferred until executable evidence and schema migration tests exist.

## P1.1 static mass balance
The analytical contract is `docs/42-STATIC-MASS-BALANCE-V01.md`.

Implemented:
- `Public/TAStaticMassBalance.h`: pure input/output contract and exported calculation API;
- `Private/TAStaticMassBalance.cpp`: fail-closed analytical four-support equilibrium implementation;
- `Private/Tests/TAStaticMassBalanceTests.cpp`: Class-A Automation coverage for total/axle conservation, centered symmetry, 55/45 baseline, longitudinal monotonicity, +Y lateral sign and invalid-input rejection;
- `Public/TAStaticLoadComparison.h`: measured-mean versus analytical-oracle comparison contract;
- `Private/TAStaticLoadComparison.cpp`: signed total/axle/side error metrics plus maximum absolute corner error, with fail-closed evidence validation;
- `Private/Tests/TAStaticLoadComparisonTests.cpp`: exact-match, signed-aggregation and invalid-evidence regressions.

Implementation properties:
- +X forward, +Y right, +Z up;
- longitudinal COM is measured from rear axle;
- each axle uses its own track width for lateral split;
- calculation/comparison outputs are zeroed before validation so failure cannot leak stale evidence;
- lateral support-boundary cases are rejected before negative analytical corner loads can arise;
- comparison rejects negative/non-finite measured support loads and internally inconsistent analytical targets;
- signed comparison errors are `Measured - Expected`;
- relative total/axle/side errors use their corresponding expected aggregate; max corner error is normalized by expected total weight;
- analytical and comparison helpers have no UObject/world dependency and remain regression evidence only.

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
14. P1.1 analytical mass balance and comparison layer are oracles only; neither injects forces or corner loads into the dynamic solver.
15. Static lateral reference split uses each axle's own track width and rejects COM positions on/outside either lateral support boundary.
16. Static-mass-balance and comparison failure leave deterministic zero outputs.
17. P1.1 comparison reports signed bias and magnitude separately; acceptance thresholds belong to the later sampling/envelope layer, not the metric calculator.

## Risks
General risks remain UHT/UBT/compiler errors, unexecuted numerical Automation assertions, unmeasured coupled-contact convergence, dynamic-unsprung uncertainty, incomplete collision-manifold persistence, deferred topology-changing suspension fracture, incomplete hydraulic/ABS/fluid-boil behavior and provisional real-world calibration.

Aero-specific risks remain the missing canonical asset property/hash contribution until patch 41 lands, possible UHT/compiler issues, and accidental double compilation/hash/COM subtraction.

P1.1 risks:
- authored suspension preload/static compression may disagree with COM-derived equilibrium;
- tire vertical compliance can alter settle dynamics but not final force equilibrium;
- transient roll stiffness distribution must not be conflated with the static analytical oracle;
- contact-force sign/orientation at the eventual telemetry adapter must be normalized exactly once before comparison;
- a single instantaneous frame is not valid static evidence; settled-window qualification is still required;
- the new helpers/tests are source-complete but still UE-build-unverified.

## Deliverables completed this session
- `TAStaticLoadComparison.h`;
- `TAStaticLoadComparison.cpp`;
- `TAStaticLoadComparisonTests.cpp`;
- refreshed active checkpoint.

## Exact continuation point
1. If a safe patch-capable path is available, land patch 41 in `TAVehicleDefinition.h/.cpp`, require `verify_aero_asset_closure.py` PASS, then run the four vehicle-definition aero regressions and effective-hash invariant.
2. Run UHT/include/module audit, UE 5.8 UBT and `Automation RunTest TorqueAtlas.` when an engine runner is available.
3. If executable Unreal verification remains unavailable, continue P1.1 with the **settled-world sampling window and acceptance envelope**. Define qualification using low chassis linear/angular velocity and load stability over a fixed window; only qualified samples may feed measured FL/FR/RL/RR means.
4. Add deterministic tests for window warm-up, reset on motion/load instability, mean accumulation, minimum sample count and threshold boundary behavior.
5. Do not begin map-based/active aero or broad content production until the integrated Proof-of-Physics gate is executable and repeatable.

## Checkpoint rule
Update this file before ending every substantial work session and before switching to a new major subsystem.
