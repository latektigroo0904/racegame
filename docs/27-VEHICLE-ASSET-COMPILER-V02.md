# Vehicle Asset Compiler v0.2

Status: implemented prototype baseline
Updated: 2026-09-23

## 1. Purpose
The physics runtime must not depend on mutable Unreal DataAssets.

Authoring flow:

```
UTAVehicleDefinition
→ validation
→ SI / COM-relative compilation
→ FTAVehicleCompiledConfig
    ├─ FTAVehicleRuntimeConfig
    └─ FTAFourWheelRuntimeConfig
→ native fixed-step solvers
```

## 2. Authored prototype domains
The DataAsset now owns prototype authoring data for:
- vehicle dimensions;
- mass/inertia;
- center-of-mass datum offset;
- tire;
- wheel/brake;
- front double-wishbone geometry;
- front spring/damper/anti-roll;
- steering rack;
- rear five-link geometry;
- rear spring/damper/anti-roll;
- engine speed range/inertia;
- clutch capacity;
- gearbox ratios;
- final drive;
- prototype FWD/RWD choice.

## 3. Coordinate spaces
`FTAChassisState::PositionWorldM` is the physical center of mass.

Authored suspension hardpoints may use a vehicle-authoring datum.

Compilation converts every hardpoint:
```
P_COM_local =
P_authored - CenterOfMassMeters
```

The high-frequency suspension/chassis solvers therefore operate in COM-relative coordinates.

Current TA-P01 seed uses:
```
CenterOfMassMeters = (0,0,0)
```
because its existing hardpoints were already authored COM-relative.

## 4. Front authoring
Only canonical right-front geometry is authored.

Compiler/runtime mirrors left-front geometry across centerline.

Authored:
- upper inner A/B;
- lower inner A/B;
- upper/lower ball joints;
- tie rod inner/outer;
- damper chassis/lower-arm points;
- wheel center;
- travel limits;
- spring preload/rate;
- bump/rebound damping;
- anti-roll;
- rack travel/input shaping.

This avoids duplicated left/right source data.

## 5. Rear authoring
Canonical right-rear geometry contains five explicit links:
- chassis pickup;
- upright pickup.

Also:
- wheel center;
- damper chassis/upright points;
- travel limits;
- spring/damper;
- anti-roll.

Left rear is mirrored from the compiled right-side geometry.

## 6. Tire authoring
Current asset fields include:
- unloaded radius;
- reference load;
- reference pressure;
- dry peak friction seed;
- tread depth;
- radial stiffness;
- progressive radial stiffness;
- radial damping;
- maximum radial deflection.

More detailed compound/thermal curves remain native defaults until the tire asset layer is expanded.

## 7. Powertrain authoring
Prototype authoring currently includes:
- idle/redline/limiter;
- crank inertia;
- clutch torque capacity;
- forward ratios;
- reverse ratio;
- final drive;
- mechanical efficiency;
- front/rear driven axle selection.

The detailed torque map remains the native TA-P01 seed and should become authored data next.

## 8. Validation
Compilation rejects:
- missing definition ID;
- non-positive mass;
- invalid principal inertia;
- non-four-wheel vehicle while using the current four-wheel prototype runtime;
- invalid tire radial parameters;
- invalid suspension travel range;
- degenerate front geometry;
- degenerate rear five-link geometry;
- invalid engine RPM hierarchy;
- missing/non-positive gearbox ratios.

Future schema will generalize the current four-wheel gate.

## 9. Compiled configuration
`FTAVehicleCompiledConfig` now stores:
- identity/version;
- dimensions/mass;
- COM/inertia;
- wheel count;
- complete `FTAVehicleRuntimeConfig`;
- complete `FTAFourWheelRuntimeConfig`;
- stable physics hash.

Runtime initialization can therefore proceed without accessing the DataAsset again.

## 10. Physics hash
The hash now includes handling-critical prototype inputs including:
- mass/COM/inertia;
- dimensions;
- tire/radial properties;
- wheel inertia/brake capacity;
- front hardpoints;
- front spring/damper/anti-roll/steering;
- rear five-link hardpoints;
- rear spring/damper/anti-roll;
- engine speed/inertia;
- clutch capacity;
- gear ratios/final drive;
- drivetrain axle choice.

A spring-rate edit changes the hash.

## 11. Current deliberate limits
Not yet authored in the DataAsset:
- full engine torque map;
- turbo calibration;
- tire thermal masses/heat-transfer curves;
- detailed brake thermal system;
- structure node graph;
- damage bindings;
- radiator geometry;
- aero elements;
- individual left/right setup overrides;
- multi-axle >4-wheel vehicles.

These remain future compiler extensions.

## 12. Acceptance
v0.2 passes when:
- a default TA-P01 definition validates;
- front/rear geometry in the compiled config validates;
- four native wheel/tire configs exist;
- drivetrain selection compiles;
- handling-critical edits change the physics hash;
- high-frequency solvers need no mutable UObject access.
