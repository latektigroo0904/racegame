# Active Development Checkpoint

Updated: 2026-09-24

## Current phase
**Proof-of-Physics v2 / P1.1 static mass balance.** Analytical oracle, settled sampling, comparison, acceptance, live four-wheel evidence adapter and end-to-end evidence pipeline are source-complete. Vertical-load provenance is audited and direct bridge/anti-roll invariants now have Automation coverage. UE 5.8 executable verification remains the acceptance gate.

Canonical repository: `latektigroo0904/racegame`. Content/runtime versions remain SchemaVersion 2, PhysicsVersion 2 and DamageModelVersion 2.

The project remains **UE-build-unverified** until UnrealHeaderTool, UnrealBuildTool/C++ compilation, Editor module loading and `Automation RunTest TorqueAtlas.` complete successfully under Unreal Engine 5.8.

## Aerodynamics status
Aerodynamics is functionally established, but canonical main still lacks the final authored `UTAVehicleDefinition.Aerodynamics` property/hash ownership edit. Source sanity previously proved patch 41's applicator/verifier contract against the real source tree. Do not reconstruct the large definition files from truncated retrieval.

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

### Vertical-load provenance audit
`VerticalLoadN` is canonically a non-negative support-load magnitude. The geometric contact path clamps suspension total force once and builds `SuspensionForceWorldN = RoadNormal * VerticalLoadN`. The compliant path uses tire `NormalForceN` as final `VerticalLoadN`. `BuildVehicleWheelContactInput` and `TAVehicleSimulation::BuildTireInput` copy the scalar without sign conversion; chassis force application uses the separate world-force vector. P1.1 therefore must validate/preserve the scalar and never apply `Abs()` or negate it again.

`TAWheelContactLoadProvenanceTests.cpp` now locks three downstream invariants: exact positive scalar/world-force bridge preservation; equal/opposite anti-roll transfer conserves pair support load when neither wheel clamps; and anti-roll processing cannot manufacture support on an airborne wheel. It also checks that in-contact world suspension force remains positively aligned with the road normal and has magnitude equal to `VerticalLoadN`.

### Compliant midpoint reaction closure
The midpoint `EvaluateCompliantTravel` call in `ResolveDoubleWishboneCompliantRoadContact` currently omits required `AdditionalSuspensionReactionN`, while endpoint calls supply it. Patch 44 contains the one-line correction.

A fail-closed applicator exists at `scripts/apply_compliant_midpoint_reaction_fix.py`. It is idempotent, requires exactly one canonical midpoint anchor, refuses ambiguous/mixed/source-drift states, and never reconstructs the large resolver file. `tests/tools/test_apply_compliant_midpoint_reaction_fix.py` covers application, idempotence and refusal states. Source-sanity CI exercises the applicator against the actual canonical resolver, requires `git diff --check`, requires exactly one changed file (`TAWheelContactResolver.cpp`), then restores that file before exercising aero closure. Source sanity run 172 on checkpoint commit `c05a13ac59f9892b9aca75973b18d8ba883a1a8f` completed successfully, so patch 44 is mechanically closure-ready against the current resolver.

### Fixed cadence decision
P1.1 live evidence is canonical at 120 Hz (`1/120 s`) with `1e-6 s` cadence tolerance. 120 consecutive settled samples represent approximately one second. Cadence drift is rejected.

### Settled gate
Defaults: linear speed <= 0.02 m/s; angular speed <= 0.01 rad/s; per-corner sample delta <= 0.25% of expected total support load; minimum 120 consecutive samples. Invalid evidence, motion or instability resets the window. Threshold equality passes.

### Acceptance envelope
Provisional defaults: total <= 2%; front/rear axle <= 3%; left/right side <= 3%; max corner error <= 2% of expected total support load. Valid qualified evidence may legitimately fail acceptance.

## Decisions / invariants
1. P1.1 is evidence-only and never injects analytical or measured loads back into physics.
2. +X forward, +Y right, +Z up; COM longitudinal position is measured from rear axle.
3. Comparison signed errors are `Measured - Expected`.
4. Failure paths clear outputs/state so stale evidence cannot leak.
5. Live load sign normalization occurs exactly once at the canonical contact contract.
6. `VerticalLoadN` is magnitude; `SuspensionForceWorldN` owns direction.
7. Chassis settled speeds are vector magnitudes from canonical chassis linear/angular velocity.
8. Fixed evidence cadence is part of the regression contract.
9. `bQualified` and `Acceptance.bPass` are separate states.
10. Aero patch 41 and compliant-contact patch 44 are independent closure items; both must land before integrated PoP acceptance.
11. Large canonical source files must be changed only through exact-anchor/fail-closed tooling or a complete blob/real patch-capable checkout; truncated retrieval is never a basis for whole-file replacement.
12. Anti-roll conservation is asserted only in the unclamped regime; once a wheel unloads, non-negative clamping intentionally breaks algebraic pair equality.

## Risks
- P1.1 source/tests remain UE 5.8 build-unverified.
- Patch 44 is mechanically preflight-proven but not yet applied to canonical resolver; current source remains compile/API-blocked at the midpoint call.
- Acceptance tolerances remain provisional until real stationary baselines are captured.
- Authored suspension preload/static compression may disagree with COM-derived equilibrium and must be diagnosed rather than hidden by wider gates.
- Aero canonical asset property/hash contribution remains open until patch 41 lands.
- New provenance tests are source-complete but cannot be treated as passing until UBT/Automation executes them.

## Deliverables completed this session
- Observed Source sanity run 172 green on the patch-44 canonical-source preflight.
- Re-fetched the complete canonical resolver blob and reconfirmed the midpoint omission plus downstream positive-load ownership.
- Added `TAWheelContactLoadProvenanceTests.cpp` with exact bridge preservation, unclamped anti-roll conservation, positive road-normal projection/magnitude, and airborne-zero regressions.
- Refreshed this checkpoint and removed the now-obsolete risk that patch-44 preflight was still awaiting CI.

## Exact continuation point
1. Apply/land patch 44 in `TAWheelContactResolver.cpp` through a safe complete-blob or patch-capable route; then adjust source-sanity so canonical fixed state is accepted rather than temporarily applying the old-state patch.
2. Add compliant-contact executable regressions for zero-reaction equivalence and positive `AdditionalSuspensionReactionN` behavior. The new provenance suite already covers anti-roll conservation, exact scalar bridge preservation and non-negative world-normal force projection.
3. Land aero patch 41 and require `verify_aero_asset_closure.py` PASS.
4. Run UHT/include/module audit and UE 5.8 UBT for the complete source set.
5. Run `Automation RunTest TorqueAtlas.Vehicle.StaticLoad.` plus `TorqueAtlas.Suspension.Contact.*`, then full `Automation RunTest TorqueAtlas.`.
6. Capture a real stationary four-wheel baseline at 120 Hz and use executable evidence to decide whether the 1.0 s window and provisional 2/3/3/2% envelope should be tightened.
7. Do not begin map-based/active aero or broad content production until integrated Proof-of-Physics is executable and repeatable.

## Checkpoint rule
Update this file before ending every substantial work session and before switching to a new major subsystem.
