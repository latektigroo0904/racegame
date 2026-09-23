# Active Development Checkpoint

Updated: 2026-09-23

## Current phase
**Proof-of-Physics v2: canonical aerodynamic force application is wired into the ordinary vehicle step. Asset-side closure is now staged behind a transactional compiler adapter; the remaining primary edit is UTAVehicleDefinition ownership/call-site integration. UE 5.8 executable verification remains the external acceptance gate.**

Canonical repository: `latektigroo0904/racegame`. Current content/runtime versions remain SchemaVersion 2, PhysicsVersion 2 and DamageModelVersion 2.

## Established source-level foundation
The repository contains four-wheel compliant contact; front shared-rack double wishbone; rear five-link geometry; tire force/thermal/wear/radial compliance; engine/clutch/gearbox/final-drive/differential dynamics; 6-DOF chassis; structural crash deformation and suspension pickup coupling; typed functional damage; brake thermal/fade/wear; telemetry/regression reporting; vehicle calibration authoring; isolated dynamic-unsprung experimentation; aerodynamic solver/bridge/authoring definition; and UE 5.8 verification runners.

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
- new `TAVehicleAerodynamicsAssetCompiler::CompileValidatedAndHash` adapter that transactionally validates, compiles and hashes effective runtime aero;
- new asset-compiler regressions using the frozen non-default values and non-zero COM.

Important invariant: **no arcade speed-dependent tire-grip multiplier**. Aero grip gain must emerge from physical force application, chassis attitude/load transfer and changed tire normal loads.

## Work completed this session
1. Re-audited the active checkpoint and current repository tree before editing.
2. Confirmed `UTAVehicleDefinition` still has no authored `Aerodynamics` property and the canonical `BuildCompiledConfig` hash tail still excludes aero.
3. Added `TAVehicleAerodynamicsAssetCompiler.h/.cpp` as a small canonical asset-side adapter around the existing definition validator/compiler/effective-runtime hash helper.
4. Made the adapter transactional: invalid authoring returns false without changing the caller's physics hash or output runtime config.
5. Added `TorqueAtlas.Vehicle.Aerodynamics.AssetCompiler.NonDefaultCompileAndHash` using Area=2.37 m^2, Cd=0.287, Cl=-0.219, authored point `(0.46,-0.04,0.52)` and COM `(0.14,-0.01,0.19)`. The expected effective point is `(0.32,-0.03,0.33)` m.
6. Added `TorqueAtlas.Vehicle.Aerodynamics.AssetCompiler.InvalidIsTransactional` to guard failure-path mutation.
7. Kept the canonical asset call-site edit deferred rather than partially modifying the large vehicle-definition implementation without closing validation, compilation and hashing together.

## Decisions and assumptions
1. Vehicle asset authoring uses vehicle-origin-local coordinates; runtime aero application point is COM-local.
2. COM subtraction happens exactly once during compilation.
3. Physics hashing follows effective compiled aero values, never raw authored coordinates.
4. Air density and wind remain transient environment state and are not asset-hashed.
5. Negative lift coefficient means downforce under the current solver convention.
6. Telemetry/regression consume applied step output and never independently recompute aero.
7. Asset integration regressions use deliberately non-default values and non-zero COM to prevent default-value masking.
8. The new asset compiler adapter is the intended single call-site for `UTAVehicleDefinition::BuildCompiledConfig`; direct duplicated Validate/Compile/Hash logic should not be added there.
9. Failed asset aero compilation must be transactional so invalid content cannot leave a half-mutated runtime/hash state.
10. Single-resultant aero remains Proof-of-Physics scope; map-based front/rear balance and active aero remain deferred pending executable evidence.

## Remaining aero work
1. include `TAAerodynamicsDefinition.h` from `TAVehicleDefinition.h` and add `UPROPERTY FTAAerodynamicsDefinition Aerodynamics`;
2. in `BuildCompiledConfig`, emit `Vehicle.InvalidAerodynamics` when authored aero is invalid;
3. invoke `TAVehicleAerodynamicsAssetCompiler::CompileValidatedAndHash` at the canonical point after COM is known and before final `PhysicsConfigHash` assignment, without hashing aero a second time;
4. assign the successful compiled output to `VehicleRuntime.Aerodynamics`;
5. add/extend `TAVehicleDefinitionTests.cpp` with asset-level success, invalid-validation-code and hash-sensitivity assertions;
6. wire `TAAeroRegressionMetrics` into the machine-readable scenario report/envelope layer;
7. run source-sanity and ultimately UE 5.8 executable verification.

## Risks
Highest general risks remain UHT/UBT/compiler errors, unexecuted numerical Automation assertions, unmeasured coupled-contact convergence, unmeasured dynamic-unsprung behavior, incomplete real collision-manifold persistence, deferred topology-changing suspension fracture, incomplete hydraulic/ABS/fluid-boil behavior, and provisional real-world calibration.

Aero-specific risks:
- runtime defaults still mask the missing `UTAVehicleDefinition` property/call-site until canonical integration is completed;
- canonical `PhysicsConfigHash` still excludes aero at the vehicle-definition call site;
- the newly added adapter/tests are source-level only and have not executed under UE 5.8;
- a future call site must not hash aero twice: the adapter already advances the supplied hash on success;
- double COM subtraction or raw-coordinate hashing would corrupt reproducibility; existing helper and adapter regressions are intended to catch both classes of error;
- scalar metric extraction exists but is not yet serialized by the machine-readable regression report.

## Deliverables completed this session
- `Public/TAVehicleAerodynamicsAssetCompiler.h`;
- `Private/TAVehicleAerodynamicsAssetCompiler.cpp`;
- `Private/Tests/TAVehicleAerodynamicsAssetCompilerTests.cpp`;
- transactional failure contract for asset aero compilation;
- refreshed active checkpoint.

## Exact continuation point
Resume with the **small canonical vehicle-definition call-site closure**:
1. add `FTAAerodynamicsDefinition Aerodynamics` to `UTAVehicleDefinition`;
2. add `Vehicle.InvalidAerodynamics` validation;
3. after COM is established, call `TAVehicleAerodynamicsAssetCompiler::CompileValidatedAndHash` into `VehicleRuntime.Aerodynamics` and ensure its hash contribution occurs exactly once;
4. extend `TAVehicleDefinitionTests.cpp` to prove non-default propagation, invalid validation code and physics-hash sensitivity;
5. then wire machine-readable aero regression metrics;
6. run source-sanity, then UE 5.8 verification when an engine environment is available.

Do not start a new major physics subsystem until the canonical vehicle-definition aero property/call-site and tests are source-level closed. Do not claim UE build success until the verification harness actually runs against UE 5.8.

## Checkpoint rule
Update this file before ending every substantial work session and before switching to a new major subsystem.
