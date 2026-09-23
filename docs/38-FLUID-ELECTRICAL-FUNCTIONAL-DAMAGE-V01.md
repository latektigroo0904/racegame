# Fluid & Electrical Functional Damage v0.1

Status: source-implemented, UE-build-unverified  
Updated: 2026-09-23

## 1. Scope

This layer adds two concrete powertrain-support failure paths:

- `ElectricalBus`
- `FuelDelivery`

The design deliberately does **not** add generic vehicle HP or a single abstract “electrical damage” multiplier applied everywhere.

Current consequences are limited to systems the existing powertrain model can represent physically and traceably.

## 2. Electrical bus state

Persistent state:

```
Damage01
StarterEfficiency01
EngineControlEfficiency01
```

### Starter consequence

```
effective starter torque
=
base starter torque
× starter efficiency
```

A complete starter-side electrical failure may reduce starter torque to zero.

### Engine-control consequence

```
effective combustion torque
=
base combustion torque
× engine-control efficiency
× fuel-delivery efficiency
```

This represents loss of ECU/ignition/injector-control authority at the current abstraction level.

It is intentionally separate from:

- thermal derate;
- accumulated engine thermal damage;
- clutch state;
- driveline losses.

## 3. Fuel-delivery state

Persistent state:

```
Damage01
DeliveryEfficiency01
```

Fuel-delivery efficiency scales combustion torque once, after the base powertrain solver has evaluated:

- run state;
- RPM;
- torque curve;
- throttle;
- limiter;
- idle control;
- thermal/damage factors already owned by the engine solver.

A complete delivery loss can remove combustion torque.

The engine then continues according to its real current angular speed, friction, clutch load and run-state logic; it is not teleported directly to zero RPM.

## 4. Signal types

The damage router now supports two pre-existing generic signal types as first-class route inputs:

- `FluidPressureLoss`
- `ElectricalDisconnection`

Route acceptance is explicit:

```
bAcceptFluidPressureLoss
bAcceptElectricalDisconnection
```

Crash-driven structure signals remain supported separately:

- ImpactEnergy
- StructuralDisplacement
- StructuralFracture

Therefore the same consumer can later receive either:

```
structure crash damage
→ component route
```

or:

```
fluid/electrical system event
→ component route
```

without special-case powertrain code.

## 5. Discrete signal severity convention

For `FluidPressureLoss` and `ElectricalDisconnection`:

- finite scalar > 0: scalar is interpreted as normalized severity in [0,1];
- scalar <= 0: event represents complete/discrete loss, severity = 1;
- non-finite scalar: rejected as zero severity.

This supports both partial degradation and a simple connector/line failure event.

## 6. Monotonic damage

Electrical/fuel damage is non-healing:

```
Damage01 = max(previous, new severity)
Efficiency = min(previous, target efficiency)
```

A later smaller event cannot restore starter, engine-control or fuel-delivery capacity.

Repair will be a separate explicit future system.

## 7. Separation from engine thermal damage

The existing engine solver owns:

```
ThermalTorqueFactor
DamageTorqueFactor
```

Electrical and fuel factors do not overwrite either state.

Canonical combustion path:

```
base solver combustion torque
× electrical engine-control efficiency
× fuel-delivery efficiency
```

This prevents one subsystem from silently erasing another subsystem’s damage state.

## 8. Vehicle authoring

`FTAVehicleDamageRouteAuthoringDefinition` now supports:

Consumers:
- `ElectricalBus`
- `FuelDelivery`

Signal acceptance:
- fluid-pressure loss;
- electrical disconnection.

Calibration:
- minimum starter efficiency;
- minimum engine-control efficiency;
- minimum fuel-delivery efficiency.

These fields are compiled into runtime routes and included in `PhysicsConfigHash`.

## 9. Crash integration

The end-to-end crash regression now includes:

```
collision
→ internal structure deformation
→ mount displacement
→ ElectricalBus / FuelDelivery route
→ persistent powertrain-support damage
```

The same crash regression already covers radiator, steering rack, wheel hub, suspension corner and anti-roll link.

No separate crash-only fuel/electrical codepath exists.

## 10. Telemetry

Global telemetry records:

- electrical damage;
- starter efficiency;
- engine-control efficiency;
- fuel-delivery damage;
- fuel-delivery efficiency.

These fields are exported to CSV and exposed as regression metrics.

Current telemetry CSV contract was mechanically audited after this change:

- global row: 26 format placeholders ↔ 26 values;
- per-wheel header: 26 placeholders ↔ 26 wheel-name arguments;
- per-wheel row: 26 numeric placeholders ↔ 26 values.

## 11. Source-level regressions

Coverage includes:

### Damage router
- partial electrical disconnect;
- full electrical disconnect;
- partial fuel-pressure loss;
- full fuel-pressure loss;
- monotonic minimum efficiency behavior.

### Vehicle runtime
- starter electrical loss reduces/prevents cranking torque;
- fuel-delivery loss reduces combustion acceleration;
- engine-control loss reduces combustion acceleration.

### Vehicle definition
- authoring compiles to runtime consumers;
- signal acceptance flags survive compilation;
- minimum efficiencies survive compilation;
- calibration and consumer changes alter physics hash.

### Crash pipeline
- structural displacement reaches electrical damage state;
- structural displacement reaches fuel-delivery state.

### Telemetry/regression
- all five global health channels map correctly;
- CSV exports all five channels;
- regression envelopes directly evaluate electrical damage, starter efficiency and fuel-delivery efficiency.

## 12. Explicit non-goals in v0.1

This is not yet a full fluid or electrical network.

Not implemented yet:

- fuel tank volume or slosh;
- physical fuel leakage mass flow;
- fuel fire/ignition;
- oil pressure network;
- brake-fluid pressure/boiling;
- battery voltage/current/state of charge;
- alternator;
- fuses/relays;
- individual wire harness graph;
- lighting/electronics consumers;
- limp-mode logic;
- ECU fault codes.

Those systems can be layered later using the same typed-event architecture.

## 13. Governing rule

**Damage must disable or degrade a concrete physical function. It must not become generic vehicle HP.**
