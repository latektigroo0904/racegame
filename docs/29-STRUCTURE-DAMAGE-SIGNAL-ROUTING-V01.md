# Structure → Damage Signal Routing v0.1

Status: implemented prototype baseline
Updated: 2026-09-23

## Purpose
Convert persistent structural solver state into deterministic, typed functional-damage events without turning the structure solver into a vehicle-specific component system.

Canonical direction:

```
TA_Structure
→ physical node/constraint state
→ TA_Damage bridge
→ typed FTADamageSignal queue
→ TA_Vehicle data-driven consumer routing
→ component state
```

Dependency direction remains one-way:
- `TA_Structure` does not depend on `TA_Damage`;
- `TA_Damage` may observe structure state;
- `TA_Vehicle` may route generic damage signals to vehicle-specific consumers.

## Structural fracture events
Each constraint has persistent bridge state:
```
FractureEventEmitted[constraint]
```

A newly broken constraint emits:
- type: `StructuralFracture`;
- source: constraint index;
- target: authored component index or generic structure;
- scalar: current absolute strain estimate;
- vector: current node-B minus node-A span.

The bridge marks the fracture emitted only after a successful queue push.

Therefore a full queue does not silently lose permanent event state; the event can be retried.

## Mount displacement events
A mount binding contains:
- target component index;
- weighted structural node set;
- ascending displacement thresholds.

Current mount displacement:

```
D =
weighted_average(
    NodeCurrent - NodeReference
)
```

Threshold state stores the highest level already emitted.

A new event is emitted only when:
```
CurrentLevel > HighestPreviouslyEmittedLevel
```

This prevents frame-by-frame event spam while still allowing escalating structural damage.

## Impact energy events
The structural collision distributor already reports injected internal deformation kinetic energy.

When non-zero, the bridge can emit:
- type: `ImpactEnergy`;
- scalar: injected deformation energy;
- target: authored impact target component.

This is a functional damage signal, not a second rigid-body impulse.

## Vehicle damage router
Vehicle-specific routing uses explicit bindings:

```
TargetComponentIndex
→ ConsumerType
→ accepted signal classes
→ consumer parameters
```

Unknown targets are ignored rather than guessed.

Duplicate target routes are rejected.

## First consumer: radiator
The radiator consumer currently accepts:

### ImpactEnergy
```
ImpactEnergy *
route energy scale
→ radiator puncture/leak model
```

### StructuralDisplacement
```
crush =
clamp(
    displacement / full_crush_displacement,
    0, 1
)
```

Crush reduces airflow through the existing radiator model.

### StructuralFracture
Optional route behavior can interpret a fracture specifically authored as radiator-support failure as full support crush.

## Suspension rule
Suspension geometry is **not** changed from a damage event percentage.

The actual structure node displacement remains the authoritative input:

```
structure node displacement
→ weighted pickup binding
→ physical pickup offset
→ suspension geometry re-solve
```

StructuralDisplacement events exist for:
- telemetry;
- gameplay/component notification;
- threshold logic;
- future repair/economy state.

They do not replace geometry.

## Crash pipeline ordering
`TACrashDamagePipeline::ProcessImpact`:

1. clear per-impact damage-event queue;
2. apply external chassis collision impulse once;
3. inject momentum-neutral internal structural deformation mode;
4. emit structural impact-energy event;
5. advance structural solver;
6. emit newly crossed fracture/displacement events;
7. deterministic queue sort;
8. route signals to vehicle components.

## Persistent compiled damage runtime
`TACompiledDamageRuntime` owns:
- current structural nodes;
- current structural constraints;
- impact scratch;
- bridge event state;
- damage queue.

After a crash it can build all current suspension offsets for the next four-wheel physics step:
- front-left double wishbone;
- front-right double wishbone;
- rear-left five-link;
- rear-right five-link.

## Tests
Source regressions cover:
- fracture emits exactly once;
- full queue does not prematurely mark an event emitted;
- mount threshold crossing;
- no duplicate event at the same level;
- escalation to a higher threshold;
- deterministic sequence IDs;
- impact-energy signal;
- radiator impact route;
- radiator-support displacement route;
- unknown target ignored;
- duplicate route rejected;
- synthetic crash → structural displacement → wishbone alignment change → tire-force change;
- compiled persistent damage state → next-step suspension offset.

## Acceptance
v0.1 is accepted when structural physical state can generate deterministic typed events and functional component consequences while suspension alignment remains driven by real structural geometry rather than arbitrary damage scalars.
