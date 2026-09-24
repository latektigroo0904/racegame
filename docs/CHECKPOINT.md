# Active Development Checkpoint

Updated: 2026-09-24

## Current phase
**Proof-of-Physics v2 / P1.1 static mass balance.** Analytical oracle, settled sampling, comparison, acceptance, live four-wheel evidence adapter and end-to-end evidence pipeline are source-complete. Vertical-load provenance has now been audited from contact resolution through chassis force application. UE 5.8 executable verification remains the acceptance gate.

Canonical repository: `latektigroo0904/racegame`. Content/runtime versions remain SchemaVersion 2, PhysicsVersion 2 and DamageModelVersion 2.

The project remains **UE-build-unverified** until UnrealHeaderTool, UnrealBuildTool/C++ compilation, Editor module loading and `Automation RunTest TorqueAtlas.` complete successfully under Unreal Engine 5.8.

## Aerodynamics status
Aerodynamics is functionally established, but canonical main still lacks the final authored `UTAVehicleDefinition.Aerodynamics` property/hash ownership edit. Source sanity run 140 previously proved patch 41's applicator/verifier contract against the real source tree. Do not reconstruct the large definition files from truncated retrieval.

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

### Defect found: compliant midpoint reaction
The midpoint `EvaluateCompliantTravel` call in `ResolveDoubleWishboneCompliantRoadContact` omits the required `AdditionalSuspensionReactionN` argument while endpoint calls supply it. This is a compile/API defect and would also invalidate anti-roll-coupled bisection semantics if masked by an overload/default. A minimal correction is prepared as `patches/44-compliant-contact-midpoint-reaction.patch`. It is not yet landed in the canonical large source file.

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

## Risks
- P1.1 source/tests remain UE 5.8 build-unverified.
- Patch 44 is prepared but not applied; current canonical compliant resolver should be treated as compile-blocked until corrected.
- Acceptance tolerances remain provisional until real stationary baselines are captured.
- Anti-roll equal/opposite conservation must be tested before final zero-load clamping because wheel unload can break equality after clamp.
- Authored suspension preload/static compression may disagree with COM-derived equilibrium and must be diagnosed rather than hidden by wider gates.
- Aero canonical asset property/hash contribution remains open until patch 41 lands.

## Deliverables completed this session
- source-level provenance audit of geometric/compliant contact, anti-roll coupling, vehicle-contact bridge, tire consumer and chassis force application;
- `patches/44-compliant-contact-midpoint-reaction.patch`;
- `docs/44-VERTICAL-LOAD-PROVENANCE-AUDIT-V01.md`;
- refreshed active checkpoint.

## Exact continuation point
1. Land patch 44 safely in `TAWheelContactResolver.cpp`; verify every `EvaluateCompliantTravel` call supplies `AdditionalSuspensionReactionN`.
2. Add/extend compliant-contact regressions for zero-reaction equivalence, positive-reaction behavior, anti-roll conservation before clamp, exact scalar bridge preservation, and non-negative world-normal force projection.
3. If a safe patch-capable path is available, also land aero patch 41 and require `verify_aero_asset_closure.py` PASS.
4. Run UHT/include/module audit and UE 5.8 UBT for the complete source set.
5. Run `Automation RunTest TorqueAtlas.Vehicle.StaticLoad.` then full `Automation RunTest TorqueAtlas.`.
6. Capture a real stationary four-wheel baseline at 120 Hz and use executable evidence to decide whether the 1.0 s window and provisional 2/3/3/2% envelope should be tightened.
7. Do not begin map-based/active aero or broad content production until integrated Proof-of-Physics is executable and repeatable.

## Checkpoint rule
Update this file before ending every substantial work session and before switching to a new major subsystem.