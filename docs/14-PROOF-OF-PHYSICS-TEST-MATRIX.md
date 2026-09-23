# Proof-of-Physics Test Matrix v0.1

Status: acceptance specification
Updated: 2026-09-23

## 1. Purpose
The first major project gate is not visual polish. It is a repeatable demonstration that one vehicle behaves coherently before and after damage.

Target vehicle: TA-P01.
Target environment: Proving Ground 01.

## 2. Test philosophy
Every test records:
- build commit;
- vehicle config hash;
- solver versions;
- fixed timestep;
- initial state;
- environment state;
- telemetry;
- pass/fail criteria.

No test relies only on "feels good".

## 3. Tolerance classes
### Class A — deterministic math
Very tight numerical tolerance.

Examples:
- ratio calculations;
- lookup interpolation;
- conservation/sign checks.

### Class B — dynamic regression
Small percentage/band tolerance.

Examples:
- acceleration time;
- braking distance;
- skidpad steady state.

### Class C — crash regression
Wider envelope due to nonlinear contacts, but critical failure pattern must be stable.

Examples:
- which primary rail yields;
- whether radiator is damaged;
- wheel-alignment direction/magnitude band.

All final tolerances require empirical tuning after first implementation.

## 4. P0 unit tests
- unit conversion round trips;
- engine RPM/rad-s conversion;
- gear ratio transforms;
- torque interpolation;
- slip ratio low-speed transition;
- combined-slip envelope;
- structural yield/fracture state transition;
- stable config hashing.

## 5. P1 static vehicle tests

### P1.1 Mass balance
Measure axle/corner loads at rest.

Pass:
- total supports vehicle weight within numerical tolerance;
- left/right symmetric setup remains symmetric;
- front/rear distribution matches compiled definition target.

### P1.2 Static ride height
Spawn, settle, measure.

Pass:
- no persistent oscillation;
- target ride-height range reached;
- no unwanted bump-stop contact.

### P1.3 Steering center
Pass:
- centered rack gives expected toe;
- no unintended steering force bias on symmetric flat surface.

## 6. P2 longitudinal tests

### P2.1 Idle
Run engine unloaded.

Pass:
- converges around target idle;
- no sustained numerical hunting;
- starter/idle controller torque bounded.

### P2.2 Launch
Controlled clutch release.

Pass:
- clutch slip occurs continuously;
- engine can bog or stall if abused;
- no artificial minimum-speed assist.

### P2.3 0–100 km/h
Target design region for TA-P01:
approximately 5 seconds, to be achieved by calibrated physics rather than hard lock.

Regression records:
- elapsed time;
- shift RPM;
- wheel slip;
- clutch energy;
- longitudinal acceleration.

### P2.4 Coast-down
Throttle released in gear and neutral.

Pass:
- in-gear deceleration exceeds neutral due to drivetrain/engine braking;
- no unexplained propulsive torque.

## 7. P3 braking tests

### P3.1 100–0 dry
Record:
- distance;
- peak/mean decel;
- slip ratio;
- brake temperatures;
- wheel loads.

Initial target band: approximately 34–37 m for the design concept, subject to tire calibration.

### P3.2 Repeated stops
Multiple high-energy stops.

Pass:
- temperature rises;
- fade emerges if thresholds are exceeded;
- cooling occurs between stops.

### P3.3 Split-mu
Left/right surfaces differ.

Pass:
- yaw response emerges physically;
- ABS/assist layer, when disabled, does not mask asymmetry.

## 8. P4 lateral tests

### P4.1 Constant-radius skidpad
Increase speed gradually.

Record:
- lateral acceleration;
- steering angle;
- slip angles;
- axle saturation order.

Pass:
- progressive limit behaviour;
- no discontinuous force cliff caused by solver switching.

### P4.2 Slalom
Pass:
- stable transient load transfer;
- no unexplained energy gain;
- steering/FFB follows tire aligning moment trend.

### P4.3 Lift-off response
Pass:
- transient yaw is plausible and repeatable;
- no scripted oversteer event.

## 9. P5 surface/weather tests

### P5.1 Dry to wet transition
Same maneuver after rainfall.

Pass:
- lower peak grip;
- lower tire surface temperature;
- longer braking distance.

### P5.2 Single-wheel puddle
Right-side wheel crosses standing water.

Pass:
- local force reduction/drag;
- yaw disturbance direction matches puddle side;
- no global all-wheel grip switch.

### P5.3 Progressive aquaplaning
Increase speed through fixed water depth.

Pass:
- tire load/contact contribution declines continuously;
- threshold is not a binary boolean;
- worn tire hydroplanes earlier/stronger than equivalent new tire.

## 10. P6 powertrain abuse tests

### P6.1 Wrong downshift
Select ratio that wheel-drives engine over redline.

Pass:
- electronic limiter cannot prevent mechanical over-rev;
- engine damage channel accumulates.

### P6.2 Clutch abuse
Hold high slip under power.

Pass:
- clutch heats;
- capacity/fade changes;
- wear increases;
- recovery/failure follows configured thermal model.

### P6.3 Open differential split traction
One driven wheel on low mu.

Pass:
- open-diff behaviour does not act like locked axle;
- torque/wheel-speed response remains physically coherent.

## 11. P7 suspension damage tests

### P7.1 Curb impact
Front-left wheel hits prescribed curb.

Pass:
- impact load transmits through wheel/upright/suspension;
- if threshold exceeded, geometry changes;
- altered alignment is visible in telemetry.

### P7.2 Bent tie rod
Inject known geometric offset.

Pass:
- toe changes from geometry;
- vehicle develops corresponding steering pull/tire scrub.

### P7.3 Pickup displacement
Move lower-arm chassis pickup via structural node.

Pass:
- kinematic solver recomputes alignment;
- no manual damage penalty required.

## 12. P8 structural crash tests

### P8.1 Frontal barrier
Speeds:
30, 50, 64, 80 km/h.

Record:
- impact energy;
- peak decel;
- node displacement;
- yielded/broken constraints;
- passenger-cell deformation;
- radiator state;
- alignment state.

Pass:
- deformation severity rises monotonically in broad terms with impact energy;
- no numerical explosion;
- passenger cell and crash zone behave differently according to authored structure.

### P8.2 40% offset
Pass:
- asymmetric structure loads;
- asymmetric wheel/suspension consequences.

### P8.3 Pole
Pass:
- localized intrusion/load path differs from broad barrier impact.

## 13. P9 integrated Proof-of-Physics scenario
Canonical demonstration:

1. spawn cold TA-P01;
2. drive dry handling section;
3. warm tires/brakes;
4. rain begins;
5. grip and temperature response changes;
6. cross standing water;
7. front-left crash occurs at defined speed/offset;
8. front structure deforms;
9. suspension alignment changes;
10. radiator leaks;
11. continue driving;
12. steering pull and tire scrub remain;
13. coolant level falls;
14. engine temperature rises;
15. protection/derate activates;
16. severe continuation can produce engine failure.

## 14. Integrated acceptance criteria
The gate passes only if:
- every consequence is traceable to simulated state;
- no scripted "post-crash mode" is required;
- telemetry explains the observed behaviour;
- replay/repeat runs fall within defined regression envelopes;
- average physics budget remains within prototype target on reference hardware.

## 15. Performance telemetry
Per test record:
```
VehicleSolver_ms
TireSolver_ms
Suspension_ms
Powertrain_ms
Structure_ms
DamageGraph_ms
TotalPhysics_ms
Substeps
ConstraintIterations
AllocatedBytesDuringStep
```

Goal: zero dynamic allocation in the inner vehicle solver after initialization.

## 16. Reference hardware
Do not freeze a commercial hardware target yet.

For R&D, define a named internal reference machine at implementation time and preserve its exact CPU/build configuration in test reports.

## 17. Automation
Use Unreal C++ automation for:
- low-level calculations;
- compiled-config validation;
- headless/simple world regression where practical.

Functional/proving-ground tests may use deterministic driver scripts and telemetry comparisons.

## 18. Gate rule
Do not start broad content production until the integrated P9 scenario passes repeatedly and performance trends show a viable scaling path.
