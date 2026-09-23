# Active Development Checkpoint

Updated: 2026-09-23

## Current phase
Technical R&D transitioning into first Unreal/C++ prototype foundation.

## Completed / documented
- overall game vision and product pillars;
- business direction;
- open-world and real-geographic-data strategy;
- Unreal/custom-C++ architecture;
- TA-P01 prototype direction;
- tire solver numerical v0.1;
- surface/wetness model baseline;
- continuous per-wheel aquaplaning v0.1;
- steering/FFB coupling direction;
- structural solver v0.2 design;
- GeoForge normalized data schema v0.1;
- vehicle data/runtime-state schema v0.1;
- powertrain solver v0.1 specification;
- suspension kinematics v0.1;
- Proof-of-Physics test matrix v0.1;
- Unreal/C++ repository skeleton specification;
- fictional brand pool (60/100 maximum);
- roadmap/MVP;
- decision log and changelog.

## Code now present
Root:
- `TorqueAtlas.uproject`;
- Unreal `.gitignore`;
- Game and Editor target files;
- `TorqueAtlas` game module.

Plugin:
`Plugins/TorqueVehicleSimulation`

Enabled runtime modules:
- `TA_Core`;
- `TA_Vehicle`;
- `TA_Powertrain`.

Implemented baseline code:
- simulation version and validation types;
- SI/Unreal conversion helpers;
- `UTAVehicleDefinition : UPrimaryDataAsset`;
- native `FTAVehicleCompiledConfig`;
- definition validation;
- stable baseline physics configuration hash;
- first engine rotational/friction helpers;
- first clutch torque-capacity helper;
- Unreal Automation smoke tests.

## Important decisions from this session
1. Vehicle definition, persistent owned instance and live simulation state are separate data domains.
2. Authored Unreal assets compile into native immutable runtime configuration before high-frequency physics.
3. Undamaged suspension may use compiled kinematic lookup; damaged geometry switches to runtime geometric solving.
4. Plugin modules are enabled incrementally only when their source/dependencies exist.

## Verification status
**Not yet compiled against a real Unreal Engine 5.8 installation.**

Therefore:
- source structure is implementation-oriented but build success is unverified;
- no claim is made that UHT/UBT/compiler validation has passed;
- first real UE build is the next mandatory integration gate once an Unreal-capable development environment is connected.

## Provisional assumptions
Still tunable:
- tire coefficients;
- surface multipliers;
- powertrain inertias/friction coefficients;
- clutch stiffness/capacity;
- suspension hardpoints;
- structural iteration counts/compliance;
- physics timestep/budgets;
- aquaplaning calibration;
- final performance targets.

## Highest risks
1. Structural crash stability/performance at high stiffness.
2. Suspension/drivetrain coupling after structural deformation.
3. Tire calibration without proprietary measurement datasets.
4. High-speed multiplayer reconciliation after damage.
5. GeoForge cleanup of complex intersections/bridges/tunnels.
6. First Unreal compile may reveal API/build-rule adjustments.

## Immediate next work — no user input required
1. Complete Powertrain v0.1 code:
   - engine torque map interface;
   - idle/stall state;
   - clutch thermal state;
   - gearbox ratios/state;
   - driveline torsional compliance;
   - open differential.
2. Add `TA_Tire` runtime module and implement first native force-solver API from docs/08.
3. Add suspension runtime configuration/state to `TA_Vehicle`.
4. Add `TA_Surface` runtime module.
5. Add deterministic test fixtures for powertrain + tire low-speed transition.
6. Add structural module skeleton only after the above APIs are coherent.
7. When UE 5.8 build access exists: generate project files, compile Development Editor, run tests, fix all UHT/UBT/compiler errors before advancing broad feature work.

## Exact continuation point
Resume at **Powertrain v0.1 code: gearbox + clutch thermal + driveline + open differential**, then create `TA_Tire`.

Do not expand world size or brand roster in the next session unless core technical work becomes blocked.

## Checkpoint rule
Update this file whenever a work session ends, before starting a new major subsystem.
