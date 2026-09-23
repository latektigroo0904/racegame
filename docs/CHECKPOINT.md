# Active Development Checkpoint

Updated: 2026-09-23

## Current phase
**Proof-of-Physics v2: aerodynamic force integration, with UE 5.8 executable verification still the external acceptance gate.**

The canonical repository is `latektigroo0904/racegame`. Current content/runtime versions remain SchemaVersion 2, PhysicsVersion 2 and DamageModelVersion 2.

## Established source-level foundation
The repository contains four-wheel compliant contact; front shared-rack double wishbone; rear five-link geometry; tire force/thermal/wear/radial compliance; engine/clutch/gearbox/final-drive/differential dynamics; 6-DOF chassis; structural crash deformation and suspension pickup coupling; typed radiator, steering, hub, suspension, anti-roll, electrical and fuel damage; brake thermal/fade/wear; telemetry/regression reporting; vehicle calibration authoring; isolated dynamic-unsprung experimentation; and UE 5.8 verification runners.

The project remains **UE-build-unverified** until UnrealHeaderTool, UnrealBuildTool/C++ compilation, Editor module loading and `Automation RunTest TorqueAtlas.` complete successfully under Unreal Engine 5.8.

## Canonical driving path
`UTAVehicleDefinition → validation/FTAVehicleCompiledConfig → controls → powertrain → wheel dynamics → suspension/contact → tire forces → chassis force accumulator → 6-DOF integration`.

Quasi-static compliant contact remains production-canonical. Dynamic unsprung remains experimental pending executable stability, energy and CPU evidence.

## Aerodynamics status
New source-level subsystem:
- `TAAerodynamics.h/.cpp` defines deterministic SI-unit aero config, environment, output and chassis-accumulator adapter.
- Relative airflow is `wind_world - vehicle_velocity_world`.
- Dynamic pressure is `0.5 * rho * |Vrel|^2`.
- Drag follows relative airflow.
- Lift acts on chassis-up; negative `Cl` produces physical downforce.
- The resultant is applied at a body-local point transformed to world space, so moments arise naturally from `r × F`.
- Automation coverage now targets zero relative speed, speed-squared scaling, headwind/tailwind, downforce and off-center pitch moment.
- Design/risks are recorded in `docs/34-AERODYNAMICS-V01.md`.

Important invariant: **no arcade speed-dependent tire-grip multiplier**. Any aero grip gain must emerge from force application, chassis load transfer and changed tire normal loads.

## Open aero work
The solver exists but is not yet wired into canonical vehicle authoring/runtime. Remaining work is:
1. authored aero definition;
2. compile to runtime config;
3. validation and `PhysicsConfigHash` ownership;
4. environment input on vehicle step;
5. force application into the same chassis accumulator before integration;
6. telemetry/regression channels;
7. source-sanity confirmation.

## Verification status and risks
GitHub source-sanity has previously caught real C++ source defects and has been green on the completed damage/brake paths, but that is not equivalent to an Unreal build. Highest risks remain UHT/UBT/compiler errors, unexecuted numerical Automation assertions, unmeasured coupled-contact convergence, unmeasured dynamic-unsprung behavior, incomplete real collision-manifold persistence, deferred topology-changing suspension fracture, incomplete hydraulic/ABS/fluid-boil behavior, and provisional real-world calibration. Aero adds the specific risk that one resultant point cannot represent front/rear balance over large attitude changes.

## Exact continuation point
Resume with **aero ownership and canonical runtime integration**:
1. add `FTAAerodynamicsDefinition` to `UTAVehicleDefinition` with area/Cd/Cl/application point;
2. compile it into `FTAVehicleRuntimeConfig`;
3. validate finite/physical ranges and include every effective aero field in `PhysicsConfigHash`;
4. extend `FTAVehicleStepInput` with environment air density/wind;
5. call `TAAerodynamics::AddToChassis` before `TAChassisDynamics::Integrate`;
6. expose aero output on `FTAVehicleStepOutput` and telemetry;
7. add compile/hash regression tests;
8. require source-sanity green.

Do not claim UE build success until the verification harness actually runs against UE 5.8.

## Checkpoint rule
Update this file before ending every substantial work session and before switching to a new major subsystem.
