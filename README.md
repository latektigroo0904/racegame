# Torque Atlas / Racegame

Realistic open-world driving and racing simulator focused on:
- high-fidelity vehicle dynamics;
- structural and mechanical damage;
- real geographic map data;
- free roam, racing, ownership, maintenance and tuning.

Working title: **Torque Atlas**.

## Current status

Torque Atlas is now in a **four-wheel compliant vehicle-physics + collision/structure coupling prototype** phase.

The canonical high-fidelity proving-ground chain is:

```
versioned UTAVehicleDefinition
→ validation + compiled immutable physics config
→ driver controls
→ engine / clutch / gearbox / final drive / differential
→ four wheel rotational states
→ front double-wishbone + steering rack
→ rear five-link multi-link
→ tire/suspension/anti-roll equilibrium
→ radial tire compliance
→ tire longitudinal/lateral forces
→ forces/moments at physical contact points
→ 6-DOF chassis motion
```

Collision baseline:

```
external collision impulse
→ chassis Δv / Δω exactly once
→ chassis-local structural impact field
→ remove rigid translation/rotation modes
→ internal structural deformation velocities
→ XPBD structure / plasticity / fracture
→ damaged suspension pickup geometry
```

The project remains **build-unverified** until the source is compiled and Automation tests are run against an installed Unreal Engine 5.8 toolchain.

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
- native fixed-step runtime;
- 6-DOF chassis;
- body principal inertia and gyroscopic term;
- force-at-point torque;
- instantaneous collision impulse-at-point response;
- wheel rotational inertia;
- brake-to-zero integration;
- RWD/FWD prototype drive selection through compiled data.

### Front axle
- 3D double-wishbone constraint solver;
- mirrored left/right geometry;
- one shared physical steering rack;
- geometric steering rather than commanded wheel yaw;
- Ackermann delta;
- bump-steer telemetry;
- damaged pickup offsets;
- geometric damper motion ratio;
- coupled anti-roll/tire equilibrium.

### Rear axle
- true five-link rigid-upright solver;
- five independent chassis/upright links;
- mirrored left/right geometry;
- camber/toe from geometry;
- structural pickup offsets;
- geometric damper response;
- coupled rear anti-roll/tire equilibrium.

### Tires
- combined longitudinal/lateral force;
- load sensitivity;
- camber contribution;
- aligning moment applied to chassis;
- rolling resistance;
- continuous per-wheel aquaplaning;
- surface/carcass/internal-air temperature;
- pressure change;
- wear and tread loss;
- thermal degradation;
- pressure-dependent radial stiffness;
- radial damping/progressive stiffness;
- finite radial deflection and bottoming detection.

### Four-wheel contact
- self-derived FL/FR/RL/RR contact;
- suspension travel from road/chassis geometry;
- chassis point velocity → tire patch velocity;
- self-derived wheel normal load;
- compliant loaded tire radius;
- front/rear anti-roll solved inside vertical equilibrium;
- rigid-radius contact retained as lower-fidelity/test path.

### Structure/damage
- structural nodes with reference positions;
- XPBD-style compliant distance constraints;
- plastic rest-state change;
- fracture;
- weighted structural-node → suspension-pickup bindings;
- momentum-neutral spatial impact distribution;
- radiator puncture/leak;
- coolant loss;
- engine overheating/derate.

### Unsprung dynamics
- isolated explicit vertical unsprung-mass integrator exists;
- force direction, relative chassis acceleration and travel limits are tested in source;
- **not yet canonical**: current four-wheel contact still uses quasi-static tire/suspension equilibrium.

### Content pipeline
`UTAVehicleDefinition` now compiles into a runtime-ready `FTAVehicleCompiledConfig` containing:
- chassis/wheel/tire/drivetrain runtime config;
- complete front/rear axle runtime config;
- COM-local suspension hardpoints;
- validated geometry;
- handling-critical `PhysicsConfigHash`.

The fixed-step solver does not read mutable UObject data.

### Telemetry
- fixed-capacity ring buffer;
- engine/chassis/tire channels;
- four wheel loads/travels/camber/toe;
- steering rack/Ackermann/bump steer;
- tire temperature/pressure/wear/radial deflection;
- physics config hash;
- CSV export outside the solver step.

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
- [Front Axle / Steering](docs/23-FRONT-AXLE-STEERING-V01.md)
- [Rear Multi-Link](docs/24-REAR-MULTILINK-GEOMETRY-V01.md)
- [Tire Vertical Compliance](docs/25-TIRE-VERTICAL-COMPLIANCE-V01.md)
- [Unsprung Vertical Dynamics](docs/26-UNSPRUNG-VERTICAL-DYNAMICS-V01.md)
- [Vehicle Content Compilation](docs/27-VEHICLE-CONTENT-COMPILATION-V01.md)
- [Collision / Structure Coupling](docs/28-COLLISION-STRUCTURE-COUPLING-V01.md)
- [Architecture Decisions](docs/DECISIONS.md)
- [Changelog](docs/CHANGELOG.md)
- [Current Checkpoint](docs/CHECKPOINT.md)

## Immediate engineering order

1. turn structural fracture/displacement results into deterministic typed damage signals;
2. route collision/structure outputs into suspension/radiator/mechanical damage consumers;
3. add authored structural node/constraint + damage-binding content compilation;
4. promote explicit unsprung dynamics into one experimental high-fidelity corner without double-counting tire/chassis normal forces;
5. add static-settle/step-road regression envelopes and telemetry comparison;
6. complete remaining tire/powertrain authoring fields;
7. perform the first actual Unreal Engine 5.8 compile and Automation run when a suitable toolchain is available.

## Correctness rules

- no global vehicle HP drives physics;
- no arbitrary damaged-camber/toe multiplier when geometry exists;
- no per-car handling branches in solver code;
- no mutable UObject reads in high-frequency native solving;
- physical forces/impulses act at physical locations;
- high-fidelity contact load/speed derive from chassis/contact state;
- tire normal load must agree with radial tire deformation in the compliant path;
- collision rigid momentum is applied exactly once;
- structural deformation excitation must not add duplicate rigid momentum;
- telemetry observes but never changes physics;
- calibration seeds remain provisional until validated;
- never claim build/test success before actual execution.

## Core development rule

**Driving quality first. Damage consequences second. World scale after the foundation works.**

## Repository policy

This repository is the canonical project record. Significant design and engineering decisions are committed here rather than existing only in chat.
