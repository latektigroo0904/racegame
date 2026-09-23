# Active Development Checkpoint

Updated: 2026-09-23

## Current phase
**Proof-of-Physics v2: aerodynamic authoring/compiler boundary complete in isolation; canonical asset/hash/Step integration is next. UE 5.8 executable verification remains the external acceptance gate.**

The canonical repository is `latektigroo0904/racegame`. Current content/runtime versions remain SchemaVersion 2, PhysicsVersion 2 and DamageModelVersion 2.

## Established source-level foundation
The repository contains four-wheel compliant contact; front shared-rack double wishbone; rear five-link geometry; tire force/thermal/wear/radial compliance; engine/clutch/gearbox/final-drive/differential dynamics; 6-DOF chassis; structural crash deformation and suspension pickup coupling; typed radiator, steering, hub, suspension, anti-roll, electrical and fuel damage; brake thermal/fade/wear; telemetry/regression reporting; vehicle calibration authoring; isolated dynamic-unsprung experimentation; and UE 5.8 verification runners.

The project remains **UE-build-unverified** until UnrealHeaderTool, UnrealBuildTool/C++ compilation, Editor module loading and `Automation RunTest TorqueAtlas.` complete successfully under Unreal Engine 5.8.

## Canonical driving path
`UTAVehicleDefinition → validation/FTAVehicleCompiledConfig → controls/environment → powertrain → wheel dynamics → suspension/contact → tire forces → shared chassis force accumulator → 6-DOF integration`.

Quasi-static compliant contact remains production-canonical. Dynamic unsprung remains experimental pending executable stability, energy and CPU evidence.

## Aerodynamics status
Established before this session:
- deterministic `TAAerodynamics` solver;
- `FTAVehicleRuntimeConfig::Aerodynamics` ownership;
- transient `FTAVehicleStepInput::AerodynamicsEnvironment` ownership;
- exact applied result retained in `FTAVehicleStepOutput::Aerodynamics`;
- `TAVehicleAerodynamicsBridge` as the single adapter into the shared chassis force accumulator;
- low-level and bridge regressions.

Completed this session:
- added Blueprint-authorable `FTAAerodynamicsDefinition` in an isolated header;
- added finite/range validation;
- added explicit vehicle-origin-local → COM-local compilation for the aero resultant application point;
- added `HashRuntimeConfig` covering effective area, Cd, Cl and all three COM-local lever-arm components;
- added Automation regressions for coordinate conversion, hash sensitivity and invalid calibration;
- documented the ownership/coordinate contract in `docs/35-AERO-AUTHORING-COMPILATION-V01.md`.

Important invariant: **no arcade speed-dependent tire-grip multiplier**. Aero grip gain must emerge from physical force application, chassis attitude/load transfer and changed tire normal loads.

## Decisions and assumptions
1. Vehicle asset authoring uses vehicle-origin-local coordinates, matching suspension/structure authoring.
2. Runtime aero application point is COM-local; compilation subtracts the authored center of mass exactly once.
3. Physics hashing uses effective compiled aero values rather than the authored coordinate representation.
4. Air density and wind remain per-step environment state, not vehicle calibration.
5. Negative lift coefficient means downforce under the existing solver convention.
6. The single-resultant aero model remains Proof-of-Physics scope; front/rear map-based balance and active aero are deferred until executable baseline evidence exists.

## Remaining aero work
1. include `TAAerodynamicsDefinition.h` from `TAVehicleDefinition.h` and add `UPROPERTY FTAAerodynamicsDefinition Aerodynamics`;
2. invoke authored aero validation inside `BuildCompiledConfig`;
3. compile into `VehicleRuntime.Aerodynamics` using the vehicle COM;
4. fold `TAAerodynamicsDefinition::HashRuntimeConfig` into canonical `PhysicsConfigHash`;
5. invoke `TAVehicleAerodynamicsBridge::AddToChassis` after wheel/suspension force accumulation and immediately before `TAChassisDynamics::Integrate`;
6. add full-step speed → aero force → chassis response regression;
7. expose aero output through telemetry/regression reporting;
8. require source-sanity green and ultimately UE 5.8 executable verification.

## Risks
Highest general risks remain UHT/UBT/compiler errors, unexecuted numerical Automation assertions, unmeasured coupled-contact convergence, unmeasured dynamic-unsprung behavior, incomplete real collision-manifold persistence, deferred topology-changing suspension fracture, incomplete hydraulic/ABS/fluid-boil behavior, and provisional real-world calibration.

Aero-specific risks:
- standalone authored definition exists but is not yet a property of `UTAVehicleDefinition`, so production assets still compile runtime defaults;
- canonical `PhysicsConfigHash` does not yet include the aero helper contribution;
- `TAVehicleSimulation::Step` does not yet call the bridge, so no claim of full causal aero integration is allowed;
- a single resultant cannot represent front/rear aero balance over large attitude changes;
- telemetry must report the exact applied result, not a separately recalculated estimate.

## Deliverables completed this session
- `TAAerodynamicsDefinition.h/.cpp`;
- aero authoring validation and COM-local compiler;
- effective-runtime aero hash helper;
- `TAAerodynamicsDefinitionTests.cpp`;
- `docs/35-AERO-AUTHORING-COMPILATION-V01.md`;
- refreshed active checkpoint.

## Exact continuation point
Resume with **canonical aero call-site integration**:
1. wire `FTAAerodynamicsDefinition` into `UTAVehicleDefinition`;
2. validation + compile into `VehicleRuntime.Aerodynamics`;
3. include effective aero values in `PhysicsConfigHash`;
4. call `TAVehicleAerodynamicsBridge::AddToChassis` immediately before chassis integration;
5. add full `TAVehicleSimulation::Step` regression proving drag/downforce/moment affect chassis state;
6. add telemetry channels sourced only from `FTAVehicleStepOutput::Aerodynamics`;
7. run/extend source-sanity checks.

Do not claim UE build success until the verification harness actually runs against UE 5.8.

## Checkpoint rule
Update this file before ending every substantial work session and before switching to a new major subsystem.
