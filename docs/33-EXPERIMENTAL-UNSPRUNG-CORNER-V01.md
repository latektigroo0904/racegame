# Experimental Dynamic Unsprung Corner v0.1

Status: source-implemented experiment, not canonical  
Updated: 2026-09-23

## 1. Purpose

The current production/proving-ground contact path is quasi-static:

```
tire radial force ≈ suspension reaction
```

That model is stable, deterministic and useful, but it cannot represent explicit wheel-hop/unsprung inertia.

This experiment adds one front double-wishbone corner with a real generalized unsprung mass while leaving `TAFourWheelVehicleRuntime` unchanged.

## 2. Canonical safety rule

The experiment is **not** allowed to become the production contact path until it proves:

- static agreement with the quasi-static baseline;
- bounded road-step transients;
- deterministic fixed-input output;
- stable travel-limit behavior;
- correct force ownership;
- acceptable CPU cost in a real UE build/profile.

## 3. Force ownership

Positive suspension travel is bump/upward relative to the chassis.

Unsprung generalized equation:

```
m_u * q_ddot =
    F_tire_along_travel
  - F_suspension
  + m_u * g_along_travel
  - m_u * a_chassis_along_travel
```

Critical distinction:

```
tire normal force
→ unsprung mass

suspension/link reaction
→ chassis

NOT:

tire normal force
→ unsprung mass
→ and directly to chassis again
```

At static equilibrium on a level road with gravity:

```
F_tire - F_suspension ≈ m_unsprung * g
```

Therefore tire normal load and chassis suspension reaction are intentionally not identical.

## 4. Chassis moment arm

The existing vehicle contact API was extended with an optional suspension-force application point.

`FTAWheelContactInput` now distinguishes:

- `ContactPointWorldM` — tire/road contact point for tire forces;
- `SuspensionForceApplicationPointWorldM` — optional chassis-side point for suspension reaction.

Legacy/canonical contacts remain backwards compatible: when the optional point is not supplied, suspension force continues to use the contact point.

The experimental corner supplies the damaged chassis-side damper mount as the suspension reaction point.

This prevents a dynamic-unsprung model from using the road-contact lever arm for a force that physically enters the sprung chassis through the suspension structure.

## 5. Adapter

Files:

- `TAExperimentalUnsprungCorner.h`
- `TAExperimentalUnsprungCorner.cpp`

Configuration contains:

- double-wishbone geometry;
- suspension force model;
- tire vertical model;
- unsprung mass/travel configuration;
- gravity;
- internal local substeps.

State contains:

- geometry solver state;
- suspension state;
- tire vertical state;
- unsprung travel/travel velocity.

## 6. Initialization

`InitializeFromQuasiStatic()` seeds the experiment from the existing compliant contact solver.

Purpose:

- avoid an arbitrary initial tire penetration;
- start from a known valid geometry/contact solution;
- isolate differences caused by explicit unsprung inertia rather than initialization noise.

Dynamic travel velocity starts at zero.

With gravity enabled, the seed is intentionally not the final dynamic equilibrium because the quasi-static solver has no explicit unsprung weight. The dynamic state must settle until:

```
F_tire - F_suspension - m_u*g ≈ 0
```

for an upward-positive travel axis.

## 7. Dynamic step

Per internal substep:

1. solve wishbone geometry at current unsprung travel;
2. estimate current motion ratio;
3. calculate spring/damper/stop reaction;
4. calculate wheel center relative to road;
5. calculate requested tire radial deflection;
6. calculate tire radial normal force;
7. project road normal force onto the travel axis;
8. integrate unsprung travel acceleration/velocity/position;
9. enforce travel limits.

After substeps, the current geometry/contact is rebuilt into a normal `FTAWheelContactInput`.

## 8. Vehicle-contact semantics

Dynamic contact output:

```
VehicleContact.VerticalLoadN
    = tire road normal force

VehicleContact.SuspensionForceWorldN
    = chassis suspension reaction only

VehicleContact.SuspensionForceApplicationPointWorldM
    = chassis-side damaged damper mount
```

The normal load is used by the longitudinal/lateral tire model.

The chassis solver does **not** separately receive tire normal force.

## 9. Diagnostics

The adapter reports:

- tire normal force;
- chassis suspension reaction;
- tire requested/effective deflection;
- travel/travel velocity;
- generalized tire force;
- generalized gravity force;
- generalized chassis inertial force;
- generalized net force balance;
- travel-limit impact state;
- suspension/geometry state.

The force-balance value is:

```
F_balance =
    F_tire_projected
  - F_suspension
  + F_gravity
  + F_chassis_inertial
```

At settled equilibrium it should approach zero.

## 10. Validation

The experimental adapter rejects:

- invalid wishbone geometry;
- invalid unsprung mass/travel settings;
- invalid/negative suspension rates;
- invalid tire vertical parameters;
- non-finite input/state;
- incompatible unsprung and geometry travel ranges;
- invalid road normals;
- more than 16 internal substeps.

For this experiment, geometry and unsprung travel limits must describe the same generalized coordinate range.

## 11. Source-level tests

Added tests cover:

### Quasi-static seed
With gravity disabled:
- contact initializes from canonical equilibrium;
- tire force and suspension reaction are close;
- first dynamic step remains close to seed travel.

### 20 mm road step
- dynamic bump response occurs;
- travel and velocity remain bounded;
- transient damps;
- settled dynamic travel is compared with quasi-static raised-road equilibrium;
- settled tire load is compared with quasi-static load.

### No normal-force double counting
With gravity enabled:
- tire load exceeds chassis suspension reaction by approximately unsprung weight;
- tire normal load is sent to tire input;
- chassis receives suspension reaction only;
- chassis force uses an explicit suspension application point;
- generalized equilibrium residual trends near zero.

### Separate chassis moment arm
Vehicle runtime test verifies:
- same net suspension force at COM produces no angular response;
- the same force at an offset explicit suspension point produces the expected chassis moment;
- linear force remains unchanged.

### Determinism
Two identical states receiving identical inputs are expected to produce identical travel, velocity and tire normal load.

## 12. Known limitations

Still intentionally omitted:

- full 3D upright rigid-body inertia;
- individual control-arm inertia;
- bushing compliance;
- lateral/longitudinal hub compliance;
- rim strike impulses;
- wheel bearing compliance;
- tire carcass modal dynamics;
- road-contact patch persistence;
- anti-roll dynamic state;
- full vehicle coupling of four dynamic corners;
- runtime CPU measurements.

## 13. Promotion gate

Do **not** replace the canonical compliant-contact path merely because source tests exist.

Promotion requires a real UE 5.8 run with:

1. Automation pass;
2. static equilibrium traces;
3. road-step traces;
4. quasi-static comparison;
5. no-normal-force-double-count proof;
6. profiling;
7. stable four-corner integration experiment.

Until then this subsystem remains experimental.
