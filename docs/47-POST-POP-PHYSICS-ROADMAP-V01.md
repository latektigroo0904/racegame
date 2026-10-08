# Post-Proof Vehicle Physics Roadmap v0.1

Updated: 2026-10-08
Status: implementation roadmap; canonical promotion remains gated by UE 5.8 execution

## 1. Purpose

This document defines the development order after the current source-complete Proof-of-Physics baseline.

The objective is to avoid adding isolated realism features that create duplicate force paths, hidden assist logic or configuration drift.

The ordering is dependency-driven.

## 2. Hard gate: executable Proof-of-Physics

Before any new subsystem becomes canonical vehicle physics:

1. UnrealHeaderTool passes under Unreal Engine 5.8;
2. UnrealBuildTool/C++ compilation passes for TorqueAtlasEditor Development;
3. editor modules load;
4. `Automation RunTest TorqueAtlas.` completes;
5. stationary four-wheel P1.1 evidence is captured at 120 Hz;
6. provisional static-load acceptance is reviewed against real traces;
7. current aero, tire, suspension, drivetrain, damage and telemetry baselines are archived with physics hash and engine/compiler provenance.

New isolated experiments may exist in source, but canonical promotion waits for this gate.

## 3. Phase P1.2 — Brake actuation and wheel-slip control

### Scope
- pedal/booster/master-cylinder abstraction;
- front/rear hydraulic split;
- per-corner line pressure;
- caliper piston/effective radius conversion;
- brake bias;
- fluid temperature/boiling state;
- pressure loss from line/circuit damage;
- ABS pressure modulation;
- optional mechanical handbrake/separate rear circuit.

### Why first
Current brake thermal/wear already exists, but brake input jumps directly to wheel torque capacity.
Hydraulic actuation must exist before:
- realistic ABS;
- brake-line damage;
- pedal feel;
- fluid boil;
- ESC brake intervention.

### Promotion gate
100–0 braking, split-mu braking, repeated hot braking, line-loss braking and ABS cycling must be deterministic and energy/sign-correct.

## 4. Phase P1.3 — Differential and traction architecture

### Scope
Backend-neutral differential interface:
- open;
- spool/locked;
- clutch LSD;
- helical/torque-biasing;
- viscous;
- electronically controlled clutch;
- active torque vectoring.

Rotational state must eventually include carrier/side/inertial effects rather than static torque splitting only.

### Why before traction control
TCS must command engine/brake torque against a physically meaningful driveline.
It must not compensate for an unrealistic differential model.

## 5. Phase P1.4 — Tire transient dynamics

### Scope
Add stateful force build-up:
- longitudinal relaxation;
- lateral relaxation;
- pneumatic-trail dynamics;
- carcass compliance hooks;
- low-speed continuity.

Steady-state force law remains replaceable.

### Why
Current slip-force response is effectively instantaneous.
Real transient response is required for:
- steering feel;
- ABS/TCS stability;
- slalom response;
- curb/road disturbances;
- high-frequency chassis/unsprung coupling.

## 6. Phase P1.5 — Dynamic unsprung promotion

The isolated unsprung vertical solver already exists.

Promotion requires:
- all four corners;
- correct tire-normal/suspension-force ownership;
- chassis reference acceleration coupling;
- stable wheel-hop response;
- road-step test;
- no duplicate tire normal force on chassis;
- deterministic static settling.

Quasi-static compliant contact remains:
- initialization helper;
- validation oracle;
- possible lower-LOD backend.

## 7. Phase P1.6 — Fluid and electrical physical networks

### Fluid domains
- brake fluid;
- engine oil;
- coolant;
- fuel.

### Electrical domains
- battery;
- starter;
- alternator;
- main bus;
- ignition/ECU;
- critical fuse/relay nodes;
- optional lights/accessories later.

This phase replaces selected normalized functional efficiencies with physically derived pressures/voltages/flows where useful.

The existing typed damage signals remain valid adapters.

## 8. Phase P1.7 — Driver assists

Only after their controlled physical systems exist:

- ABS;
- traction control;
- stability control;
- launch control;
- hill hold;
- configurable realism profiles.

Assists modify commands:
- hydraulic brake pressure;
- engine torque request;
- clutch request;
- active differential request.

They never edit:
- tire friction coefficient;
- chassis velocity;
- yaw rate directly;
- wheel speed directly.

## 9. Phase P1.8 — Steering force feedback

Canonical rack-force chain:

tire aligning moment
→ steering geometry
→ rack force/torque
→ steering-column compliance/friction
→ steering wheel torque
→ device-safe FFB output

Damage and caster/scrub geometry act through the same physical path.

Canned road texture may be additive presentation only.

## 10. Phase P2 — Advanced powertrain

After base vehicle dynamics are executable:
- turbo state;
- throttle/manifold response;
- engine accessory loads;
- over-rev damage;
- gearbox synchronization;
- shift timing;
- gear damage;
- half-shaft/CV compliance;
- EV/hybrid backend;
- regenerative braking integration.

## 11. Phase P3 — Advanced aero

Only after base split-aero traces exist:
- ride-height maps;
- pitch sensitivity;
- diffuser/ground effect;
- aero stall;
- active wing/DRS;
- radiator/cooling drag;
- damage-dependent aero;
- slipstream/wake.

No CFD-sized system is required for runtime. Offline maps may later be generated from CFD/wind-tunnel-like data.

## 12. Phase P4 — Vehicle environment coupling

- road temperature;
- precipitation;
- local water film;
- drainage/runoff;
- snow/ice;
- dirt/rubber;
- wind fields;
- ambient pressure/temperature.

These feed existing surface/tire/aero interfaces rather than bypassing them.

## 13. Phase P5 — World/traffic integration

Once one vehicle is repeatably correct:
- road-network runtime;
- GeoForge import;
- traffic agents;
- collision traffic LOD;
- weather zones;
- streaming;
- persistent vehicle ownership.

## 14. Required invariant set

1. SI units inside physics.
2. Fixed-step deterministic interfaces.
3. No generic vehicle HP.
4. No speed-grip multiplier.
5. No direct velocity correction for normal assists.
6. Every persistent physics calibration enters PhysicsConfigHash.
7. Environment-only transient inputs are not vehicle-asset hashed.
8. Telemetry observes exact applied physics and does not recompute.
9. Damage changes concrete subsystem parameters/states.
10. Experimental solvers stay behind explicit promotion gates.

## 15. Definition of implementation-ready

A subsystem is implementation-ready only when its spec defines:
- configuration;
- mutable runtime state;
- inputs;
- outputs;
- equations/sign conventions;
- integration ownership;
- authoring/compiler/hash behavior;
- telemetry;
- failure behavior;
- deterministic tests;
- promotion gate;
- explicit non-goals.

## 16. Recommended immediate sequence

Current:
- finish UE 5.8 executable acceptance.

Prepared next:
1. hydraulic brake actuation/ABS;
2. differential architecture;
3. tire transients;
4. dynamic unsprung promotion;
5. fluid/electrical networks.

This ordering minimizes later rewrites because each controller operates on a physical actuator that already exists.
