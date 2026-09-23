# Active Development Checkpoint

Updated: 2026-09-23

## Current phase
**Four-wheel compliant vehicle runtime + first collision/structure coupling.**

The TA-P01 proving-ground path now resolves all four wheel contacts from authored/compiled physics, includes finite tire radial compliance, solves axle anti-roll inside the tire/suspension equilibrium, and can apply collision impulses to chassis motion while injecting a momentum-neutral internal deformation mode into structural nodes.

The source remains **build-unverified** until Unreal Engine 5.8 UHT/UBT/C++ compilation and Automation execution are actually run.

## Canonical repository
`latektigroo0904/racegame`

## Runtime modules
```
TA_Core
TA_Surface
TA_Tire
TA_Powertrain
TA_Structure
TA_Damage
TA_Vehicle
TA_Telemetry
```

## Canonical vehicle path

```
UTAVehicleDefinition
→ validation
→ FTAVehicleCompiledConfig
→ immutable vehicle + four-wheel runtime configs
→ driver controls
→ drivetrain / wheel dynamics
→ front double-wishbone + shared rack
→ rear true five-link
→ tire/suspension/anti-roll equilibrium
→ radial tire compliance
→ longitudinal/lateral tire forces
→ forces and moments at four physical contact patches
→ 6-DOF chassis integration
```

## Canonical collision baseline

```
world collision impulse at contact point
→ full external J applied once to chassis
→ Δv + Δω
→ contact/impulse transformed to chassis-local
→ spatial structural node weighting
→ remove rigid translation mode
→ remove rigid rotation mode
→ internal deformation velocity field
→ structural XPBD/plasticity/fracture
→ displaced mechanical/suspension mounts
```

## Implemented this session

### Front axle
- mirrored front-left geometry from canonical right side;
- shared physical rack translation axis;
- normalized steering → rack displacement;
- both front corners solved from one rack state;
- Ackermann delta;
- bump-steer telemetry;
- compliant tire contact;
- coupled anti-roll equilibrium using iterative left/right reaction solve.

### Rear axle
- true five-link right-side geometry;
- mirrored left-side geometry;
- rigid upright distance preservation;
- rear road-contact solve;
- rear damper motion ratio;
- rear anti-roll;
- compliant tire contact;
- structural displacement bindings for all five rear chassis pickups.

### Four-wheel vehicle runtime
- fixed wheel order FL/FR/RL/RR;
- front axle resolved before tire solve;
- rear axle resolved before tire solve;
- four self-derived `FTAWheelContactInput` values;
- four-wheel static-support source regression;
- static-stability source regression;
- acceleration source regression;
- braking source regression;
- steering → tire force → yaw source regression.

### Tire vertical compliance
Runtime tire state now includes:
- radial deflection;
- radial deflection velocity;
- initialization state.

Runtime tire config now includes:
- radial stiffness;
- progressive radial stiffness;
- radial damping;
- pressure stiffness exponent;
- maximum radial deflection.

Canonical high-fidelity contact solves approximately:
```
F_tire(travel, deflection, pressure, deflection velocity)
=
F_suspension(travel, damper, stops)
+ F_anti-roll
```

Rigid-radius contact remains as lower-fidelity/test path.

### Unsprung dynamics
An isolated explicit generalized vertical solver now exists:
- effective unsprung mass;
- tire normal force;
- suspension reaction force;
- gravity projection;
- chassis reference-frame acceleration;
- semi-implicit travel integration;
- bump/droop limit handling;
- velocity ceiling.

This solver is **not yet canonical**.

### Vehicle content compilation
`FTAVehicleCompiledConfig` now carries:
- metadata/mass/dimensions;
- `FTAVehicleRuntimeConfig`;
- `FTAFourWheelRuntimeConfig`;
- `PhysicsConfigHash`.

`UTAVehicleDefinition::BuildCompiledConfig` now compiles:
- wheel/tire runtime;
- RWD/FWD driven flags;
- core drivetrain settings;
- front hardpoints/spring/damper/anti-roll/rack;
- rear five-link/spring/damper/anti-roll;
- chassis mass/inertia.

Hardpoints are transformed:
```
vehicle-reference local → COM-local
```

Validation now covers:
- mass/inertia;
- current four-wheel topology;
- tire compliance parameters;
- suspension travel;
- drivetrain ratios;
- front/rear geometric solver validity;
- wheelbase/track vs physical wheel-center consistency.

### Physics config identity
Handling-critical fields now participate in the prototype physics hash:
- mass/COM/inertia;
- tire force/compliance;
- wheel/brake/drive flags;
- drivetrain;
- front geometry/suspension/rack/anti-roll;
- rear geometry/suspension/anti-roll.

### Collision / structure
Added:
- `TAChassisDynamics::ApplyImpulseAtWorldPoint`;
- `TAStructureImpactDistributor`;
- `TACollisionStructureCoupling`.

Structural impact distribution:
- spatial distance weighting;
- configurable deformation fraction;
- affected-node mass centroid;
- rigid translation removal;
- rigid rotation removal through point-mass inertia tensor;
- uniform node-velocity limiting;
- residual momentum telemetry;
- deformation kinetic-energy telemetry.

### Telemetry
Current trace channels include:
- simulation tick;
- physics config hash;
- engine/gear;
- chassis linear/angular velocity;
- total tire force;
- rack;
- front steering angles;
- bump steer;
- Ackermann delta;
- FL/FR/RL/RR vertical load;
- suspension travel;
- camber/toe;
- slip ratio/angle;
- tire force;
- tire surface temperature;
- pressure;
- wear;
- radial tire deflection.

CSV export exists outside the physics step.

## Documentation added / current
- `23-FRONT-AXLE-STEERING-V01.md`
- `24-REAR-MULTILINK-GEOMETRY-V01.md`
- `25-TIRE-VERTICAL-COMPLIANCE-V01.md`
- `26-UNSPRUNG-VERTICAL-DYNAMICS-V01.md`
- `27-VEHICLE-CONTENT-COMPILATION-V01.md`
- `28-COLLISION-STRUCTURE-COUPLING-V01.md`

## Important design corrections this session
1. Anti-roll load is no longer post-applied after compliant tire solve; it participates in axle equilibrium.
2. Internal coupled-solver probes work on copied state; only final state commits.
3. Tire normal load and radial tire deflection now remain causally linked.
4. Prototype static load tuning is no longer hiding axle weight bias inconsistent with the current symmetric COM seed.
5. Authored suspension geometry is compiled into runtime instead of existing only in tests.
6. Collision structural deformation no longer duplicates chassis rigid linear/angular momentum.
7. A telemetry CSV guard regression introduced during editing was detected and repaired before checkpoint.

## New/expanded tests written
Source Automation coverage now additionally includes:
- front mirror/shared rack;
- rack mapping;
- Ackermann;
- compliant anti-roll load/deflection coupling;
- rear five-link reference/bump/droop/mirror/damage;
- rear structural pickup bindings;
- four-wheel self-support;
- static stability;
- acceleration;
- braking;
- steering/yaw;
- tire radial compliance;
- pressure-dependent radial stiffness;
- tire bottoming;
- unsprung force directions;
- unsprung chassis-relative acceleration;
- unsprung travel-limit handling;
- vehicle asset → compiled config;
- physics hash sensitivity;
- invalid suspension asset rejection;
- COM-local hardpoint transform;
- compiled asset → canonical four-wheel step;
- telemetry CSV/config hash/radial deflection;
- structural impact spatial distribution;
- zero residual structural rigid linear momentum;
- zero residual structural rigid angular momentum;
- impact velocity limiting;
- chassis center/off-center impulse;
- collision chassis/structure non-double-counting.

**All tests are written but have not been executed in Unreal.**

## Still build-unverified
- UnrealHeaderTool;
- UnrealBuildTool;
- MSVC/Clang C++ compile;
- Editor module load;
- Automation tests;
- actual runtime convergence;
- profiling budgets;
- replay determinism;
- cross-machine determinism;
- driving feel/calibration.

No build/test-pass claim may be made before those operations actually run.

## Highest current technical risks
1. First UE 5.8 build can expose UHT/include/API/compiler errors.
2. Coupled compliant axle solve has not yet been numerically executed under UE Automation.
3. Rear five-link iterative projection needs convergence profiling across full travel/damage envelope.
4. Explicit unsprung dynamics is not yet integrated with chassis/contact without force double-counting.
5. Structural impact distributor seeds deformation but real collision manifolds/contact geometry do not yet exist.
6. Structural fracture/displacement events are not yet converted into the typed damage graph.
7. Structural nodes/constraints/damage bindings are not yet authored/compiled from `UTAVehicleDefinition`.
8. Tire/powertrain authoring still leaves some runtime coefficients at shared prototype defaults.
9. Aero and brake thermal dynamics remain incomplete.

## Immediate next work — no user input required

### 1. Structural result → typed damage signal bridge
Implement deterministic extraction of:
- newly fractured constraints;
- mount displacement threshold crossings;
- structural impact/deformation energy.

Emit:
- `StructuralFracture`;
- `StructuralDisplacement`;
- `ImpactEnergy`.

Do not emit duplicate events every frame for already-known fractures.

### 2. Mechanical damage routing
Bind structural/collision signals to:
- front/rear suspension mounts;
- steering rack;
- radiator support;
- wheel/hub;
- future fluid/electrical systems.

First concrete regression:
```
front-corner impact
→ internal structural deformation
→ lower-arm pickup displacement
→ changed front camber/toe
→ altered tire force
```

### 3. Structural content schema
Add authored:
- structural nodes;
- distance constraints;
- node masses;
- yield/fracture parameters;
- named mount bindings;
- damage component bindings.

Compile them into allocation-ready structure state/config.

### 4. Experimental dynamic unsprung corner
Use `TAUnsprungVerticalDynamics` on one isolated proving-ground corner:
- tire radial force on unsprung mass;
- suspension force opposite;
- chassis receives suspension reaction only;
- no direct tire-normal double application to chassis.

Compare against the current quasi-static equilibrium reference.

### 5. Regression envelope tooling
Add:
- expected ranges;
- pass/fail comparison;
- config hash;
- scenario ID;
- min/max/steady-state extraction from telemetry CSV/ring buffer.

### 6. Complete authored tire/powertrain fields
Move remaining handling-critical prototype defaults from C++ runtime defaults into versioned content definitions.

### 7. First UE 5.8 build gate
When a usable UE 5.8 build environment is available:
1. generate project files;
2. compile Development Editor;
3. repair UHT/UBT/compiler issues;
4. launch Editor;
5. run all `TorqueAtlas.*` Automation tests;
6. record exact UE/toolchain/commit/hash;
7. profile front/rear geometry, four-wheel contact, structure impact and telemetry;
8. create the first trusted regression baseline.

## Exact continuation point
Resume with **structural fracture/displacement → deterministic `FTADamageSignal` extraction**.

After that, connect those signals to the existing suspension-pickup and radiator/mechanical damage paths so one synthetic crash can demonstrate:

```
collision impulse
→ chassis motion
→ internal structural deformation
→ fracture / mount displacement event
→ changed suspension geometry
→ changed wheel alignment
→ changed tire force
→ degraded vehicle behavior
```

Do not expand world size, car roster, career/economy or production art until this full Proof-of-Physics crash-to-handling chain is demonstrated.

## Checkpoint rule
Update this file before ending every substantial work session and before switching to a new major subsystem.
