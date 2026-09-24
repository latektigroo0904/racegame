# Active Development Checkpoint

Updated: 2026-09-24

## Current phase
**Proof-of-Physics v2: aerodynamic solver, force application, applied telemetry and machine-readable regression reporting are source-level closed. Asset-level regression coverage is versioned. The remaining canonical aero ownership edit is the authored `UTAVehicleDefinition` property plus its compile/hash call site. A safe local applicator now exists to remove the GitHub whole-file replacement limitation. UE 5.8 executable verification remains the external acceptance gate.**

Canonical repository: `latektigroo0904/racegame`. Current content/runtime versions remain SchemaVersion 2, PhysicsVersion 2 and DamageModelVersion 2.

## Established source-level foundation
The repository contains four-wheel compliant contact; front shared-rack double wishbone; rear five-link geometry; tire force/thermal/wear/radial compliance; engine/clutch/gearbox/final-drive/differential dynamics; 6-DOF chassis; structural crash deformation and suspension pickup coupling; typed functional damage; brake thermal/fade/wear; telemetry/regression reporting; vehicle calibration authoring; isolated dynamic-unsprung experimentation; deterministic aerodynamic solver/bridge/authoring/compiler helpers; and UE 5.8 verification runners.

The project remains **UE-build-unverified** until UnrealHeaderTool, UnrealBuildTool/C++ compilation, Editor module loading and `Automation RunTest TorqueAtlas.` complete successfully under Unreal Engine 5.8.

## Aerodynamics status
Established:
- deterministic `TAAerodynamics` solver;
- `FTAVehicleRuntimeConfig::Aerodynamics` ownership;
- transient `FTAVehicleStepInput::AerodynamicsEnvironment` ownership;
- exact result slot in `FTAVehicleStepOutput::Aerodynamics`;
- `TAVehicleAerodynamicsBridge` as the single adapter into the shared chassis force accumulator;
- Blueprint-authorable `FTAAerodynamicsDefinition` with validation, vehicle-origin-local -> COM-local compilation and effective-runtime hashing helper;
- canonical `TAVehicleSimulation::Step` aero call immediately before the single chassis integration;
- exact applied-result aero telemetry and scalar regression extraction;
- full-step source regressions for moving-air response, zero-density neutrality and headwind amplification;
- effective aero hash coordinate-invariance regression;
- `TAVehicleAerodynamicsAssetCompiler::CompileValidatedAndHash` transactional asset adapter;
- asset-compiler regressions using frozen non-default values and non-zero COM;
- `TAAeroRegressionReport` machine-readable summary over exact applied compact telemetry, with JSON-lines and CSV export;
- exact vehicle-definition closure patch versioned as `patches/41-aero-vehicle-definition-callsite.patch`;
- `scripts/verify_aero_asset_closure.py`, a static source gate enforcing property/include/validation/compiler-call presence, exactly-one canonical compiler call, COM/runtime destination and `structure hash -> aero hash -> final PhysicsConfigHash` ordering;
- `scripts/apply_aero_asset_closure.py`, an idempotent, fail-closed local source applicator that validates all anchors before writing either canonical source file;
- `TAVehicleDefinitionAerodynamicsTests.cpp`, providing asset-level non-default propagation, invalid-authoring rejection, full physics-hash sensitivity and deterministic repeated compile/hash regressions.

Important invariant: **no arcade speed-dependent tire-grip multiplier**. Aero grip gain must emerge from physical force application, chassis attitude/load transfer and changed tire normal loads.

## Telemetry ownership decision
`docs/41-TELEMETRY-OWNERSHIP-ADR-V01.md` remains accepted for Proof-of-Physics v2:
- `FTATelemetrySample` is the canonical compact applied-step physics evidence stream, including exact applied aero;
- `FTAVehicleTelemetrySample` remains the broad diagnostic/state-correlation stream;
- aero is not duplicated into the broad stream during this phase;
- telemetry never re-solves physics;
- structural consolidation is deferred until UE 5.8 executable evidence exists and consumers/export schemas can be migrated with golden-output tests.

## Work completed this session
1. Re-read the active checkpoint and exact patch 41.
2. Re-audited the canonical header and confirmed `UTAVehicleDefinition` still ends at `Structure`; authored aero is not yet present.
3. Confirmed the asset-level aero tests intentionally reference `Definition->Aerodynamics`, keeping the missing source edit visible as a compile-time closure gate.
4. Added `scripts/apply_aero_asset_closure.py` to make patch 41 safely executable from any local checkout without relying on a line-patch API.
5. The applicator is idempotent, checks exact source anchors, refuses ambiguous/missing anchors, constructs both edited files before any write, and preserves the single intended validation/compile/hash ownership path.
6. Refreshed this checkpoint.

## Decisions and assumptions
1. Vehicle asset aero authoring uses vehicle-origin-local coordinates; runtime application point is COM-local.
2. COM subtraction happens exactly once inside the established aero compile path.
3. Aero hash contribution follows effective compiled aero values, never raw authored coordinates.
4. Complete vehicle `PhysicsConfigHash` includes physical COM and therefore is not required to be invariant under an authored origin/COM shift. Only the isolated effective-aero hash contribution has that coordinate-invariance contract.
5. Air density and wind remain transient environment state and are not asset-hashed.
6. Negative lift coefficient means downforce under the current solver convention.
7. Telemetry/regression consume applied step output and never independently recompute aero.
8. Asset integration regressions use non-default values and non-zero COM to prevent default-value masking.
9. `TAVehicleAerodynamicsAssetCompiler` is the single intended asset call site for validate/compile/hash.
10. Failed asset aero compilation is transactional.
11. Single-resultant aero remains Proof-of-Physics scope; map-based front/rear balance and active aero remain deferred pending executable evidence.
12. Compact telemetry owns exact applied-step physics evidence; broad telemetry owns high-dimensional diagnostic/state correlation for this phase.
13. Static source verification is a pre-build guard only; it cannot substitute for UHT, C++ compilation or Automation execution.
14. Automated source editing must fail closed on source drift; ambiguous anchors are an error, never a reason to guess.

## Risks
Highest general risks remain UHT/UBT/compiler errors, unexecuted numerical Automation assertions, unmeasured coupled-contact convergence, unmeasured dynamic-unsprung behavior, incomplete real collision-manifold persistence, deferred topology-changing suspension fracture, incomplete hydraulic/ABS/fluid-boil behavior, and provisional real-world calibration.

Aero-specific risks:
- runtime defaults still mask the missing `UTAVehicleDefinition` property/call site until the closure is applied;
- canonical `PhysicsConfigHash` still excludes aero until that edit is applied;
- the asset-level test translation unit intentionally references `Definition->Aerodynamics`, so the tree is expected not to compile until closure is applied;
- source-level report/tests have not executed under UE 5.8;
- the call site must not hash aero twice: the adapter already advances the supplied hash on success;
- double COM subtraction or raw-coordinate hashing would corrupt reproducibility;
- two telemetry families intentionally remain, so ownership discipline is required to prevent field drift;
- the GitHub connector still has no line-patch primitive; the new local applicator removes that operational blocker once a checkout/runner executes it, but this automation run cannot execute repository-local commands through GitHub itself.

## Deliverables completed this session
- `scripts/apply_aero_asset_closure.py`;
- safe/idempotent/fail-closed source-edit contract;
- refreshed active checkpoint.

## Exact continuation point
Resume with the **canonical vehicle-definition source edit**:
1. in a patch-capable/local checkout run `python scripts/apply_aero_asset_closure.py`;
2. immediately run `python scripts/verify_aero_asset_closure.py` and require PASS;
3. inspect the resulting diff and commit the two canonical source edits only if the gate passes;
4. run/compile the four `TorqueAtlas.Vehicle.Definition.Aerodynamics.*` asset regressions and existing effective-hash coordinate-invariance regression;
5. inspect includes/module dependencies and ensure UHT can reflect the new Blueprint property;
6. run UE 5.8 UHT/UBT and `Automation RunTest TorqueAtlas.` when an engine environment is available;
7. only after source closure passes, begin the next major Proof-of-Physics subsystem audit.

After aero asset closure is applied, keep the telemetry split defined by ADR 41 until executable verification. Do not start map-based aero, active aero or another large physics subsystem before the canonical asset property/call site and asset tests are source-level closed.

## Checkpoint rule
Update this file before ending every substantial work session and before switching to a new major subsystem.
