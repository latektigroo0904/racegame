# Active Development Checkpoint

Updated: 2026-09-23

## Current phase
**Proof-of-Physics v2: aerodynamic authoring/compiler pieces and telemetry ownership exist; canonical asset/hash/vehicle-step integration remains the immediate closure task. UE 5.8 executable verification remains the external acceptance gate.**

Canonical repository: `latektigroo0904/racegame`. Current content/runtime versions remain SchemaVersion 2, PhysicsVersion 2 and DamageModelVersion 2.

## Established source-level foundation
The repository contains four-wheel compliant contact; front shared-rack double wishbone; rear five-link geometry; tire force/thermal/wear/radial compliance; engine/clutch/gearbox/final-drive/differential dynamics; 6-DOF chassis; structural crash deformation and suspension pickup coupling; typed radiator, steering, hub, suspension, anti-roll, electrical and fuel damage; brake thermal/fade/wear; telemetry/regression reporting; vehicle calibration authoring; isolated dynamic-unsprung experimentation; aerodynamic solver/bridge/authoring definition; and UE 5.8 verification runners.

The project remains **UE-build-unverified** until UnrealHeaderTool, UnrealBuildTool/C++ compilation, Editor module loading and `Automation RunTest TorqueAtlas.` complete successfully under Unreal Engine 5.8.

## Canonical driving path
`UTAVehicleDefinition -> validation/FTAVehicleCompiledConfig -> controls/environment -> powertrain -> wheel dynamics -> suspension/contact -> tire forces -> shared chassis force accumulator -> 6-DOF integration`.

Quasi-static compliant contact remains production-canonical. Dynamic unsprung remains experimental pending executable stability, energy and CPU evidence.

## Aerodynamics status
Already established:
- deterministic `TAAerodynamics` solver;
- `FTAVehicleRuntimeConfig::Aerodynamics` ownership;
- transient `FTAVehicleStepInput::AerodynamicsEnvironment` ownership;
- exact result slot in `FTAVehicleStepOutput::Aerodynamics`;
- `TAVehicleAerodynamicsBridge` as the single adapter into the shared chassis force accumulator;
- Blueprint-authorable `FTAAerodynamicsDefinition`;
- finite/range validation;
- vehicle-origin-local -> COM-local application-point compilation;
- effective-runtime aero hash helper;
- low-level, bridge and authoring/compiler regressions.

Completed this session:
- audited the repository against the prior checkpoint and confirmed the remaining canonical integration gap is real;
- extended `FTATelemetrySample` with relative air speed, dynamic pressure, applied world force and applied world torque;
- made `TATelemetry::MakeSample` consume only `FTAVehicleStepOutput::Aerodynamics`, with no independent aero solve/reconstruction path;
- added `TorqueAtlas.Telemetry.Aerodynamics.CapturesAppliedStepOutput` regression;
- documented the telemetry ownership contract in `docs/36-AERO-TELEMETRY-INTEGRATION-V01.md`.

Important invariant: **no arcade speed-dependent tire-grip multiplier**. Aero grip gain must emerge from physical force application, chassis attitude/load transfer and changed tire normal loads.

## Decisions and assumptions
1. Vehicle asset authoring uses vehicle-origin-local coordinates, matching suspension/structure authoring.
2. Runtime aero application point is COM-local; compilation subtracts the authored center of mass exactly once.
3. Physics hashing must use effective compiled aero values.
4. Air density and wind remain per-step environment state, not vehicle calibration.
5. Negative lift coefficient means downforce under the current solver convention.
6. Telemetry reports the exact applied step result and never calls the aero solver independently.
7. Aero telemetry retains world-space force/torque vectors so wind/yaw cases remain observable.
8. The single-resultant aero model remains Proof-of-Physics scope; front/rear map-based balance and active aero stay deferred until executable baseline evidence exists.

## Remaining aero work
1. include `TAAerodynamicsDefinition.h` from `TAVehicleDefinition.h` and add `UPROPERTY FTAAerodynamicsDefinition Aerodynamics`;
2. invoke authored aero validation inside `BuildCompiledConfig`;
3. compile into `VehicleRuntime.Aerodynamics` using the vehicle COM;
4. fold `TAAerodynamicsDefinition::HashRuntimeConfig` into canonical `PhysicsConfigHash`;
5. invoke `TAVehicleAerodynamicsBridge::AddToChassis` after wheel/suspension force accumulation and immediately before `TAChassisDynamics::Integrate`;
6. add full-step speed/wind -> aero force -> chassis response regression;
7. extend machine-readable regression metrics/reporting with scalar aero channels;
8. require source-sanity green and ultimately UE 5.8 executable verification.

## Risks
Highest general risks remain UHT/UBT/compiler errors, unexecuted numerical Automation assertions, unmeasured coupled-contact convergence, unmeasured dynamic-unsprung behavior, incomplete real collision-manifold persistence, deferred topology-changing suspension fracture, incomplete hydraulic/ABS/fluid-boil behavior, and provisional real-world calibration.

Aero-specific risks:
- standalone authored definition is still not a property of `UTAVehicleDefinition`, so production assets compile runtime defaults;
- canonical `PhysicsConfigHash` still does not include the aero helper contribution;
- `TAVehicleSimulation::Step` still does not call the bridge, so ordinary full-step aero output remains zero and no claim of full causal integration is allowed;
- a single resultant cannot represent front/rear aero balance over large attitude changes;
- machine-readable regression reporting is primarily scalar and still needs an explicit aero metric policy.

## Deliverables completed this session
- aerodynamic channels in `FTATelemetrySample`;
- exact applied-result mapping in `TATelemetry::MakeSample`;
- aero telemetry Automation regression;
- `docs/36-AERO-TELEMETRY-INTEGRATION-V01.md`;
- refreshed active checkpoint.

## Exact continuation point
Resume with **canonical aero physics closure**:
1. wire `FTAAerodynamicsDefinition` into `UTAVehicleDefinition`;
2. validation + compile into `VehicleRuntime.Aerodynamics`;
3. include effective aero values in `PhysicsConfigHash`;
4. call `TAVehicleAerodynamicsBridge::AddToChassis` immediately before chassis integration;
5. add full `TAVehicleSimulation::Step` regression proving drag/downforce/moment affect chassis state;
6. add scalar aero metrics to regression reporting, sourced from the telemetry sample;
7. run/extend source-sanity checks.

Do not claim UE build success until the verification harness actually runs against UE 5.8.

## Checkpoint rule
Update this file before ending every substantial work session and before switching to a new major subsystem.
