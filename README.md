# Torque Atlas / Racegame

Realistic open-world driving and racing simulator focused on:
- high-fidelity vehicle dynamics;
- structural and mechanical damage;
- real geographic map data;
- free roam, racing, ownership, maintenance and tuning.

Working title: **Torque Atlas**.

## Current status

Torque Atlas is now in a **Proof-of-Physics + build-verification-readiness** phase.

The canonical four-wheel solver remains the quasi-static compliant-contact path. An explicit dynamic-unsprung corner now exists as an isolated experiment, and the vehicle/damage/telemetry content pipeline has advanced to schema/physics/damage **v2/v2/v2**.

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
- radiator puncture/leak and coolant loss;
- steering-rack authority/free-play damage;
- per-wheel hub brake/drive efficiency and bearing-drag damage;
- engine overheating/derate;
- chassis owns world gravity; vehicle-attached internal structure uses zero local gravity.

### Unsprung dynamics
- isolated explicit vertical unsprung-mass integrator exists;
- force direction, relative chassis acceleration and travel limits are tested in source;
- **not yet canonical**: current four-wheel contact still uses quasi-static tire/suspension equilibrium.

### Content pipeline
`UTAVehicleDefinition` now compiles into a runtime-ready `FTAVehicleCompiledConfig` containing:
- chassis/wheel/tire/drivetrain runtime config;
- engine/clutch/thermal/cooling calibration;
- complete front/rear axle runtime config;
- COM-local suspension hardpoints;
- compiler-derived 17-point kinematic caches;
- structural nodes/constraints/bindings/routes;
- validated geometry and calibration;
- handling/crash-critical `PhysicsConfigHash`.

Current vehicle content defaults to schema/physics/damage **2/2/2**. The fixed-step solver does not read mutable UObject data.

### Telemetry
- fixed-capacity full vehicle ring buffer;
- engine/chassis/tire channels;
- four wheel loads/travels/camber/toe;
- steering rack/Ackermann/bump steer;
- steering-rack damage/authority/free play;
- per-wheel hub damage/brake/drive/drag;
- tire temperature/pressure/wear/radial deflection;
- physics config hash;
- CSV export outside the solver step;
- scenario regression envelopes and rich min/max/steady-state CSV/JSONL reports.

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
- [Structure Damage Signal Routing](docs/29-STRUCTURE-DAMAGE-SIGNAL-ROUTING-V01.md)
- [Structural Content Schema](docs/30-STRUCTURAL-CONTENT-SCHEMA-V01.md)
- [Regression Envelope](docs/31-REGRESSION-ENVELOPE-V01.md)
- [Functional Damage & Regression](docs/32-FUNCTIONAL-DAMAGE-AND-REGRESSION-V01.md)
- [Experimental Dynamic Unsprung Corner](docs/33-EXPERIMENTAL-UNSPRUNG-CORNER-V01.md)
- [Vehicle Calibration Authoring v2](docs/34-VEHICLE-CALIBRATION-AUTHORING-V02.md)
- [UE 5.8 Verification Harness](docs/35-UE58-VERIFICATION-HARNESS-V01.md)
- [Architecture Decisions](docs/DECISIONS.md)
- [Changelog](docs/CHANGELOG.md)
- [Current Checkpoint](docs/CHECKPOINT.md)

## Verification

Static source checks:

```bash
python Scripts/source_sanity.py
```

Windows UE 5.8 build + Automation:

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\Verify-Unreal.ps1
```

Linux/macOS UE 5.8 build + Automation:

```bash
bash Scripts/verify-unreal.sh --ue-root /path/to/UE_5.8
```

Verification artifacts are written to `Saved/Verification/<timestamp>/`.

**Current status:** the harness exists, but no UE 5.8 build or Automation run has yet been executed in this environment.

## Immediate engineering order

1. run the first real UE 5.8 Development Editor build and `TorqueAtlas.*` Automation suite when a suitable toolchain is available;
2. repair UHT/UBT/compiler or Automation failures from that run before expanding the canonical physics path;
3. capture and review first real regression traces, then promote only measured ranges to trusted baselines;
4. profile the isolated dynamic-unsprung corner against the canonical compliant-contact solver before any four-corner promotion;
5. continue discrete suspension-component, fluid/electrical and brake-thermal damage work only without weakening the build gate;
6. keep world/roster/career expansion behind the Proof-of-Physics gate.

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
