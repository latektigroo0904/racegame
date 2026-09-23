# Active Development Checkpoint

Updated: 2026-09-23

## Completed / documented
- overall game vision;
- product pillars;
- business direction;
- open-world design;
- real geographic-data strategy;
- engine recommendation;
- modular C++ architecture;
- prototype vehicle TA-P01;
- suspension/powertrain direction;
- tire-system interface and first mathematical outline;
- structural node/constraint architecture;
- mechanical damage dependency concept;
- physics tick ordering;
- telemetry/debug strategy;
- crash laboratory concept;
- validation/regression philosophy;
- fictional brand pool (60/100 maximum);
- roadmap and MVP;
- GitHub repository baseline.

## Immediate next work
1. Finish Tire Solver v0.1 numerically:
   - select initial force-curve family;
   - define reference street tire;
   - combined slip;
   - load sensitivity;
   - thermal and pressure calibration.
2. Finish Surface Contact Model:
   - dry/wet coefficients;
   - roughness;
   - loose surfaces;
   - standing water.
3. Finish Aquaplaning v0.1:
   - water depth;
   - tread;
   - pressure;
   - speed;
   - load.
4. Steering/FFB v0.2.
5. Structural Solver v0.2:
   - integration method;
   - solver iterations;
   - plasticity;
   - fracture;
   - performance budget.
6. GeoForge data schema v0.1.
7. Create code skeleton after specs are sufficiently stable.

## No user input currently required
The above items can be advanced using safe engineering defaults. Values remain marked provisional until test data/calibration exists.

## Next checkpoint condition
Update this file whenever a work session ends, before starting a new major subsystem.
