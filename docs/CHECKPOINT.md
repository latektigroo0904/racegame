# Active Development Checkpoint

Updated: 2026-09-23

## Current phase
**Proof-of-Physics v2: aerodynamic subsystem pieces are established and the exact canonical closure contract is now frozen. Implementation of asset/hash/full-step wiring is the immediate next code task. UE 5.8 executable verification remains the external acceptance gate.**

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
- exact applied-result aero channels in `FTATelemetrySample`;
- `TAAeroRegressionMetrics` scalar extraction for air speed, dynamic pressure, force magnitude, drag, vertical force/downforce and pitch torque;
- Automation coverage for low-level aero, bridge, authoring/compiler helper, telemetry propagation and scalar regression projection;
- `docs/38-AERO-CANONICAL-CLOSURE-CONTRACT-V01.md`, freezing ownership, compile order, simulation order, regressions and definition-of-done.

Important invariant: **no arcade speed-dependent tire-grip multiplier**. Aero grip gain must emerge from physical force application, chassis attitude/load transfer and changed tire normal loads.

## Work completed this session
1. Re-audited the canonical repository against the prior checkpoint rather than assuming the documented state.
2. Confirmed `UTAVehicleDefinition` still has no authored `Aerodynamics` property.
3. Confirmed `BuildCompiledConfig` validates/compiles tire, wheel, powertrain, cooling, suspension and structure but still does not validate/compile/hash authored aero.
4. Confirmed `TAVehicleSimulation::Step` adds tire, suspension and aligning loads and then calls `TAChassisDynamics::Integrate` without an aero bridge call.
5. Confirmed runtime/environment/output ownership already exists and the bridge delegates directly to `TAAerodynamics::AddToChassis`.
6. Identified a masking risk: current default `FTAAerodynamicsConfig` values numerically match default authored `FTAAerodynamicsDefinition` values, so a default-only test could falsely suggest asset compilation is wired.
7. Added `docs/38-AERO-CANONICAL-CLOSURE-CONTRACT-V01.md` with the exact implementation sequence and non-default regression requirements.

## Decisions and assumptions
1. Vehicle asset authoring uses vehicle-origin-local coordinates, matching suspension/structure authoring.
2. Runtime aero application point is COM-local; compilation subtracts the authored center of mass exactly once.
3. Physics hashing uses effective compiled aero values, not raw authored coordinates.
4. Air density and wind remain per-step environment state, not vehicle calibration.
5. Negative lift coefficient means downforce under the current solver convention.
6. Telemetry and regression reporting consume the exact applied step result and never call the aero solver independently.
7. Aero is evaluated from the pre-integration chassis state of the current fixed step and is added to the same accumulator before the single chassis integration.
8. Full-step regressions must use deliberately non-default aero values where they are intended to prove authoring/compile ownership.
9. The single-resultant aero model remains Proof-of-Physics scope; front/rear map-based balance and active aero stay deferred until executable baseline evidence exists.

## Remaining aero work
1. include `TAAerodynamicsDefinition.h` from `TAVehicleDefinition.h` and add `UPROPERTY FTAAerodynamicsDefinition Aerodynamics`;
2. invoke authored aero validation inside `BuildCompiledConfig` and emit a vehicle validation error on failure;
3. compile into `VehicleRuntime.Aerodynamics` using the vehicle COM;
4. fold `TAAerodynamicsDefinition::HashRuntimeConfig` into canonical `PhysicsConfigHash` exactly once;
5. invoke `TAVehicleAerodynamicsBridge::AddToChassis` after wheel/suspension force accumulation and immediately before `TAChassisDynamics::Integrate`;
6. add asset/hash regressions using non-default Cd/Cl/application point and non-zero COM;
7. add full-step zero-air/speed/headwind/downforce/off-COM-pitch regressions;
8. wire `TAAeroRegressionMetrics` into the machine-readable scenario report/envelope layer;
9. require source-sanity green and ultimately UE 5.8 executable verification.

## Risks
Highest general risks remain UHT/UBT/compiler errors, unexecuted numerical Automation assertions, unmeasured coupled-contact convergence, unmeasured dynamic-unsprung behavior, incomplete real collision-manifold persistence, deferred topology-changing suspension fracture, incomplete hydraulic/ABS/fluid-boil behavior, and provisional real-world calibration.

Aero-specific risks:
- runtime defaults currently mask the missing asset compile path when only default values are inspected;
- canonical `PhysicsConfigHash` still does not include aero;
- ordinary full-step aero output remains zero because the bridge is not called by `Step`;
- a coordinate-space error can preserve plausible drag while corrupting pitch moment;
- placing the bridge after chassis integration would create a one-step lag;
- a single resultant cannot represent front/rear aero balance over large attitude changes;
- scalar metric extraction exists but is not yet serialized by the machine-readable regression report.

## Deliverables completed this session
- repository-level audit of the exact authored/runtime/step integration boundaries;
- `docs/38-AERO-CANONICAL-CLOSURE-CONTRACT-V01.md`;
- refreshed active checkpoint with a stricter non-default regression requirement and pre-integration ordering invariant.

## Exact continuation point
Resume with **implementation of the frozen canonical aero closure contract**:
1. `UTAVehicleDefinition.Aerodynamics` property;
2. validation + COM-local compilation into `VehicleRuntime.Aerodynamics`;
3. effective aero contribution to `PhysicsConfigHash`;
4. `TAVehicleAerodynamicsBridge::AddToChassis` immediately before chassis integration;
5. non-default asset/hash regressions;
6. full `TAVehicleSimulation::Step` regressions proving drag/downforce/moment alter chassis state;
7. machine-readable aero regression metrics;
8. source-sanity, then UE 5.8 verification when the engine environment is available.

Do not start a new major physics subsystem until items 1-6 are source-level closed. Do not claim UE build success until the verification harness actually runs against UE 5.8.

## Checkpoint rule
Update this file before ending every substantial work session and before switching to a new major subsystem.
