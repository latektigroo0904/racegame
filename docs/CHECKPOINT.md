# Active Development Checkpoint

Updated: 2026-10-08

## Current phase
**Proof-of-Physics v2 / P1.1 static mass balance — canonical source closure and UE 5.8 source preflight complete.** Analytical oracle, settled sampling, comparison, acceptance, live four-wheel evidence adapter and end-to-end evidence pipeline are source-complete. Vertical-load provenance and compliant additional-reaction behavior have Automation source coverage. Patch 44 and aero patch 41 are both landed on canonical `main`. A source-level UHT/include/module audit found no deterministic dependency defect requiring speculative edits. UE 5.8 executable verification is now the acceptance gate.

Canonical repository: `latektigroo0904/racegame`. Content/runtime versions remain SchemaVersion 2, PhysicsVersion 2 and DamageModelVersion 2.

The project remains **UE-build-unverified** until UnrealHeaderTool, UnrealBuildTool/C++ compilation, Editor module loading and `Automation RunTest TorqueAtlas.` complete successfully under Unreal Engine 5.8.

## Aerodynamics status — SPLIT BALANCE SOURCE-CLOSED

The earlier one-lift-coefficient/resultant simplification has been superseded by a split front/rear source model.

Canonical runtime now owns:
- one reference area;
- one drag coefficient and drag application point;
- one front lift/downforce coefficient and application point;
- one rear lift/downforce coefficient and application point.

Aerodynamic forces are applied through `TAVehicleAerodynamicsBridge` into the same chassis force accumulator as tire/suspension loads before the single chassis integration.

There is no speed-dependent tire-grip multiplier. Any grip increase must emerge through physical aerodynamic loading, chassis attitude, suspension/contact response and resulting tire normal loads.

Compact telemetry/regression now records:
- relative air speed;
- dynamic pressure;
- drag;
- front lift/downforce;
- rear lift/downforce;
- total aero force/torque;
- diagnostic front-downforce balance.

Direct source regression proves:
- V² drag scaling;
- headwind effect;
- negative-Cl downforce;
- application-point moment generation;
- front-heavy aero creates nose-down pitch;
- symmetric front/rear downforce cancels lift-induced pitch for symmetric points.

Latest split-aero source-sanity evidence:
`ac1f0bd1f8b57923d695b0e4abe89ca6a36d60d5` — GitHub Actions run 220 — success.

Canonical design note:
`docs/46-AERO-SPLIT-BALANCE-V01.md`.

The aero model is still **UE-build-unverified** and not real-world calibrated.

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

## Architecture prepared beyond current executable gate

Implementation-ready design now exists for:
- post-Proof subsystem dependency order;
- hydraulic brake actuation and ABS;
- differential backend architecture;
- tire transient dynamics;
- physical coolant/fuel/oil/brake-fluid networks;
- 12 V battery/starter/alternator/bus network;
- ABS/TCS/ESC/launch and steering FFB controller boundaries;
- subsystem promotion/test gates;
- physics calibration provenance/versioning;
- Proof-of-Physics → 20–25 km² world transition;
- multiplayer vehicle/damage replication.

Canonical documents:
- `47-POST-POP-PHYSICS-ROADMAP-V01.md`;
- `48-HYDRAULIC-BRAKE-ABS-V01.md`;
- `49-DIFFERENTIAL-TRACTION-ARCHITECTURE-V01.md`;
- `50-TIRE-TRANSIENT-DYNAMICS-V01.md`;
- `51-VEHICLE-FLUID-NETWORKS-V01.md`;
- `52-VEHICLE-ELECTRICAL-NETWORK-V01.md`;
- `53-DRIVER-ASSISTS-FFB-V01.md`;
- `54-VEHICLE-PHYSICS-PROMOTION-TEST-MATRIX-V01.md`;
- `55-PHYSICS-CALIBRATION-PROVENANCE-V01.md`;
- `56-POP-TO-WORLD-TRANSITION-V01.md`;
- `57-MULTIPLAYER-VEHICLE-REPLICATION-V01.md`.

These are design/implementation contracts, not claims of runtime validation.

## Post-gate implementation order

After the existing UE 5.8 executable baseline is captured:

1. implement isolated hydraulic brake solver;
2. add authored brake hydraulic config + hash + telemetry;
3. integrate hydraulic actuation while preserving existing thermal/fade/wear capacity;
4. implement per-wheel ABS pressure controller;
5. promote after braking/ABS traces pass;
6. implement backend-neutral differential solver;
7. add tire transient relaxation layer;
8. evaluate/promote dynamic unsprung four-corner path;
9. implement physical fluid networks;
10. implement physical electrical network;
11. layer TCS/ESC/FFB controllers;
12. begin integrated world/GeoForge prototype.

## Exact continuation point

### Highest-priority external gate
1. Run UE 5.8 UHT/UBT for `TorqueAtlasEditor` Development.
2. Fix the first deterministic UHT/compiler defect only if one appears.
3. Run `TorqueAtlas.Suspension.Contact.Compliant.*`.
4. Run `Automation RunTest TorqueAtlas.Vehicle.StaticLoad.`.
5. Run `TorqueAtlas.Suspension.Contact.*`.
6. Run full `Automation RunTest TorqueAtlas.`.
7. Capture stationary four-wheel baseline at 120 Hz with engine/compiler/commit/hash provenance.

### If executable UE remains unavailable
Continue only with work that does not falsely promote unverified physics:
1. isolated solver primitives behind non-canonical interfaces;
2. tooling/validation/calibration manifests;
3. GeoForge offline schema/tooling;
4. traffic/world data contracts;
5. network serialization contracts;
6. content archetype generation;
7. test fixtures and expected analytical oracles.

### First isolated source module allowed next
**Hydraulic brake actuation primitive**, following `docs/48-HYDRAULIC-BRAKE-ABS-V01.md`.

Initial implementation boundary:
- pedal/master-cylinder pressure request;
- front/rear circuit state;
- bounded pressure rise/release;
- per-corner hydraulic torque conversion;
- circuit-health/leak authority;
- no replacement of canonical vehicle brake path yet;
- unit tests only;
- no ABS until the pressure actuator primitive is stable.

## Checkpoint rule
Update this file before ending every substantial work session and before switching to a new major subsystem.
