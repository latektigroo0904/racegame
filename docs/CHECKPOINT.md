# Active Development Checkpoint

Updated: 2026-09-24

## Current phase
**Proof-of-Physics v2 / P1.1 static mass balance — canonical source closure and UE 5.8 source preflight complete.** Analytical oracle, settled sampling, comparison, acceptance, live four-wheel evidence adapter and end-to-end evidence pipeline are source-complete. Vertical-load provenance and compliant additional-reaction behavior have Automation source coverage. Patch 44 and aero patch 41 are both landed on canonical `main`. A source-level UHT/include/module audit found no deterministic dependency defect requiring speculative edits. UE 5.8 executable verification is now the acceptance gate.

Canonical repository: `latektigroo0904/racegame`. Content/runtime versions remain SchemaVersion 2, PhysicsVersion 2 and DamageModelVersion 2.

The project remains **UE-build-unverified** until UnrealHeaderTool, UnrealBuildTool/C++ compilation, Editor module loading and `Automation RunTest TorqueAtlas.` complete successfully under Unreal Engine 5.8.

## Aerodynamics status — CANONICAL OWNERSHIP CLOSED
Aero patch 41 landed as commit `8f76d4cfe2767c2cb5a281afc06584d7d65abc27`. `UTAVehicleDefinition` owns authored `FTAAerodynamicsDefinition Aerodynamics`; validation rejects invalid authored aero; canonical compilation converts the authored vehicle-origin-local application point to COM-local exactly once; and the compiled aero runtime contributes transactionally to the vehicle physics hash.

The landing used `scripts/apply_aero_asset_closure.py` plus `scripts/verify_aero_asset_closure.py` under an exact two-file diff gate. The temporary `contents: write` workflow was removed immediately after landing. Source sanity treats the applicator as a fixed-state no-op and requires the verifier to pass with a clean git diff.

Invariant: no arcade speed-dependent tire-grip multiplier. Aero grip gain must emerge from physical force application, chassis attitude/load transfer and changed tire normal loads.

## UE 5.8 source preflight
Canonical audit: `docs/45-UE58-UHT-MODULE-AUDIT-V01.md`.

Findings:
- `TA_Vehicle` already declares the public engine/subsystem dependencies required by its exported vehicle types.
- the game module keeps `TA_Core`/`TA_Vehicle` private and does not leak plugin implementation dependencies publicly;
- `TAVehicleDefinition.h` includes `TAAerodynamicsDefinition.h` before its generated header, with `TAVehicleDefinition.generated.h` remaining the final include;
- `TAAerodynamicsDefinition.h` follows the same generated-header ordering and exports its reflected struct with `TA_VEHICLE_API`;
- P1.1 evidence classes remain inside `TA_Vehicle`, so no additional module edge is justified;
- no speculative Build.cs/include edit is warranted without UHT/UBT evidence.

Source inspection cannot prove generated-code, compiler, module-load or Automation success; executable status remains unverified.

## P1.1 static mass balance status
Canonical design docs: `docs/42-STATIC-MASS-BALANCE-V01.md`, `docs/43-STATIC-LOAD-EVIDENCE-PIPELINE-V01.md`, `docs/44-VERTICAL-LOAD-PROVENANCE-AUDIT-V01.md`, and `docs/45-UE58-UHT-MODULE-AUDIT-V01.md`.

Implemented layers:
- `TAStaticMassBalance`: fail-closed analytical FL/FR/RL/RR equilibrium oracle;
- `TAStaticLoadComparison`: measured mean versus analytical signed errors;
- `TAStaticLoadSettledSampler`: consecutive settled-world evidence window;
- `TAStaticLoadAcceptance`: configurable Proof-of-Physics acceptance envelope;
- `TAStaticLoadEvidenceAdapter`: canonical four-wheel contact evidence -> sampler evidence;
- `TAStaticLoadEvidencePipeline`: oracle -> adapter -> settle -> mean -> comparison -> acceptance;
- Automation source coverage for every P1.1 layer, including end-to-end regression.

### Vertical-load provenance
`VerticalLoadN` is canonically a non-negative support-load magnitude. `SuspensionForceWorldN` owns direction. `BuildVehicleWheelContactInput` and vehicle tire-input construction preserve the scalar without a second sign conversion. P1.1 validates/preserves the scalar and never applies `Abs()` or negates it again.

`TAWheelContactLoadProvenanceTests.cpp` locks exact scalar/world-force bridge preservation, unclamped anti-roll pair-load conservation, positive road-normal projection/magnitude and airborne-zero behavior.

### Compliant midpoint reaction closure — CANONICAL FIX LANDED
Patch 44 is landed as commit `9b8caca177bbf3de913f3a08bb5e4cf8f2914008`. It propagates `AdditionalSuspensionReactionN` through the compliant bisection midpoint evaluation, matching endpoint evaluations and the public resolver contract. Source sanity requires the patch-44 applicator to be a no-op on canonical state.

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
9. Patch 44 and aero patch 41 are source-closed; no known source-ownership closure remains before integrated PoP execution.
10. Large canonical source files may be changed only through exact-anchor/fail-closed tooling or a complete blob/real patch-capable checkout; truncated retrieval is never a basis for whole-file replacement.
11. Anti-roll conservation is asserted only in the unclamped regime.
12. Additional suspension reaction is a force contribution, not a hidden load-sign conversion.
13. One-shot write-capable CI workflows must be removed immediately after successful canonical landing.
14. Do not change module dependencies or include topology speculatively when the source graph is internally consistent; let UHT/UBT produce the next actionable defect.

## Risks
- P1.1 source/tests remain UE 5.8 build-unverified.
- The positive-reaction monotonicity assertion is physically intended but still requires UE execution; diagnose equilibrium/sign semantics rather than weakening the test if it fails.
- Acceptance tolerances remain provisional until real stationary baselines are captured.
- Authored suspension preload/static compression may disagree with COM-derived equilibrium and must be diagnosed rather than hidden by wider gates.
- No available hosted runner currently proves Unreal 5.8 UHT/UBT; executable acceptance requires a UE 5.8-capable environment.
- Source inspection cannot expose generated-code or engine-version-specific compile failures.

## Deliverables completed this session
- Audited the actual `TorqueAtlas.Build.cs` and plugin `TA_Vehicle.Build.cs` dependency direction.
- Audited UHT-facing generated-header ordering for canonical `TAVehicleDefinition.h` and `TAAerodynamicsDefinition.h`.
- Confirmed P1.1 remains module-local to `TA_Vehicle`; no new dependency edge is justified.
- Added `docs/45-UE58-UHT-MODULE-AUDIT-V01.md` with executable gate and failure semantics.
- Deliberately made no speculative source/Build.cs changes where engine evidence is required.
- Preserved UE-build-unverified status pending real engine execution.

## Exact continuation point
1. Run UE 5.8 UHT/UBT for `TorqueAtlasEditor` Development on a UE-capable environment; capture engine/compiler/command/exit status.
2. Fix the first deterministic UHT/compile defect only if one appears, then rebuild before touching physics tolerances.
3. Run `TorqueAtlas.Suspension.Contact.Compliant.*`.
4. Run `Automation RunTest TorqueAtlas.Vehicle.StaticLoad.` and then `TorqueAtlas.Suspension.Contact.*`.
5. Run full `Automation RunTest TorqueAtlas.`.
6. Diagnose any positive-reaction monotonicity failure as physics/sign semantics, not by widening tolerances.
7. Capture a real stationary four-wheel baseline at 120 Hz and use executable evidence to decide whether the 1.0 s window and provisional 2/3/3/2% envelope should be tightened.
8. Do not begin map-based/active aero or broad content production until integrated Proof-of-Physics is executable and repeatable.

## Checkpoint rule
Update this file before ending every substantial work session and before switching to a new major subsystem.
