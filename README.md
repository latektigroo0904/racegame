# Torque Atlas / Racegame

Realistic open-world driving and racing simulator focused on:
- high-fidelity vehicle dynamics;
- structural and mechanical damage;
- real geographic map data;
- free roam, racing, ownership, maintenance and tuning.

Working title: **Torque Atlas**.

## Current status

Torque Atlas is now in an **integrated front-suspension/contact physics prototype** phase.

The native simulation can already represent:

```
engine
→ clutch
→ gearbox/final drive
→ differential
→ wheel inertia
→ tire slip/force
→ force at physical contact point
→ 6-DOF chassis motion
```

For the high-fidelity front corner, the contact path can now also derive:

```
chassis pose
→ 3D double-wishbone geometry
→ road contact
→ suspension travel
→ damper motion ratio
→ normal load
→ contact-patch velocity
→ tire force
```

Structural deformation can be bound directly to suspension pickup displacement, so crash deformation can alter camber, toe, motion ratio and tire behavior without a generic suspension-health multiplier.

Tire state now includes temperature, pressure, wear, tread depth and permanent thermal degradation.

**Important:** the source has not yet been compiled or executed against an installed Unreal Engine 5.8 toolchain. Build/test success is therefore not claimed.

## Runtime modules

`Plugins/TorqueVehicleSimulation`:

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

## Current major physics systems

### Vehicle/chassis
- fixed-step native runtime;
- 6-DOF chassis;
- force-at-point torque generation;
- wheel rotational inertia;
- braking;
- drivetrain coupling.

### Front suspension/contact
- 3D double-wishbone constraint solver;
- rack/tie-rod geometry;
- camber/toe reconstruction;
- structural pickup offsets;
- damper geometry/motion ratio;
- road-plane contact/travel solve;
- self-derived vertical load;
- contact velocity from chassis motion;
- anti-roll load transfer.

### Tires
- combined longitudinal/lateral force;
- load sensitivity;
- camber contribution;
- aligning moment;
- rolling resistance;
- per-wheel aquaplaning;
- surface/carcass heat;
- pressure change;
- temperature/pressure grip;
- wear and tread loss;
- thermal degradation.

### Damage
- structural nodes/constraints;
- plasticity/fracture baseline;
- reference-vs-current node displacement;
- weighted suspension-pickup bindings;
- radiator puncture/leak;
- coolant loss;
- engine overheating/derate.

### Telemetry
- fixed-capacity ring buffer;
- vehicle/tire/thermal channels;
- reserved profiling channels.

## Canonical documentation

- [Project Canon](docs/00-PROJECT-CANON.md)
- [Master GDD](docs/01-GDD-MASTER.md)
- [Technical Architecture](docs/02-TDD-ARCHITECTURE.md)
- [Tire, Surface & Steering](docs/03-PHYSICS-TIRE-SURFACE-STEERING.md)
- [Structural & Mechanical Damage](docs/04-DAMAGE-STRUCTURE.md)
- [Open World & GeoForge](docs/05-WORLD-GEOFORGE.md)
- [Fictional Brands](docs/06-BRANDS.md)
- [Roadmap & MVP](docs/07-ROADMAP-MVP.md)
- [Numerical Tire Solver](docs/08-TIRE-SOLVER-NUMERICAL.md)
- [Structural Solver v0.2](docs/09-STRUCTURAL-SOLVER-V02.md)
- [GeoForge Data Schema](docs/10-GEOFORGE-DATA-SCHEMA.md)
- [Vehicle Data Schema](docs/11-VEHICLE-DATA-SCHEMA.md)
- [Powertrain Solver v0.1](docs/12-POWERTRAIN-SOLVER-V01.md)
- [Suspension Kinematics v0.1](docs/13-SUSPENSION-KINEMATICS-V01.md)
- [Proof-of-Physics Test Matrix](docs/14-PROOF-OF-PHYSICS-TEST-MATRIX.md)
- [Unreal/C++ Skeleton](docs/15-UNREAL-CPP-SKELETON.md)
- [Functional Damage Graph](docs/16-DAMAGE-GRAPH-V01.md)
- [Integrated Vehicle Runtime](docs/17-INTEGRATED-VEHICLE-RUNTIME-V01.md)
- [Chassis Dynamics](docs/18-CHASSIS-DYNAMICS-V01.md)
- [Telemetry](docs/19-TELEMETRY-V01.md)
- [Double-Wishbone Geometry v0.2](docs/20-DOUBLE-WISHBONE-GEOMETRY-V02.md)
- [Suspension/Road Contact Pipeline](docs/21-CONTACT-LOAD-PIPELINE-V01.md)
- [Tire Thermal/Wear](docs/22-TIRE-THERMAL-WEAR-V01.md)
- [Architecture Decisions](docs/DECISIONS.md)
- [Changelog](docs/CHANGELOG.md)
- [Current Checkpoint](docs/CHECKPOINT.md)

## Immediate engineering order

1. mirrored front-left suspension definition;
2. driver steering input → steering-rack displacement;
3. complete front-axle left/right contact + anti-roll solve;
4. true rear multi-link geometry/contact solver;
5. full four-wheel self-support/static-settle fixture;
6. tire vertical compliance and unsprung mass;
7. structural collision impulse distribution;
8. first actual Unreal Engine 5.8 compile/test run when the toolchain is available.

## Correctness rules

- no global vehicle HP drives physics;
- no arbitrary damaged-camber/toe multiplier when geometry exists;
- no per-car handling branches in solver code;
- no mutable UObject reads in high-frequency native solving;
- physical forces act at physical locations;
- high-fidelity load/speed should derive from chassis/contact state;
- telemetry observes but never changes physics;
- calibration seeds remain provisional until validated;
- never claim build/test success before actual execution.

## Core development rule

**Driving quality first. Damage consequences second. World scale after the foundation works.**

## Repository policy

This repository is the canonical project record. Significant design and engineering decisions are committed here rather than existing only in chat.
