# Vehicle Physics Promotion & Test Matrix v0.1

Updated: 2026-10-08
Status: canonical engineering gate matrix

## 1. Purpose

Define exactly when an isolated subsystem may become part of the canonical Torque Atlas vehicle runtime.

A source implementation is not automatically production physics.

Each subsystem moves through:

```
Specified
→ Isolated Source Implemented
→ Source-Sanity Green
→ UE Build Green
→ Automation Green
→ Integrated Trace Green
→ Calibrated
→ Canonical
```

## 2. Global evidence required for every promotion

Record:
- Git commit SHA;
- PhysicsConfigHash;
- Unreal Engine version;
- compiler/toolchain;
- test command;
- fixed timestep;
- scenario ID;
- vehicle definition/version;
- environment definition;
- output trace artifact;
- acceptance envelope version.

No anonymous “feels good” promotion.

## 3. Baseline vehicle gate

Before any post-PoP subsystem promotion:
- UHT pass;
- UBT/C++ pass;
- editor/module-load pass;
- full TorqueAtlas Automation pass;
- static four-wheel equilibrium evidence;
- straight-line zero-input stability;
- deterministic repeated trace;
- current aero/tire/suspension/powertrain/damage telemetry archived.

## 4. Hydraulic brake gate

Required:
- pressure/torque dimensional unit tests;
- bounded pressure rise/release;
- brake bias trace;
- thermal/fade compatibility;
- circuit failure trace;
- 100–0 stopping trace;
- no wheel reversal at standstill.

Acceptance:
- no direct pedal→wheel torque bypass remains in canonical path;
- pressure/geometry explain applied torque.

## 5. ABS gate

Required:
- dry threshold braking;
- wet threshold braking;
- split-mu braking;
- rough/bumpy braking;
- low-speed stop;
- repeated braking with hot brakes;
- one-wheel-airborne transition.

Acceptance:
- no direct wheel-speed edits;
- no tire-mu edits;
- modulation operates through pressure actuator;
- stopping/yaw behavior remains bounded.

## 6. Differential gate

Required:
- open-diff baseline parity;
- split-mu launch;
- one-wheel low load;
- power-on corner;
- coast corner;
- thermal/energy test for locking devices.

Acceptance:
- axle torque accounting closes within explicit losses;
- side-speed freedom/constraint matches backend;
- no hidden speed lock for open diff.

## 7. Tire transient gate

Required:
- steady-state skidpad parity;
- steering-step response;
- slalom phase response;
- braking-slip step;
- ABS interaction;
- low-speed creep;
- contact-loss/re-entry.

Acceptance:
- steady-state force envelope unchanged within tolerance;
- transient lag finite/continuous;
- no oscillatory numerical instability.

## 8. Dynamic unsprung gate

Required:
- static equilibrium;
- road step;
- curb input;
- sinusoidal road sweep;
- wheel-hop frequency response;
- airborne/re-contact;
- anti-roll interaction.

Acceptance:
- no road-normal force double counting;
- chassis receives suspension/link reactions only;
- stable at canonical fixed step/substep.

## 9. Fluid-network gate

Required:
- closed-system mass conservation;
- leak monotonicity;
- pump-pressure behavior;
- oil starvation;
- fuel starvation;
- coolant leak/overheat;
- brake-fluid pressure loss.

Acceptance:
- consequence derives from mass/pressure/temp/flow state;
- legacy scalar path is removed or explicitly compatibility-only.

## 10. Electrical-network gate

Required:
- battery no-load;
- starter sag;
- alternator recharge;
- ECU brownout;
- fuel-pump undervoltage;
- cooling fan undervoltage;
- fuse trip;
- disconnected accessory.

Acceptance:
- current/power/voltage signs consistent;
- no free electrical power;
- mechanical alternator load closes energy path.

## 11. TCS gate

Required:
- dry launch;
- wet launch;
- split-mu launch;
- corner exit;
- crest/unload;
- disabled transparency.

Acceptance:
- only actuator requests change;
- no tire physics modification.

## 12. ESC gate

Required:
- steering step;
- lift-off oversteer;
- split-mu braking;
- emergency lane change;
- low-mu corner;
- disabled transparency.

Acceptance:
- no direct chassis yaw torque;
- interventions visible as wheel brake/engine/diff requests.

## 13. FFB gate

Required:
- static center;
- constant-radius;
- steering step;
- curb strike;
- damage/off-center alignment;
- clipping test;
- device-independent physical output trace.

Acceptance:
- physics steering torque exists before device scaling;
- presentation layer cannot influence chassis physics.

## 14. Advanced aero gate

Required before active/ride-height maps:
- base split-aero traces;
- ride-height sweep;
- pitch sweep;
- yaw/crosswind sweep;
- high-speed stability;
- damage state comparison when damage aero exists.

Acceptance:
- maps are data-backed;
- interpolation stable;
- no speed grip multiplier.

## 15. Performance gate

Every promotion reports:
- mean solver time;
- 95th percentile;
- worst observed;
- allocations per step;
- memory/state size.

Prototype budgets are targets, not acceptance truths until measured on reference hardware.

## 16. Regression artifact naming

Recommended:

```
<ScenarioId>_
<VehicleId>_
<PhysicsHash>_
<EngineVersion>_
<CommitSha>_
<UTC timestamp>
```

## 17. Failure policy

If an integrated test fails:
1. preserve failing trace;
2. classify numerical / sign / configuration / calibration / integration ownership;
3. fix root cause;
4. rerun same scenario;
5. only widen envelope when measurement proves original envelope invalid.

Never widen tolerances solely to make CI green.
