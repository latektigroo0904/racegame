# Active Development Checkpoint

Updated: 2026-09-23

## Current phase
**Proof-of-Physics v2: canonical aerodynamic force application is now wired into the ordinary vehicle step and covered by full-step source regressions. Asset authoring/compile/hash ownership remains the immediate closure task. UE 5.8 executable verification remains the external acceptance gate.**

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
- `docs/38-AERO-CANONICAL-CLOSURE-CONTRACT-V01.md` freezing ownership and ordering.

Important invariant: **no arcade speed-dependent tire-grip multiplier**. Aero grip gain must emerge from physical force application, chassis attitude/load transfer and changed tire normal loads.

## Work completed this session
1. Re-audited the canonical repository and confirmed the prior checkpoint was accurate.
2. Wired `TAVehicleAerodynamicsBridge::AddToChassis` into `TAVehicleSimulation::Step` at the required pre-integration boundary.
3. Kept exactly one `TAChassisDynamics::Integrate` call per fixed step; aero therefore has no deliberate one-step lag.
4. Added `TAVehicleSimulationAerodynamicsTests.cpp` with non-default aero coefficients and off-COM application point.
5. Added full-step assertions that drag decelerates the chassis, downforce changes vertical velocity and an off-COM aero force creates pitch response.
6. Added zero-air-density and headwind comparison regressions.
7. Rechecked the remaining closure gap: `UTAVehicleDefinition` still does not own authored aero, so asset compilation and canonical physics hashing are still incomplete.

## Decisions and assumptions
1. Vehicle asset authoring uses vehicle-origin-local coordinates, matching suspension/structure authoring.
2. Runtime aero application point is COM-local; compilation subtracts the authored center of mass exactly once.
3. Physics hashing uses effective compiled aero values, not raw authored coordinates.
4. Air density and wind remain per-step environment state, not vehicle calibration.
5. Negative lift coefficient means downforce under the current solver convention.
6. Telemetry and regression reporting consume the exact applied step result and never call the aero solver independently.
7. Aero is evaluated from the pre-integration chassis state of the current fixed step and is added to the same accumulator before the single chassis integration.
8. Full-step regressions use deliberately non-default aero values where they prove runtime ownership.
9. The single-resultant aero model remains Proof-of-Physics scope; front/rear map-based balance and active aero stay deferred until executable baseline evidence exists.

## Remaining aero work
1. include `TAAerodynamicsDefinition.h` from `TAVehicleDefinition.h` and add `UPROPERTY FTAAerodynamicsDefinition Aerodynamics`;
2. invoke authored aero validation inside `BuildCompiledConfig` and emit `Vehicle.InvalidAerodynamics` on failure;
3. compile into `VehicleRuntime.Aerodynamics` using the vehicle COM;
4. fold `TAAerodynamicsDefinition::HashRuntimeConfig` into canonical `PhysicsConfigHash` exactly once;
5. add asset/hash regressions using non-default Cd/Cl/application point and non-zero COM;
6. wire `TAAeroRegressionMetrics` into the machine-readable scenario report/envelope layer;
7. run source-sanity and ultimately UE 5.8 executable verification.

## Risks
Highest general risks remain UHT/UBT/compiler errors, unexecuted numerical Automation assertions, unmeasured coupled-contact convergence, unmeasured dynamic-unsprung behavior, incomplete real collision-manifold persistence, deferred topology-changing suspension fracture, incomplete hydraulic/ABS/fluid-boil behavior, and provisional real-world calibration.

Aero-specific risks:
- runtime defaults still mask the missing asset compile path when only default values are inspected;
- canonical `PhysicsConfigHash` still does not include aero;
- source-level full-step tests have not executed under UE 5.8 yet;
- a coordinate-space error in the future asset wiring could preserve plausible drag while corrupting pitch moment;
- a single resultant cannot represent front/rear aero balance over large attitude changes;
- scalar metric extraction exists but is not yet serialized by the machine-readable regression report.

## Deliverables completed this session
- canonical pre-integration aero bridge call in `TAVehicleSimulation::Step`;
- `TAVehicleSimulationAerodynamicsTests.cpp` with three full-step regression cases;
- refreshed active checkpoint.

## Exact continuation point
Resume with **asset-side closure of the frozen canonical aero contract**:
1. `UTAVehicleDefinition.Aerodynamics` property;
2. validation + COM-local compilation into `VehicleRuntime.Aerodynamics`;
3. effective aero contribution to `PhysicsConfigHash`;
4. non-default asset/hash regressions, including non-zero COM;
5. machine-readable aero regression metrics;
6. source-sanity, then UE 5.8 verification when the engine environment is available.

Do not start a new major physics subsystem until items 1-4 are source-level closed. Do not claim UE build success until the verification harness actually runs against UE 5.8.

## Checkpoint rule
Update this file before ending every substantial work session and before switching to a new major subsystem.
