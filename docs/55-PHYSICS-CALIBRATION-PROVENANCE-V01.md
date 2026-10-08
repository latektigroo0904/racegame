# Physics Calibration & Provenance Contract v0.1

Updated: 2026-10-08
Status: canonical content/engineering policy

## 1. Purpose

Torque Atlas requires hundreds of physical calibration values.

Without provenance, realistic-looking numbers become untraceable magic constants.

Every handling-critical value must therefore have:
- owner;
- units;
- source/provenance class;
- confidence;
- version;
- applicable vehicle/component;
- validation evidence.

## 2. Provenance classes

### P0 — Placeholder
Engineering seed used to make code executable.
Not physically validated.

### P1 — Literature bounded
Derived from credible generic engineering literature or public datasets.
Not vehicle-specific.

### P2 — Reference-vehicle inferred
Derived from public dimensions, performance, mass, gearing or test data for a comparable vehicle class.

### P3 — Measured / licensed test data
Direct measurement, supplier data or licensed dataset.

### P4 — Validated production calibration
Parameter set validated against Torque Atlas proving-ground scenario traces and approved for production content.

## 3. Confidence

Each calibration group receives:
- low;
- medium;
- high.

Confidence is separate from provenance.

A precise published value for the wrong component can still be low-confidence for the target vehicle.

## 4. Required metadata

Conceptual record:

```
FTACalibrationMetadata
{
    CalibrationId
    Version
    ProvenanceClass
    Confidence
    SourceReference
    Notes
    Author
    Date
}
```

Runtime physics does not need string-heavy metadata in every hot struct.

Metadata belongs in authoring/content/tooling and compiled manifests.

## 5. Units

All authored physics values declare units.

Examples:
- kg;
- m;
- N;
- N/m;
- N*s/m;
- Pa;
- N*m;
- kg*m²;
- W/C;
- J/C;
- rad/s.

Dimensionless fields must explicitly state their convention.

## 6. No hidden calibration

Prohibited:
- car-specific values buried in solver code;
- undocumented global multipliers;
- special-case “make this car stable” branches;
- graphics/UI code modifying physics values;
- realism modes changing physical constants.

## 7. Calibration groups

Vehicle calibration is grouped by subsystem:
- mass/inertia;
- tire;
- suspension geometry;
- springs/dampers/stops;
- steering;
- brakes;
- engine;
- clutch;
- gearbox/final drive;
- differential;
- aero;
- cooling/fluids;
- electrical;
- structure/damage.

## 8. PhysicsConfigHash

The hash identifies effective compiled physics.

Any persistent field capable of changing:
- force;
- torque;
- energy;
- mass/inertia;
- geometry;
- damage consequence;
- controller behavior in canonical simulation

must be represented in configuration identity.

Transient environment/state does not belong in vehicle-definition hash.

## 9. Scenario baseline identity

A trusted regression baseline stores:
- physics hash;
- calibration manifest version;
- scenario version;
- engine/toolchain version;
- fixed timestep.

Changing calibration invalidates trust in an old baseline unless explicitly requalified.

## 10. Calibration workflow

1. start P0/P1 seed;
2. run analytical/unit tests;
3. capture executable trace;
4. compare against target evidence;
5. adjust only relevant physical parameters;
6. rerun;
7. record resulting provenance/confidence;
8. promote baseline;
9. freeze version.

## 11. Parameter fitting

Automated fitting is allowed when:
- objective function is explicit;
- fitted parameters have physical bounds;
- training scenarios are separate from validation scenarios;
- resulting values remain interpretable.

Do not fit dozens of unconstrained coefficients to one lap time.

## 12. Validation hierarchy

Prefer validation in this order:
1. direct component behavior;
2. subsystem behavior;
3. vehicle maneuver;
4. lap/event outcome.

Example:
spring rate should first match suspension/load response, not be tuned solely to improve a lap time.

## 13. Tire calibration

Separate targets:
- pure longitudinal;
- pure lateral;
- combined slip;
- load sensitivity;
- thermal;
- pressure;
- wear;
- wet/hydro;
- transient response.

One global tire grip scalar is prohibited as a production tuning shortcut.

## 14. Aero calibration

Separate:
- Cd;
- total Cl;
- front/rear balance;
- application points;
- later ride-height/pitch/yaw maps.

Top speed alone cannot uniquely calibrate Cd because powertrain losses also matter.

## 15. Powertrain calibration

Separate:
- crank torque;
- rotational inertia;
- friction;
- clutch capacity;
- gearbox efficiency;
- final drive;
- differential behavior.

0–100 time alone is not sufficient.

## 16. Damage calibration

Use:
- impact speed/mass;
- deformation;
- broken connections;
- mount displacement;
- fluid loss;
- resulting drivability.

Do not calibrate crash severity using a generic HP bar.

## 17. Content scaling

When adding many fictional cars:
- derive shared component families;
- reuse measured archetypes;
- vary physically meaningful geometry/mass/power/config;
- avoid hand-tuning every field from scratch.

## 18. Auditability

A debug vehicle should be able to report:
- definition ID;
- version triplet;
- physics hash;
- calibration manifest;
- subsystem backend IDs;
- active controller profile.

## 19. Change control

Physics-affecting content changes require:
- changelog entry;
- hash change;
- impacted regression list;
- baseline review.

Cosmetic-only changes must not change physics hash.

## 20. Definition of production calibration

A value is production-calibrated only when:
- provenance is known;
- units/convention are clear;
- relevant executable tests pass;
- no contradictory trusted scenario exists;
- calibration review is recorded.
