# Active Development Checkpoint

Updated: 2026-09-23

## Current phase
**Proof-of-Physics v2: aerodynamic ownership/runtime bridge integration, with UE 5.8 executable verification still the external acceptance gate.**

The canonical repository is `latektigroo0904/racegame`. Current content/runtime versions remain SchemaVersion 2, PhysicsVersion 2 and DamageModelVersion 2.

## Established source-level foundation
The repository contains four-wheel compliant contact; front shared-rack double wishbone; rear five-link geometry; tire force/thermal/wear/radial compliance; engine/clutch/gearbox/final-drive/differential dynamics; 6-DOF chassis; structural crash deformation and suspension pickup coupling; typed radiator, steering, hub, suspension, anti-roll, electrical and fuel damage; brake thermal/fade/wear; telemetry/regression reporting; vehicle calibration authoring; isolated dynamic-unsprung experimentation; and UE 5.8 verification runners.

The project remains **UE-build-unverified** until UnrealHeaderTool, UnrealBuildTool/C++ compilation, Editor module loading and `Automation RunTest TorqueAtlas.` complete successfully under Unreal Engine 5.8.

## Canonical driving path
`UTAVehicleDefinition → validation/FTAVehicleCompiledConfig → controls/environment → powertrain → wheel dynamics → suspension/contact → tire forces → shared chassis force accumulator → 6-DOF integration`.

Quasi-static compliant contact remains production-canonical. Dynamic unsprung remains experimental pending executable stability, energy and CPU evidence.

## Aerodynamics status
The low-level deterministic aero solver remains established in `TAAerodynamics.h/.cpp`.

This session advanced ownership/runtime integration:
- `FTAVehicleRuntimeConfig` now owns `FTAAerodynamicsConfig`.
- `FTAVehicleStepInput` now owns transient `FTAAerodynamicsEnvironment`; wind and air density are therefore environment state, not vehicle calibration.
- `FTAVehicleStepOutput` now exposes `FTAAerodynamicsOutput` for telemetry/regression consumption.
- `TAVehicleAerodynamicsBridge` is the single adapter from vehicle-step data to `TAAerodynamics::AddToChassis`.
- bridge regressions cover force propagation into the shared chassis accumulator and headwind sensitivity.

Important invariant: **no arcade speed-dependent tire-grip multiplier**. Aero grip gain must emerge from physical force application, chassis attitude/load transfer and changed tire normal loads.

## Decisions and assumptions
1. Vehicle-owned aero coefficients are immutable runtime calibration for a compiled vehicle.
2. Air density and wind are per-step environment inputs because weather/altitude/world systems may vary them independently of vehicle content.
3. Aero output is retained on the vehicle-step output rather than recomputed by telemetry; this preserves one source of truth for the exact force applied.
4. The bridge accepts the same `FTAChassisForceAccumulator` used by tire/suspension forces. No second integration path is permitted.
5. Default aero runtime values remain the low-level solver defaults until authored content is compiled into them.

## Open aero work
The ownership boundary is now explicit, but canonical integration is not complete. Remaining work:
1. add `FTAAerodynamicsDefinition` to `UTAVehicleDefinition`;
2. compile area/Cd/Cl/application point into `FTAVehicleRuntimeConfig::Aerodynamics`;
3. validate finite/physical authored ranges;
4. include every effective aero field in `PhysicsConfigHash`;
5. invoke `TAVehicleAerodynamicsBridge::AddToChassis` inside `TAVehicleSimulation::Step` after wheel/suspension force accumulation and before `TAChassisDynamics::Integrate`;
6. add compile/hash and full-step regressions;
7. expose aero channels through telemetry/regression reporting;
8. require source-sanity green.

## Risks
Highest general risks remain UHT/UBT/compiler errors, unexecuted numerical Automation assertions, unmeasured coupled-contact convergence, unmeasured dynamic-unsprung behavior, incomplete real collision-manifold persistence, deferred topology-changing suspension fracture, incomplete hydraulic/ABS/fluid-boil behavior, and provisional real-world calibration.

Aero-specific risks:
- a single resultant application point cannot represent front/rear aero balance over large attitude changes;
- authoring the application point in vehicle-origin coordinates but consuming it as COM-local would create a silent pitching-moment error, so compilation must explicitly subtract authored COM;
- runtime bridge types are now source-level connected but `TAVehicleSimulation::Step` has not yet called the bridge, so no claim of full causal integration is allowed yet;
- telemetry must report the exact applied result, not a separately recalculated estimate.

## Deliverables completed this session
- vehicle runtime aero config ownership;
- per-step aero environment ownership;
- per-step aero output ownership;
- `TAVehicleAerodynamicsBridge.h/.cpp`;
- bridge Automation regressions;
- updated integration checkpoint and risk record.

## Exact continuation point
Resume with **authored aero compilation + canonical Step wiring**:
1. define `FTAAerodynamicsDefinition` with reference area, Cd, Cl and vehicle-local application point;
2. add it to `UTAVehicleDefinition`;
3. validate and compile application point to COM-local coordinates;
4. hash all four effective aero calibration fields;
5. wire `TAVehicleAerodynamicsBridge::AddToChassis` immediately before chassis integration;
6. add a full `TAVehicleSimulation::Step` regression proving speed → aero force → chassis acceleration/downforce moment;
7. extend telemetry channels and source-sanity checks.

Do not claim UE build success until the verification harness actually runs against UE 5.8.

## Checkpoint rule
Update this file before ending every substantial work session and before switching to a new major subsystem.
