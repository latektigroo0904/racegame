# Active Development Checkpoint

Updated: 2026-09-24

## Current phase
**Proof-of-Physics v2 / P1.1 static mass balance.** The analytical oracle, settled sampling, comparison, acceptance, live four-wheel evidence adapter and end-to-end evidence pipeline are now source-complete. UE 5.8 executable verification remains the acceptance gate.

Canonical repository: `latektigroo0904/racegame`. Content/runtime versions remain SchemaVersion 2, PhysicsVersion 2 and DamageModelVersion 2.

The project remains **UE-build-unverified** until UnrealHeaderTool, UnrealBuildTool/C++ compilation, Editor module loading and `Automation RunTest TorqueAtlas.` complete successfully under Unreal Engine 5.8.

## Aerodynamics status
Aerodynamics remains functionally established, but canonical main still lacks the final authored `UTAVehicleDefinition.Aerodynamics` property/hash ownership edit. Source sanity run 140 previously proved the exact patch-41 applicator/verifier contract against the real source tree. Do not fake or reconstruct the large canonical definition files from truncated retrieval.

Important invariant: no arcade speed-dependent tire-grip multiplier. Aero grip gain must emerge from physical force application, chassis attitude/load transfer and changed tire normal loads.

## P1.1 static mass balance status
Canonical design docs: `docs/42-STATIC-MASS-BALANCE-V01.md` and `docs/43-STATIC-LOAD-EVIDENCE-PIPELINE-V01.md`.

Implemented layers:
- `TAStaticMassBalance`: fail-closed analytical FL/FR/RL/RR equilibrium oracle;
- `TAStaticLoadComparison`: measured mean versus analytical signed errors;
- `TAStaticLoadSettledSampler`: consecutive settled-world evidence window;
- `TAStaticLoadAcceptance`: configurable Proof-of-Physics acceptance envelope;
- `TAStaticLoadEvidenceAdapter`: maps canonical four-wheel runtime/contact evidence into sampler evidence;
- `TAStaticLoadEvidencePipeline`: oracle -> adapter -> settle -> mean -> comparison -> acceptance orchestration;
- Automation source coverage for every layer, including an end-to-end regression.

### Live evidence ownership
`FTAWheelContactInput::VerticalLoadN` is consumed as an already non-negative support-load magnitude. The adapter performs no second sign inversion. Corner mapping is explicit FL/FR/RL/RR from the corresponding front/rear axle left/right `VehicleContact`. Negative/non-finite loads, non-finite chassis velocities, unsolved contacts or invalid cadence fail closed.

### Fixed cadence decision
P1.1 live evidence is canonical at 120 Hz (`1/120 s`) with `1e-6 s` cadence tolerance. The existing 120 consecutive settled samples therefore represent approximately one second. Cadence drift is rejected instead of silently changing the physical qualification duration. If production physics cadence changes, migrate sample count/duration deliberately with executable evidence.

### Settled gate
Defaults remain: linear speed <= 0.02 m/s; angular speed <= 0.01 rad/s; per-corner sample delta <= 0.25% of expected total support load; minimum 120 consecutive samples. Invalid evidence, motion or instability resets the window. Threshold equality passes.

### Acceptance envelope
Provisional defaults remain: total <= 2%; front/rear axle <= 3%; left/right side <= 3%; max corner error <= 2% of expected total support load. A qualified result may legitimately fail acceptance; valid evidence of bad physics is distinct from invalid evidence.

## Decisions / invariants
1. P1.1 is evidence-only and never injects analytical or measured loads back into physics.
2. +X forward, +Y right, +Z up; COM longitudinal position is measured from rear axle.
3. Comparison signed errors are `Measured - Expected`.
4. Failure paths clear outputs/state so stale evidence cannot leak.
5. Live load sign normalization occurs exactly once at the canonical contact contract; the adapter only validates/preserves magnitude.
6. Chassis settled speeds are vector magnitudes from canonical chassis linear/angular velocity.
7. Fixed evidence cadence is part of the regression contract, not an incidental caller detail.
8. `bQualified` means a complete valid settled window exists; `Acceptance.bPass` separately states whether that evidence satisfies PoP tolerances.
9. Aero patch 41 remains independent and must still land before aero ownership is considered closed.

## Risks
- All new P1.1 source/tests remain UE 5.8 build-unverified; compile/API mistakes are still possible until UBT executes.
- Runtime provenance of `VerticalLoadN` must be confirmed in executable suspension/contact tests; the P1.1 consumer convention is frozen but does not replace solver validation.
- Acceptance tolerances remain provisional until real stationary baselines are captured.
- Authored suspension preload/static compression may disagree with COM-derived equilibrium and must be diagnosed, not hidden by loosening gates.
- Aero canonical asset property/hash contribution remains open until patch 41 lands.

## Deliverables completed this session
- `Public/TAStaticLoadEvidenceAdapter.h`;
- `Private/TAStaticLoadEvidenceAdapter.cpp`;
- `Private/Tests/TAStaticLoadEvidenceAdapterTests.cpp`;
- `Public/TAStaticLoadEvidencePipeline.h`;
- `Private/TAStaticLoadEvidencePipeline.cpp`;
- `Private/Tests/TAStaticLoadEvidencePipelineTests.cpp`;
- `docs/43-STATIC-LOAD-EVIDENCE-PIPELINE-V01.md`;
- refreshed active checkpoint.

## Exact continuation point
1. If a safe patch-capable path is available, land aero patch 41, require `verify_aero_asset_closure.py` PASS, then run vehicle-definition aero regressions/effective-hash invariant.
2. Run UHT/include/module audit and UE 5.8 UBT for the complete P1.1 + aero source set when an engine runner is available.
3. Run `Automation RunTest TorqueAtlas.Vehicle.StaticLoad.` followed by full `Automation RunTest TorqueAtlas.`.
4. Capture a real stationary four-wheel baseline at 120 Hz: settle time, FL/FR/RL/RR mean, aggregate/corner error distribution and reset/recovery behavior.
5. Use that executable evidence to decide whether 1.0 s/120 samples and the provisional 2/3/3/2% envelope should be tightened; do not tune thresholds from source-only reasoning.
6. If Unreal execution remains unavailable, next source-only work is an audit of `VerticalLoadN` production through front/rear contact resolvers and suspension force application, with a regression proving support-load sign/provenance exactly once.
7. Do not begin map-based/active aero or broad content production until the integrated Proof-of-Physics gate is executable and repeatable.

## Checkpoint rule
Update this file before ending every substantial work session and before switching to a new major subsystem.
