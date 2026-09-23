# Active Development Checkpoint

Updated: 2026-09-23

## Completed / documented
- overall game vision and product pillars;
- business direction;
- open-world and real-geographic-data strategy;
- Unreal/custom-C++ architecture;
- TA-P01 prototype vehicle direction;
- suspension/powertrain architecture;
- tire solver numerical v0.1 baseline;
- surface/wetness model baseline;
- continuous per-wheel aquaplaning v0.1;
- steering/FFB coupling v0.2 direction;
- structural node/constraint architecture;
- structural solver v0.2 integration/compliance/plasticity/fracture plan;
- structural/mechanical damage dependency concept;
- physics tick ordering and performance targets;
- telemetry/debug and regression strategy;
- crash laboratory concept;
- GeoForge normalized data schema v0.1;
- fictional brand pool (60/100 maximum);
- roadmap and MVP;
- GitHub repository baseline, changelog and ADR log.

## Important decisions from latest session
1. Prototype tire backend uses a brush-inspired semi-empirical model behind a replaceable API.
2. Aquaplaning is continuous, local and per wheel; water also creates displacement drag/yaw disturbances.
3. Structural solver prototype uses iterative XPBD-style compliance with semi-implicit integration and adaptive crash quality.
4. GeoForge converts all source datasets into a stable normalized intermediate schema before Unreal import.

## Assumptions still provisional
- all tire calibration coefficients;
- dry/wet surface multipliers;
- solver iteration counts and frequency;
- physics performance budgets;
- aquaplaning calibration;
- launch/world/team/budget scale estimates.

These must remain tunable and must not be represented as measured truth.

## Risks currently highest
1. Structural deformation stability/performance at high stiffness and crash speed.
2. Coupling structural deformation to suspension/drivetrain without solver instability.
3. High-speed multiplayer correction of damaged vehicles.
4. Geo-data cleanup cost, especially intersections/bridges/tunnels.
5. Tire calibration without proprietary test datasets.

## Immediate next work — no user input required
1. Define `TA_VehicleDefinition` and simulation-state schemas in implementation-level detail.
2. Define powertrain solver v0.1: engine inertia/torque, clutch slip/thermal, gearbox, differential and driveline compliance.
3. Define suspension kinematics v0.1 and damaged-pickup coupling.
4. Define Proof-of-Physics proving-ground test matrix and acceptance tolerances.
5. Prepare Unreal/C++ repository skeleton specification (`Source`, plugin modules, tests, coding/units conventions).
6. Only after those specs are coherent, begin actual code skeleton files.

## Exact continuation point
Start with **Vehicle Data Schema + Powertrain Solver v0.1**. Do not expand the world or brand roster in the next session unless core technical work becomes blocked.

## Checkpoint rule
Update this file whenever a work session ends, before starting a new major subsystem.
