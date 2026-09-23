# Active Development Checkpoint

Updated: 2026-09-23

## Current phase
**Proof-of-Physics crash-to-handling closure + regression baseline tooling.**

The TA-P01 source path now spans authored vehicle/structure content, four compliant wheel contacts, 6-DOF chassis motion, collision-to-structure coupling, persistent structural deformation, typed damage signals, radiator routing, structural pickup displacement injection and a source-level regression that continues through changed wheel alignment into changed tire force.

The source remains **build-unverified** until Unreal Engine 5.8 UHT/UBT/C++ compilation and Automation execution are actually run.

## Canonical repository
`latektigroo0904/racegame`

## Runtime modules
`TA_Core`, `TA_Surface`, `TA_Tire`, `TA_Powertrain`, `TA_Structure`, `TA_Damage`, `TA_Vehicle`, `TA_Telemetry`.

## Canonical driving path
```
UTAVehicleDefinition
→ validation / FTAVehicleCompiledConfig
→ driver controls
→ drivetrain + wheel dynamics
→ front double-wishbone/shared rack + rear five-link
→ suspension/anti-roll/tire radial equilibrium
→ tire forces at four physical contact patches
→ 6-DOF chassis integration
```

## Canonical crash-to-handling path
```
collision impulse at world contact
→ chassis Δv + Δω (external impulse applied once)
→ momentum-neutral structural deformation excitation
→ structure solve / plasticity / fracture
→ deterministic typed damage signals
→ persistent structure state
→ structural pickup bindings
→ suspension hardpoint offsets
→ camber/toe/load change
→ tire-input change
→ tire-force change
```

## Implemented baseline

### Four-wheel vehicle
- fixed FL/FR/RL/RR order;
- front shared rack, mirrored double wishbone, Ackermann and bump steer;
- rear true five-link and mirrored left side;
- front/rear anti-roll coupled inside compliant tire/suspension equilibrium;
- tire radial spring/damper/progressive/pressure-dependent compliance;
- 6-DOF chassis force-at-point integration;
- RWD prototype powertrain, clutch, open differential and wheel inertia;
- engine thermal/radiator degradation chain.

### Structure and damage
- spatial structural impact distribution;
- rigid translation/rotation removal from internal deformation excitation;
- chassis/structure collision coupling without duplicate rigid momentum;
- authored structural nodes, constraints, mount bindings and damage routes compiled into runtime config;
- `TAStructureDamageBridge` emits deterministic `StructuralFracture`, `StructuralDisplacement` and `ImpactEnergy` signals with duplicate-fracture suppression;
- `TAVehicleDamageRouter` currently routes supported signals to radiator damage;
- `TACompiledDamageRuntime` owns persistent structure/damage state and maps current structure displacement into front/rear suspension damage offsets.

### Crash-to-tire-force regression closure
`TACompiledDamageRuntimeTests.cpp` now explicitly verifies:
```
front-corner crash
→ persistent structural node displacement
→ front-right lower-arm pickup offset
→ changed camber/toe
→ changed tire force at the same reference chassis state
```
This is source-level coverage only until UE Automation runs.

### Telemetry and regression envelopes
Existing telemetry includes simulation/config identity, chassis state, powertrain, wheel loads, suspension travel/alignment, tire slip/force/temperature/pressure/wear and radial deflection. CSV export remains outside the physics step.

New `TARegressionEnvelope` provides allocation-free scalar trace evaluation for:
- sample count;
- min/max;
- full-trace mean;
- trailing steady-state mean;
- expected min/max/steady-state ranges;
- deterministic pass/fail result.

Automation source tests cover summary math, pass/fail envelopes and invalid-input rejection.

## Important assumptions
1. TA-P01 remains the proving-ground vehicle; current four-wheel topology is intentionally constrained.
2. Structural displacement is persistent mechanical geometry damage, not a cosmetic-only effect.
3. Collision rigid impulse belongs to the chassis exactly once; structural excitation contains internal deformation modes only.
4. The quasi-static compliant contact solver remains canonical until explicit unsprung dynamics demonstrates equal-or-better stability without normal-force double counting.
5. Regression envelopes are diagnostics, not gameplay tuning authority; baseline ranges become trusted only after UE execution.

## Highest current risks
1. First UE 5.8 build may expose UHT/include/API/compiler errors.
2. Coupled compliant axle and five-link convergence have not been numerically executed in UE.
3. The new crash-to-tire-force assertion threshold has not yet been measured under the real Automation runtime.
4. Explicit unsprung dynamics is isolated and not canonical.
5. Real collision manifolds/contact persistence are not yet integrated; crash tests use synthetic impulses.
6. Damage routing is still narrow: radiator is signal-routed, while suspension geometry currently consumes persistent structure displacement directly.
7. Steering rack, wheel/hub, fluid and electrical damage consumers remain incomplete.
8. Aero and brake thermal dynamics remain incomplete.
9. Tire/powertrain authoring still contains prototype defaults that should become versioned content.

## Build-verification status
Still unverified:
- UnrealHeaderTool;
- UnrealBuildTool;
- MSVC/Clang compile;
- Editor module load;
- all `TorqueAtlas.*` Automation tests;
- runtime convergence/profiling;
- replay/cross-machine determinism;
- driving feel/calibration.

No build/test-pass claim may be made before those operations actually run.

## Immediate next work — no user input required

### 1. Finish regression envelope integration
Connect `TARegressionEnvelope` to telemetry traces/scenario metadata:
- scenario ID;
- physics config hash;
- metric name;
- expected range set;
- extracted min/max/steady-state;
- machine-readable pass/fail report.

First scenarios: static settle, 0→speed acceleration, braking, constant-steer yaw, asymmetric-road load transfer, synthetic front-corner crash.

### 2. Damage consumer expansion
Add typed consumers/bindings for:
- steering rack displacement/damage;
- wheel/hub damage;
- suspension component failure severity;
- radiator support (already baseline-routed);
- future fluid/electrical systems.

Avoid duplicating the direct structure→hardpoint geometry path; typed events should represent discrete/threshold effects while continuous structural displacement remains geometry truth.

### 3. Experimental dynamic unsprung corner
Integrate one isolated proving-ground corner with `TAUnsprungVerticalDynamics` and enforce:
- tire radial force acts on unsprung mass;
- suspension reaction acts on chassis;
- no direct duplicate tire-normal force on chassis;
- compare transient response against quasi-static canonical solver.

### 4. Complete authored tire/powertrain coefficients
Move remaining handling-critical prototype defaults into `UTAVehicleDefinition` and include them in config identity/hash where relevant.

### 5. First UE 5.8 build gate
When a usable UE 5.8 environment exists:
1. generate project files;
2. compile Development Editor;
3. repair UHT/UBT/compiler issues;
4. launch Editor;
5. run all `TorqueAtlas.*` tests;
6. record exact UE/toolchain/commit/config hash;
7. profile geometry/contact/structure/telemetry;
8. capture first trusted regression envelopes.

## Exact continuation point
Resume with **telemetry → scenario regression report integration** around `TARegressionEnvelope`.

Then add the first scenario-level baseline schema so a future UE run can emit a compact record such as:
```
scenario_id
physics_config_hash
metric
observed_min
observed_max
observed_steady_state
expected_ranges
pass_fail
```

After that, proceed to typed steering/wheel-hub damage consumers and the isolated dynamic-unsprung experiment.

Do not expand world size, car roster, career/economy or production art until the Proof-of-Physics chain has been compiled, executed and captured as a trusted regression baseline.

## Checkpoint rule
Update this file before ending every substantial work session and before switching to a new major subsystem.
