# Torque Atlas / Racegame

Realistic open-world driving and racing simulator focused on:
- high-fidelity vehicle dynamics;
- structural and mechanical damage;
- real geographic map data;
- free roam, racing, ownership, maintenance and tuning.

Working title: **Torque Atlas**.

## Current status

The project is now in an **integrated native vehicle-motion prototype** phase.

The repository contains a UE 5.8-targeted project skeleton plus first native implementations for:
- surface and wet/ice state;
- tire forces and per-wheel aquaplaning;
- engine/clutch/gearbox/final-drive/open-differential physics;
- starter/stall state;
- cooling/engine thermal derate;
- wheel rotational dynamics and braking;
- suspension cache + spring/damper forces;
- structural XPBD-style deformation/fracture baseline;
- functional radiator damage;
- 6-DOF chassis integration;
- tire/suspension forces applied at physical wheel contact points;
- telemetry ring buffer;
- Unreal Automation test source.

**Important:** source has not yet been compiled or executed against an installed Unreal Engine 5.8 toolchain, so build/test success is not claimed.

## Runtime modules

`Plugins/TorqueVehicleSimulation` currently contains/enables:

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

## Current native simulation path

```
driver controls
→ engine
→ clutch
→ gearbox
→ final drive
→ open differential
→ wheel inertia
→ tire slip/force
→ force at wheel contact point
→ chassis force/torque
→ 6-DOF chassis motion
```

Damage path:

```
radiator impact
→ puncture/leak
→ coolant loss
→ cooling loss
→ temperature rise
→ engine torque derate/damage
```

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
- [Architecture Decisions](docs/DECISIONS.md)
- [Changelog](docs/CHANGELOG.md)
- [Current Checkpoint](docs/CHECKPOINT.md)

## Immediate engineering order

1. implement TA-P01 double-wishbone hardpoint schema;
2. solve suspension travel and vertical contact load from geometry;
3. derive wheel contact velocity from chassis motion;
4. connect damaged structural pickup positions to suspension geometry;
5. remove externally supplied contact load/velocity from the canonical vehicle fixture;
6. add telemetry export/regression comparison;
7. compile and execute against Unreal Engine 5.8 when a suitable build environment is available;
8. prove the vehicle can support its own weight, accelerate, brake, corner, crash and continue damaged.

## Correctness rules

- no global vehicle HP drives physics;
- no per-car constants buried in solver branches;
- no mutable UObject access in high-frequency solver loops;
- fixed-step simulation;
- physical forces act at physical locations;
- structural displacement changes geometry where possible;
- telemetry observes but does not change physics;
- calibration seeds remain provisional until validated;
- never claim build/test success before actual execution.

## Core development rule

**Driving quality first. Damage consequences second. World scale after the foundation works.**

## Repository policy

This repository is the canonical project record. Significant design and engineering decisions are committed here rather than existing only in chat.
