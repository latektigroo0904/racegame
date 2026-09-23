# Torque Atlas / Racegame

Realistic open-world driving and racing simulator focused on:
- high-fidelity vehicle dynamics;
- structural and mechanical damage;
- real geographic map data;
- free roam, racing, ownership, maintenance and tuning.

Working title: **Torque Atlas**.

## Current status

The project has moved from pure design into its **first Unreal/C++ prototype foundation**.

The repository now contains:
- a UE 5.8-targeted project descriptor;
- game/editor targets;
- a custom `TorqueVehicleSimulation` plugin;
- initial `TA_Core`, `TA_Vehicle`, and `TA_Powertrain` modules;
- a data-driven `UTAVehicleDefinition`;
- native compiled runtime configuration;
- initial powertrain math;
- automation smoke tests.

**Important:** the skeleton has not yet been compiled against an installed Unreal Engine 5.8 toolchain, so build success is not claimed.

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
- [Architecture Decisions](docs/DECISIONS.md)
- [Changelog](docs/CHANGELOG.md)
- [Current Checkpoint](docs/CHECKPOINT.md)

## Immediate work order

1. finish native powertrain v0.1;
2. add the native tire runtime module;
3. add suspension runtime geometry/state;
4. add surface contact runtime module;
5. expand automated physics regression tests;
6. compile and verify against Unreal 5.8 when a suitable build environment is available;
7. prove one complete vehicle before scaling world/content.

## Core development rule

**Driving quality first. Damage consequences second. World scale after the foundation works.**

## Repository policy

This repository is the canonical project record. Significant design and engineering decisions are committed here rather than existing only in chat.
