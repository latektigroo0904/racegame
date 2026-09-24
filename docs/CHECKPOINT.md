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
- `TAStaticMassBalance`: pure fail-closed analytical four-support equilibrium oracle;
- `TAStaticLoadComparison`: measured-mean versus oracle signed aggregate/corner error metrics;
- `TAStaticLoadSettledSampler`: consecutive settled-world qualification window and mean accumulator;
- `TAStaticLoadAcceptance`: configurable Proof-of-Physics acceptance envelope;
- Automation source coverage for mass balance, comparison, settled sampling and acceptance.

Settled sampling contract:
- default maximum chassis linear speed: 0.02 m/s;
- default maximum chassis angular speed: 0.01 rad/s;
- default maximum per-corner sample-to-sample load delta: 0.25% of expected total support load;
- default minimum consecutive qualified samples: 120;
- threshold equality qualifies;
- invalid/non-finite evidence, excess motion or load instability resets the entire consecutive window;
- the first motion-qualified sample seeds the load-stability reference and participates in the mean;
- no partial/pre-reset samples may leak into accepted means.

Acceptance envelope defaults are deliberately provisional PoP tolerances, not vehicle-calibration truth:
- total support-load absolute relative error <= 2%;
- front/rear axle absolute relative error <= 3%;
- left/right side absolute relative error <= 3%;
- maximum absolute corner error <= 2% of expected total support load.
All component gates must pass. Threshold equality passes. Acceptance evaluation is downstream of settled qualification and comparison; it never alters physics.

Implementation properties:
- +X forward, +Y right, +Z up;
- longitudinal COM is measured from rear axle;
- each axle uses its own track width for lateral split;
- calculation/comparison/acceptance outputs are zeroed before validation so failure cannot leak stale evidence;
- lateral support-boundary cases are rejected before negative analytical corner loads can arise;
- comparison rejects negative/non-finite measured support loads and internally inconsistent analytical targets;
- signed comparison errors are `Measured - Expected`;
- relative total/axle/side errors use their corresponding expected aggregate; max corner error is normalized by expected total weight;
- all P1.1 helpers remain regression evidence only and never inject forces or corner loads into the dynamic solver.

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
14. P1.1 analytical mass balance, sampling, comparison and acceptance are evidence-only.
15. Static lateral reference split uses each axle's own track width and rejects COM positions on/outside either lateral support boundary.
16. Failure paths leave deterministic zero outputs/evidence windows.
17. Sampling requires consecutive settled evidence; a single instantaneous frame is never sufficient.
18. Load stability is normalized to expected total support load so the gate scales with vehicle mass.
19. Acceptance thresholds are isolated in a configurable envelope and may be tightened only after executable baselines exist.

## Risks
General risks remain UHT/UBT/compiler errors, unexecuted numerical Automation assertions, unmeasured coupled-contact convergence, dynamic-unsprung uncertainty, incomplete collision-manifold persistence, deferred topology-changing suspension fracture, incomplete hydraulic/ABS/fluid-boil behavior and provisional real-world calibration.

Aero-specific risks remain the missing canonical asset property/hash contribution until patch 41 lands, possible UHT/compiler issues, and accidental double compilation/hash/COM subtraction.

P1.1 risks:
- authored suspension preload/static compression may disagree with COM-derived equilibrium;
- tire vertical compliance can alter settle dynamics but not final force equilibrium;
- transient roll stiffness distribution must not be conflated with the static analytical oracle;
- contact-force sign/orientation at the eventual telemetry adapter must be normalized exactly once before comparison;
- current sampler consumes already-normalized support-load magnitudes and is not yet wired to live wheel/contact telemetry;
- fixed sample count is intentionally deterministic but corresponds to different wall-clock durations if the caller's sampling cadence changes; integration must use a documented fixed physics cadence or later move to duration-aware qualification;
- default acceptance tolerances are provisional until real UE 5.8 baseline runs exist;
- new helpers/tests are source-complete but still UE-build-unverified.

## Deliverables completed this session
- `Public/TAStaticLoadSettledSampler.h`;
- `Private/TAStaticLoadSettledSampler.cpp`;
- `Private/Tests/TAStaticLoadSettledSamplerTests.cpp`;
- `Public/TAStaticLoadAcceptance.h`;
- `Private/TAStaticLoadAcceptance.cpp`;
- `Private/Tests/TAStaticLoadAcceptanceTests.cpp`;
- refreshed active checkpoint.

## Exact continuation point
1. If a safe patch-capable path is available, land patch 41 in `TAVehicleDefinition.h/.cpp`, require `verify_aero_asset_closure.py` PASS, then run the four vehicle-definition aero regressions and effective-hash invariant.
2. Run UHT/include/module audit, UE 5.8 UBT and `Automation RunTest TorqueAtlas.` when an engine runner is available.
3. If executable Unreal verification remains unavailable, continue P1.1 by implementing the **live static-load evidence adapter**: map canonical FL/FR/RL/RR wheel/contact telemetry into non-negative support-load magnitudes, document sign normalization exactly once, and feed the settled sampler only at a fixed physics cadence.
4. Add an end-to-end evidence helper/test path: analytical oracle -> settled samples -> qualified mean -> comparison -> acceptance result, including reset/recovery and deliberate bias failures.
5. Audit whether 120 samples should remain the canonical gate or become a duration-based window once the actual fixed-step cadence is executable and measured.
6. Do not begin map-based/active aero or broad content production until the integrated Proof-of-Physics gate is executable and repeatable.

## Checkpoint rule
Update this file before ending every substantial work session and before switching to a new major subsystem.
