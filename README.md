# Torque Atlas / Racegame

Realistic open-world driving and racing simulator focused on:
- high-fidelity vehicle dynamics;
- structural and mechanical damage;
- real geographic map data;
- free roam, racing, ownership, maintenance and tuning.

Working title: **Torque Atlas**.

## Current status

The project is now in an **early Unreal/C++ physics prototype foundation** phase.

The repository contains a UE 5.8-targeted project skeleton plus first native implementations for:
- core simulation/versioning;
- vehicle definitions and compiled runtime configuration;
- road/surface state;
- tire forces and aquaplaning;
- engine/clutch/gearbox/final-drive/differential helpers;
- structural compliant constraints, plasticity and fracture;
- typed functional damage events;
- radiator puncture/leak/cooling degradation;
- Unreal Automation test sources.

**Important:** the source has not yet been compiled against an installed Unreal Engine 5.8 toolchain, so build success is not claimed.

## Runtime modules

`Plugins/TorqueVehicleSimulation` currently enables:

```
TA_Core
TA_Surface
TA_Tire
TA_Powertrain
TA_Structure
TA_Damage
TA_Vehicle
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
- [Architecture Decisions](docs/DECISIONS.md)
- [Changelog](docs/CHANGELOG.md)
- [Current Checkpoint](docs/CHECKPOINT.md)

## Immediate engineering order

1. build the integrated `TA_Vehicle` fixed-step runtime;
2. add wheel rotational dynamics and brake torque;
3. add suspension runtime geometry and damaged pickup coupling;
4. finish engine torque-map / idle / starter / turbo / over-rev state;
5. connect structure → radiator → cooling → engine thermal consequences;
6. add telemetry;
7. compile and run the full test set against Unreal 5.8 when a suitable build environment is available;
8. prove one complete vehicle before scaling world/content.

## Correctness rules

- no global vehicle HP drives physics;
- no per-car constants buried in solver code;
- no mutable UObject access in high-frequency solver loops;
- use fixed-step simulation;
- structural displacement should alter geometry directly where possible;
- all calibration seeds remain provisional until validated;
- never claim a build/test pass before it has actually run.

## Core development rule

**Driving quality first. Damage consequences second. World scale after the foundation works.**

## Repository policy

This repository is the canonical project record. Significant design and engineering decisions are committed here rather than existing only in chat.
