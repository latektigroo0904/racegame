# Active Development Checkpoint

Updated: 2026-09-23

## Current phase
**Proof-of-Physics v2: canonical aerodynamic force application is wired into the ordinary vehicle step. The remaining primary closure is asset authoring/compile/hash ownership; effective-runtime hash coordinate invariance is now explicitly regression-locked. UE 5.8 executable verification remains the external acceptance gate.**

Canonical repository: `latektigroo0904/racegame`. Current content/runtime versions remain SchemaVersion 2, PhysicsVersion 2 and DamageModelVersion 2.

## Established source-level foundation
The repository contains four-wheel compliant contact; front shared-rack double wishbone; rear five-link geometry; tire force/thermal/wear/radial compliance; engine/clutch/gearbox/final-drive/differential dynamics; 6-DOF chassis; structural crash deformation and suspension pickup coupling; typed radiator, steering, hub, suspension, anti-roll, electrical and fuel damage; brake thermal/fade/wear; telemetry/regression reporting; vehicle calibration authoring; isolated dynamic-unsprung experimentation; aerodynamic solver/bridge/authoring definition; and UE 5.8 verification runners.

The project remains **UE-build-unverified** until UnrealHeaderTool, UnrealBuildTool/C++ compilation, Editor module loading and `Automation RunTest TorqueAtlas.` complete successfully under Unreal Engine 5.8.

## Aerodynamics status
Established:
- deterministic `TAAerodynamics` solver;
- `FTAVehicleRuntimeConfig::Aerodynamics` ownership;
- transient `FTAVehicleStepInput::AerodynamicsEnvironment` ownership;
- exact result slot in `FTAVehicleStepOutput::Aerodynamics`;
- `TAVehicleAerodynamicsBridge` as the single adapter into the shared chassis force accumulator;
- Blueprint-authorable `FTAAerodynamicsDefinition` with validation, vehicle-origin-local -> COM-local compilation and effective-runtime hashing helper;
- canonical `TAVehicleSimulation::Step` call to `TAVehicleAerodynamicsBridge::AddToChassis` after wheel/suspension/alignment accumulation and immediately before the single chassis integration;
- exact applied-result aero channels in telemetry and scalar regression extraction;
- full-step source regressions for moving-air chassis response, zero-density neutrality and headwind amplification;
- effective aero hash regression proving equivalent authored origin/COM shifts preserve the same COM-local runtime hash contribution;
- `docs/38-AERO-CANONICAL-CLOSURE-CONTRACT-V01.md` and `docs/39-AERO-ASSET-CLOSURE-IMPLEMENTATION-NOTES-V01.md` freezing ownership, ordering and the exact remaining asset edits.

Important invariant: **no arcade speed-dependent tire-grip multiplier**. Aero grip gain must emerge from physical force application, chassis attitude/load transfer and changed tire normal loads.

## Work completed this session
1. Re-audited `UTAVehicleDefinition`, `BuildCompiledConfig`, the canonical hash tail and `FTAAerodynamicsDefinition` against the prior checkpoint.
2. Confirmed the remaining source gap is real: `UTAVehicleDefinition` still has no authored aero property, so runtime aero still falls back to struct defaults during asset compilation and canonical `PhysicsConfigHash` still excludes aero.
3. Confirmed the existing aero helper already implements the intended vehicle-origin-local -> COM-local conversion and hashes effective runtime values.
4. Added `TorqueAtlas.Vehicle.Aerodynamics.Definition.EffectiveHashCoordinateInvariant` regression. It compiles two different authored coordinate representations that describe the same physical COM-local aero point and requires identical effective hash contributions.
5. Added `docs/39-AERO-ASSET-CLOSURE-IMPLEMENTATION-NOTES-V01.md` with the exact five remaining canonical edits, fixed non-default test values, expected COM-local result, validation code and acceptance assertions.
6. Rechecked that no new major physics subsystem should start before asset compile/hash closure.

## Decisions and assumptions
1. Vehicle asset authoring uses vehicle-origin-local coordinates, matching suspension/structure authoring.
2. Runtime aero application point is COM-local; compilation subtracts the authored center of mass exactly once.
3. Physics hashing uses effective compiled aero values, not raw authored coordinates.
4. Equivalent simultaneous shifts of authored vehicle-origin point and COM must preserve the effective aero hash contribution.
5. Air density and wind remain per-step environment state, not vehicle calibration and not physics-hashed asset values.
6. Negative lift coefficient means downforce under the current solver convention.
7. Telemetry and regression reporting consume the exact applied step result and never call the aero solver independently.
8. Aero is evaluated from the pre-integration chassis state of the current fixed step and is added to the same accumulator before the single chassis integration.
9. Asset integration regressions must use deliberately non-default aero values and non-zero COM to prevent runtime-default masking.
10. The single-resultant aero model remains Proof-of-Physics scope; front/rear map-based balance and active aero stay deferred until executable baseline evidence exists.

## Remaining aero work
1. include `TAAerodynamicsDefinition.h` from `TAVehicleDefinition.h` and add `UPROPERTY FTAAerodynamicsDefinition Aerodynamics`;
2. invoke authored aero validation inside `BuildCompiledConfig` and emit `Vehicle.InvalidAerodynamics` on failure;
3. compile into `VehicleRuntime.Aerodynamics` using the vehicle COM;
4. fold `TAAerodynamicsDefinition::HashRuntimeConfig` into canonical `PhysicsConfigHash` exactly once;
5. add asset-level compile/hash regressions using the fixed non-default values and non-zero COM from doc 39;
6. wire `TAAeroRegressionMetrics` into the machine-readable scenario report/envelope layer;
7. run source-sanity and ultimately UE 5.8 executable verification.

## Risks
Highest general risks remain UHT/UBT/compiler errors, unexecuted numerical Automation assertions, unmeasured coupled-contact convergence, unmeasured dynamic-unsprung behavior, incomplete real collision-manifold persistence, deferred topology-changing suspension fracture, incomplete hydraulic/ABS/fluid-boil behavior, and provisional real-world calibration.

Aero-specific risks:
- runtime defaults still mask the missing asset compile path when only default values are inspected;
- canonical `PhysicsConfigHash` still does not include aero;
- source-level regressions have not executed under UE 5.8 yet;
- double COM subtraction or wrong coordinate ownership could preserve plausible drag while corrupting pitch moment;
- hashing raw authored coordinates would make physically equivalent coordinate representations spuriously different; the new helper-level regression now guards this invariant;
- a single resultant cannot represent front/rear aero balance over large attitude changes;
- scalar metric extraction exists but is not yet serialized by the machine-readable regression report.

## Deliverables completed this session
- effective-runtime aero hash coordinate-invariance Automation regression;
- `docs/39-AERO-ASSET-CLOSURE-IMPLEMENTATION-NOTES-V01.md`;
- refreshed active checkpoint with exact remaining implementation contract.

## Exact continuation point
Resume with **asset-side implementation of the frozen canonical aero contract**:
1. `UTAVehicleDefinition.Aerodynamics` property;
2. `Vehicle.InvalidAerodynamics` validation;
3. COM-local compilation into `VehicleRuntime.Aerodynamics`;
4. effective aero contribution to `PhysicsConfigHash`;
5. non-default asset/hash regressions, including non-zero COM and expected point `(0.32, -0.03, 0.33)` m;
6. machine-readable aero regression metrics;
7. source-sanity, then UE 5.8 verification when the engine environment is available.

Do not start a new major physics subsystem until items 1-5 are source-level closed. Do not claim UE build success until the verification harness actually runs against UE 5.8.

## Checkpoint rule
Update this file before ending every substantial work session and before switching to a new major subsystem.
