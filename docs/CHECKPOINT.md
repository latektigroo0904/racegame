# Active Development Checkpoint

Updated: 2026-09-24

## Current phase
**Proof-of-Physics v2 / P1.1 static mass balance.** Analytical oracle, settled sampling, comparison, acceptance, live four-wheel evidence adapter and end-to-end evidence pipeline are source-complete. Vertical-load provenance and compliant additional-reaction behavior have Automation source coverage. UE 5.8 executable verification remains the acceptance gate.

Canonical repository: `latektigroo0904/racegame`. Content/runtime versions remain SchemaVersion 2, PhysicsVersion 2 and DamageModelVersion 2.

The project remains **UE-build-unverified** until UnrealHeaderTool, UnrealBuildTool/C++ compilation, Editor module loading and `Automation RunTest TorqueAtlas.` complete successfully under Unreal Engine 5.8.

## Aerodynamics status
Aerodynamics is functionally established, but canonical main still lacks the final authored `UTAVehicleDefinition.Aerodynamics` property/hash ownership edit. Source sanity previously proved patch 41's applicator/verifier contract against the real source tree. Do not reconstruct large definition files from truncated retrieval.

Invariant: no arcade speed-dependent tire-grip multiplier. Aero grip gain must emerge from physical force application, chassis attitude/load transfer and changed tire normal loads.

## P1.1 static mass balance status
Canonical design docs: `docs/42-STATIC-MASS-BALANCE-V01.md`, `docs/43-STATIC-LOAD-EVIDENCE-PIPELINE-V01.md`, and `docs/44-VERTICAL-LOAD-PROVENANCE-AUDIT-V01.md`.

Implemented layers:
- `TAStaticMassBalance`: fail-closed analytical FL/FR/RL/RR equilibrium oracle;
- `TAStaticLoadComparison`: measured mean versus analytical signed errors;
- `TAStaticLoadSettledSampler`: consecutive settled-world evidence window;
- `TAStaticLoadAcceptance`: configurable Proof-of-Physics acceptance envelope;
- `TAStaticLoadEvidenceAdapter`: canonical four-wheel contact evidence -> sampler evidence;
- `TAStaticLoadEvidencePipeline`: oracle -> adapter -> settle -> mean -> comparison -> acceptance;
- Automation source coverage for every P1.1 layer, including end-to-end regression.

### Vertical-load provenance
`VerticalLoadN` is canonically a non-negative support-load magnitude. `SuspensionForceWorldN` owns direction. `BuildVehicleWheelContactInput` and vehicle tire-input construction preserve the scalar without a second sign conversion. P1.1 must therefore validate/preserve the scalar and never apply `Abs()` or negate it again.

`TAWheelContactLoadProvenanceTests.cpp` locks exact scalar/world-force bridge preservation, unclamped anti-roll pair-load conservation, positive road-normal projection/magnitude and airborne-zero behavior.

### Compliant midpoint reaction closure — CANONICAL FIX LANDED
Patch 44 is now landed on canonical main. Commit `9b8caca177bbf3de913f3a08bb5e4cf8f2914008` propagates `AdditionalSuspensionReactionN` through the compliant bisection midpoint evaluation, matching the endpoint evaluations and the public resolver contract.

The fix was landed through a one-shot GitHub Actions workflow using the already preflight-proven fail-closed applicator. Landing run `36003776990` completed successfully. The temporary contents-write workflow was then deleted so no persistent write-capable CI path remains.

Source sanity was changed from temporary old-state application to a fixed-state invariant: running `scripts/apply_compliant_midpoint_reaction_fix.py` must be a no-op and `git diff` must remain clean. This makes regression to the old midpoint signature fail CI.

`TAWheelContactCompliantReactionTests.cpp` covers zero-reaction determinism/equivalence and positive `AdditionalSuspensionReactionN` behavior through the public resolver boundary.

## Fixed cadence / settled evidence
P1.1 live evidence is canonical at 120 Hz (`1/120 s`) with `1e-6 s` cadence tolerance. 120 consecutive settled samples represent approximately one second. Defaults: linear speed <= 0.02 m/s; angular speed <= 0.01 rad/s; per-corner sample delta <= 0.25% of expected total support load; minimum 120 consecutive samples. Invalid evidence, motion or instability resets the window. Threshold equality passes.

Provisional acceptance defaults: total <= 2%; front/rear axle <= 3%; left/right side <= 3%; max corner error <= 2% of expected total support load.

## Decisions / invariants
1. P1.1 is evidence-only and never injects analytical or measured loads back into physics.
2. +X forward, +Y right, +Z up; COM longitudinal position is measured from rear axle.
3. Comparison signed errors are `Measured - Expected`.
4. Failure paths clear outputs/state so stale evidence cannot leak.
5. Live load sign normalization occurs exactly once at the canonical contact contract.
6. `VerticalLoadN` is magnitude; `SuspensionForceWorldN` owns direction.
7. Fixed evidence cadence is part of the regression contract.
8. `bQualified` and `Acceptance.bPass` are separate states.
9. Aero patch 41 remains the final source-level closure item before integrated PoP acceptance; patch 44 is closed.
10. Large canonical source files may be changed only through exact-anchor/fail-closed tooling or a complete blob/real patch-capable checkout; truncated retrieval is never a basis for whole-file replacement.
11. Anti-roll conservation is asserted only in the unclamped regime.
12. Additional suspension reaction is a force contribution, not a hidden load-sign conversion; positive reaction must alter the compliant equilibrium through the same force-residual solve used by the tire/suspension pair.
13. One-shot write-capable CI workflows must be removed immediately after successful canonical landing.

## Risks
- P1.1 source/tests remain UE 5.8 build-unverified.
- The positive-reaction monotonicity assertion is physically intended but still requires UE execution; if it fails after patch 44, diagnose equilibrium/sign semantics rather than weakening the test immediately.
- Acceptance tolerances remain provisional until real stationary baselines are captured.
- Authored suspension preload/static compression may disagree with COM-derived equilibrium and must be diagnosed rather than hidden by wider gates.
- Aero canonical asset property/hash contribution remains open until patch 41 lands.
- Source-sanity after the fixed-state workflow change must be observed green before treating CI closure as complete.

## Deliverables completed this session
- Created a narrow one-shot landing workflow for patch 44 with `contents: write`, exact one-file diff gate and the preflight-proven applicator.
- Observed landing workflow run `36003776990` complete successfully.
- Landed canonical resolver fix as commit `9b8caca177bbf3de913f3a08bb5e4cf8f2914008`.
- Converted Source sanity to require patch 44's canonical fixed state/no-op applicator behavior.
- Deleted the one-shot write-capable workflow after use.
- Preserved UE-build-unverified status pending real engine execution.

## Exact continuation point
1. Observe Source sanity for the fixed-state commits and resolve any CI regression before moving on.
2. Land aero patch 41 through the same safe exact-anchor/complete-blob discipline; require `verify_aero_asset_closure.py` PASS and convert its CI preflight to canonical fixed-state verification.
3. Run UHT/include/module audit and UE 5.8 UBT for the complete source set.
4. Run `TorqueAtlas.Suspension.Contact.Compliant.*`, then `Automation RunTest TorqueAtlas.Vehicle.StaticLoad.`, `TorqueAtlas.Suspension.Contact.*`, and full `Automation RunTest TorqueAtlas.`.
5. Diagnose any positive-reaction monotonicity failure as physics/sign semantics, not by widening tolerances.
6. Capture a real stationary four-wheel baseline at 120 Hz and use executable evidence to decide whether the 1.0 s window and provisional 2/3/3/2% envelope should be tightened.
7. Do not begin map-based/active aero or broad content production until integrated Proof-of-Physics is executable and repeatable.

## Checkpoint rule
Update this file before ending every substantial work session and before switching to a new major subsystem.
