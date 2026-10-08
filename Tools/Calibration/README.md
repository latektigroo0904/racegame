# Calibration Manifest Tooling

Versioned provenance contract for physics calibration.

Each manifest identifies:
- vehicle/content target;
- physics version;
- calibration groups;
- parameter paths, values and units;
- provenance class P0..P4;
- confidence;
- validation scenario IDs.

Production-like P4 groups must cite at least one validation scenario.
P3/P4 groups require a non-empty source reference.
