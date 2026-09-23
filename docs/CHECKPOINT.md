# Active Development Checkpoint

Updated: 2026-09-23

## Current phase
**Four-corner suspension/contact integration.**

The prototype now has a complete front-axle steering/contact runtime and a true five-link rear geometry/contact path in source. The next gate is a four-wheel self-support fixture that couples all four independently resolved contacts to the existing tire/chassis simulation.

The source remains **build-unverified** until Unreal Engine 5.8 UHT/UBT/C++ compilation and Automation execution are actually run.

## Canonical repository
`latektigroo0904/racegame`

## Runtime modules
`TA_Core`, `TA_Surface`, `TA_Tire`, `TA_Powertrain`, `TA_Structure`, `TA_Damage`, `TA_Vehicle`, `TA_Telemetry`.

## Completed physical chains

### Front axle
`driver Steering01 → shaped physical rack displacement → shared rack translation → mirrored left/right double-wishbone solve → road contact → spring/damper load → axle anti-roll → tire input`.

Implemented this phase:
- canonical right-front geometry mirrored across chassis centerline;
- rack axis deliberately remains one shared chassis-local translation axis when geometry is mirrored;
- normalized steering input with clamp, exponent and explicit steering sign;
- left/right road contacts solved independently;
- anti-roll applied after both base corner loads exist;
- Ackermann delta exposed;
- neutral-rack bump-steer telemetry exposed;
- regression source for mirror, rack clamp, shared-rack steering and asymmetric-road anti-roll.

### Rear suspension
`chassis pose → five independent link constraints → rigid upright → wheel center/orientation → road-plane travel solve → damper motion ratio → spring/damper load → tire input`.

Implemented this phase:
- true five-link rear solver; no double-wishbone substitution;
- five independent chassis pickups and upright pickups;
- rigid upright preservation through internal distance constraints;
- travel coordinate closes the six-DOF upright mechanism;
- wheel orientation reconstruction;
- camber/toe output;
- damper length output;
- per-link structural chassis-pickup displacement;
- mirrored left-side configuration;
- rear road-contact bisection;
- rear self-derived normal load and contact-patch velocity;
- rear anti-roll pair helper;
- rear conversion to `FTAWheelContactInput`.

### Existing vehicle physics retained
- 6-DOF chassis integration;
- physical force-at-point torque generation;
- engine/clutch/gearbox/final-drive/open-differential/wheel/tire chain;
- tire thermal, pressure and wear state;
- radiator/cooling/thermal engine damage;
- structural pickup displacement path.

## Telemetry
Added front-axle channels for:
- steering rack displacement;
- front-left/right steering angle;
- front-left/right bump steer;
- Ackermann delta.

Also corrected a module dependency inconsistency: `TA_Telemetry` publicly includes vehicle types and now explicitly declares `TA_Vehicle` as a dependency.

## New documentation
- `23-FRONT-AXLE-STEERING-V01.md`
- `24-REAR-MULTILINK-GEOMETRY-V01.md`

## New/expanded tests written
- mirrored front geometry;
- shared rack-axis invariant;
- steering input/rack clamp;
- same-turn-direction left/right steering;
- Ackermann differential;
- asymmetric front travel/anti-roll;
- rear five-link reference geometry;
- rear bump/droop;
- rear damper response;
- rear mirror symmetry;
- individual rear pickup damage changing alignment;
- rear reference road contact/load;
- rear chassis velocity → patch velocity.

All Automation tests remain **written but not executed**.

## Decisions and assumptions
1. Positive `Steering01` means a right turn for TA-P01. Physical rack sign is configuration, not hidden solver behavior.
2. Mirroring front hardpoints does not mirror the physical rack translation axis because one rack moves both inner tie-rods in the same chassis-local direction.
3. Rear suspension is represented as five real links attached to one rigid upright. This preserves a direct damage path per link.
4. Rear wheel travel is the sixth kinematic constraint for the initial rigid-upright solver.
5. Rear multi-link kinematic caching is disabled for now; correctness precedes optimization.
6. Current hardpoints and spring/damper values are engineering seeds, not measured production-car data.

## Risks / verification debt
1. First UE 5.8 compile may expose API/compiler issues; no build-pass claim is made.
2. The iterative rear rigid-upright projection needs real Automation execution and residual profiling across the full travel envelope.
3. Four-corner energy stability is not yet proven.
4. Tire radial compliance and unsprung mass are still absent, so current road contact is kinematic/rigid-radius.
5. Structural crash impulses are not yet distributed from real collision manifolds.
6. Ackermann and bump-steer values are geometry outputs but not yet calibrated to a target handling specification.

## Immediate next work — no user input required

### 1. Four-wheel self-support fixture
Create one high-level prototype rig containing:
- front axle runtime state;
- mirrored rear-left + rear-right multi-link states;
- four road contacts;
- front/rear anti-roll;
- four `FTAWheelContactInput` outputs;
- gravity/chassis integration.

Acceptance: at reference ride height all four corners derive their own contact, load and patch velocity without manually supplied wheel loads.

### 2. Static settle regression
From a slightly displaced chassis pose:
- integrate gravity + suspension reactions;
- verify finite settle behavior;
- check total vertical support against vehicle weight;
- check left/right symmetry;
- record ride-height/load envelopes.

### 3. Dynamic four-wheel regression
Then run source fixtures for:
- straight acceleration;
- braking;
- steering;
- asymmetric road/roll transfer.

### 4. Tire vertical compliance / unsprung mass
Only after four-wheel rigid-radius contact is stable:
- tire radial spring/damper;
- wheel/upright unsprung mass;
- wheel hop;
- curb/rough-road transient load.

### 5. Build gate
As soon as UE 5.8 execution is available: compile Development Editor, run all `TorqueAtlas.*` Automation tests, fix UHT/UBT/compiler errors, record toolchain/commit and profile solver cost.

## Exact continuation point
Resume by implementing the **full four-wheel self-support runtime/fixture** using `TAFrontAxleRuntime` for the front and `TAMultiLinkContactResolver` for both rear corners. Do not start world/career/content expansion before four-wheel self-derived support, acceleration, braking and steering are demonstrated.

## Checkpoint rule
Update this file before ending every substantial work session and before switching to a new major subsystem.
