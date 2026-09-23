# Structural & Mechanical Damage Specification — v0.1

## Goal
A crash changes how the vehicle works. Visual deformation is secondary to correct mechanical consequence.

## Architecture
Two linked layers:

### Structural solver
Represents chassis/body load paths.

### Mechanical damage graph
Maps structural deformation/impact/heat/leaks to vehicle-system failures.

## Structural model
Prototype target:
- ~350–500 important nodes;
- ~1,500–3,000 constraints.

Nodes represent chassis rails, pillars, floor, roof, subframes, crash structures and suspension mounts.

## Constraint data
```
NodeA
NodeB
RestLength
Stiffness
Damping
YieldStrain
BreakStrain
Plasticity
MaterialID
Fatigue/Damage
```

States:
```
Elastic → Yielding → Plastic → Broken
```

Strain:
```
(CurrentLength - RestLength) / RestLength
```

Above yield, rest geometry can permanently change.
Above fracture threshold, the connection breaks.

## Materials
Separate profiles for:
- mild/structural steel;
- high-strength steel;
- aluminium;
- carbon composite;
- plastics;
- glass;
- rubber.

Profiles contain density, stiffness, yield behaviour, ultimate strength, fracture strain, damping and plasticity/failure style.

## Structural zones
- front crash zone;
- front subframe;
- passenger cell;
- floor;
- roof;
- doors;
- rear crash zone;
- rear subframe.

Passenger cell should generally preserve shape better than sacrificial crash zones.

## Mounts/joints
Represent:
- welds;
- bolts;
- adhesive joints;
- hinges;
- latches;
- bushings.

This enables panels and mechanical assemblies to partially detach or fail.

## Damageable systems
- wheels/tires;
- hubs;
- control arms;
- steering rack/tie rods;
- brakes and hydraulic lines;
- radiator/cooling;
- oil system;
- fuel system;
- engine;
- clutch;
- gearbox;
- driveshafts/CV joints;
- differentials;
- exhaust;
- lights;
- electrical systems.

## No global car HP
Vehicle state is distributed:
```
Radiator 22%
Engine block 95%
Oil pan 40%
Left control arm 31%
Steering rack 78%
Transmission 92%
```

## Functional status
```
Driveable
Limping
Critical
Immobilized
EconomicTotalLoss
```

## Example front-left impact
1. bumper structure loads;
2. front rail yields;
3. radiator support moves;
4. lower-arm pickup shifts;
5. toe/camber changes;
6. radiator punctures;
7. coolant quantity drops;
8. steering pulls;
9. tire scrub/heat increases;
10. coolant temperature rises;
11. engine protection reduces power;
12. prolonged driving can seize the engine.

## Suspension damage
A curb strike can bend:
- rim;
- knuckle;
- arm;
- tie rod;
- subframe mount.

Alignment must be recalculated from damaged geometry rather than applying an arbitrary penalty.

## Fluids/leaks
Track:
- coolant;
- engine oil;
- transmission fluid;
- brake fluid;
- fuel where relevant.

Leak rate depends on damaged component and opening severity.

## Visual deformation
Structural nodes drive a higher-resolution deformation cage that deforms the render mesh.

Specialized solutions can handle:
- glass cracking;
- scratches;
- paint loss;
- detachable trim;
- debris.

Not every visual element needs full structural simulation.

## Crash sandbox
Presets:
- frontal;
- offset;
- overlap;
- side;
- pole;
- rear;
- rollover;
- barrier;
- ramp/drop/high speed.

Telemetry:
- impact speed;
- relative velocity;
- peak deceleration;
- structural deformation;
- broken constraints;
- wheel alignment;
- temperatures;
- component damage.

## Regression testing
Repeat identical vehicle/barrier/speed/angle setup and compare:
- deformation;
- broken constraints;
- alignment;
- cooling damage;
- functional state.

The solver must be stable enough for tuning and multiplayer reconciliation.
