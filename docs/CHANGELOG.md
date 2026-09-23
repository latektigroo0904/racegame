# Changelog

## 2026-09-23 — Integrated vehicle runtime session

Added:
- `docs/17-INTEGRATED-VEHICLE-RUNTIME-V01.md`;
- first integrated native `TA_Vehicle` fixed-step runtime;
- wheel angular dynamics with brake-to-zero clamping;
- drivetrain-to-wheel-to-tire reaction coupling;
- aggregate tire force outputs;
- open-differential split-grip integration;
- integrated vehicle Automation tests for drive torque, neutral, braking and split-mu;
- suspension runtime cache interpolation;
- spring/damper/bump/droop force model;
- static spring compression at reference ride height;
- suspension Automation tests;
- engine torque-curve interpolation;
- redline/limiter torque fade;
- engine thermal model and torque derate;
- radiator coolant-loss → engine thermal chain integration;
- integrated regression test for radiator damage causing thermal derate;
- engine starter torque;
- explicit Running/Stalled/Cranking state transitions;
- starter/stall Automation test.

Corrections:
- prevented a zero-rpm Running engine from self-starting via the torque curve;
- corrected suspension reference force so ride height includes static spring compression;
- confirmed Unreal 5.8 `TVector::GetClampedToMaxSize` API used by the structural solver.

Current limitation:
- no Unreal 5.8 compiler/UHT/UBT/runtime execution has yet been performed;
- suspension damaged-hardpoint geometric solve is not yet implemented;
- chassis 6-DOF force integration is not yet connected;
- telemetry module remains next-stage work.

## 2026-09-23 — Core solver implementation session

Added runtime modules:
- `TA_Surface`;
- `TA_Tire`;
- `TA_Structure`;
- `TA_Damage`.

Implemented:
- typed surface samples and dry/wet/ice baseline blending;
- first native tire force solver with slip ratio/angle, load sensitivity, combined slip, camber contribution, pneumatic trail, rolling resistance and continuous per-wheel aquaplaning;
- tire smoke/regression tests including worn-vs-new standing-water behaviour;
- clutch thermal/wear integration;
- gearbox ratio transforms;
- separate final-drive transforms;
- driveline torsional compliance state/helpers;
- quasi-static open-differential torque-limiting helper;
- extended powertrain automation tests;
- first XPBD-style structural distance solver;
- structural plastic rest-length change and fracture;
- structural regression tests;
- deterministic typed damage event queue;
- radiator impact, leak, coolant-loss and cooling-efficiency model;
- damage/radiator automation tests.

Documentation:
- added `16-DAMAGE-GRAPH-V01.md`;
- corrected the earlier gearbox-speed equation in `12-POWERTRAIN-SOLVER-V01.md`;
- recorded new architecture decisions.

Important limitation:
- the Unreal/C++ source still has not been compiled against an installed Unreal Engine 5.8 toolchain. Code is reviewed for API/architecture consistency but remains build-unverified.

## 2026-09-23 — Vehicle foundation and first code session
Added implementation-level specifications:
- `11-VEHICLE-DATA-SCHEMA.md`;
- `12-POWERTRAIN-SOLVER-V01.md`;
- `13-SUSPENSION-KINEMATICS-V01.md`;
- `14-PROOF-OF-PHYSICS-TEST-MATRIX.md`;
- `15-UNREAL-CPP-SKELETON.md`.

Added first Unreal project/code foundation:
- `TorqueAtlas.uproject`;
- game/editor target files;
- `TorqueAtlas` game module;
- `TorqueVehicleSimulation` plugin;
- `TA_Core`, `TA_Vehicle`, and `TA_Powertrain` runtime modules;
- native SI/unit/version/validation types;
- `UTAVehicleDefinition : UPrimaryDataAsset`;
- compiled native vehicle configuration;
- vehicle definition validation/hash path;
- initial engine rotational/friction helper functions;
- initial clutch torque-capacity helper;
- Unreal Automation smoke tests;
- Unreal-oriented `.gitignore`.

Decisions recorded:
- authored vehicle definition, persistent instance and live simulation state remain distinct;
- mutable UObject assets are compiled to native runtime configuration before high-frequency simulation;
- suspension uses cached undamaged kinematics plus runtime solving for damaged geometry;
- plugin modules are enabled incrementally as real code appears.

Important limitation:
- repository skeleton has **not yet been compiled against an installed Unreal Engine 5.8 toolchain**. Build success is not claimed.

## 2026-09-23 — Numerical simulation design session
Added:
- `08-TIRE-SOLVER-NUMERICAL.md` with initial numerical force model, combined slip, load sensitivity, thermal/pressure/wear state, wet-surface model, continuous per-wheel aquaplaning, FFB coupling and regression tests;
- `09-STRUCTURAL-SOLVER-V02.md` with iterative compliance-based solver architecture, plasticity, fracture, joint types, adaptive impact solving, structural LOD and regression tests;
- `10-GEOFORGE-DATA-SCHEMA.md` with normalized geospatial entities, provenance/confidence, hybrid-scale metadata, export units and validation gates.

Decisions recorded:
- brush-inspired semi-empirical tire solver for prototype behind backend-neutral API;
- continuous per-wheel aquaplaning;
- XPBD-style structural compliance approach;
- normalized GeoForge intermediate representation rather than raw source tags in gameplay.

## 2026-09-23 — Documentation baseline
Added canonical repository documentation for:
- project vision;
- master GDD;
- technical architecture;
- tire/surface/steering specification;
- structural/mechanical damage specification;
- open-world and GeoForge direction;
- 60-brand concept pool;
- roadmap/MVP;
- architecture decision log;
- active checkpoint.

## Earlier design work consolidated
The repository baseline incorporates prior project decisions including:
- realistic open-world driving/racing simulator vision;
- maximum 100 fictional brands;
- real-map-data world generation;
- functional soft/structural damage target;
- crash-test sandbox;
- four realism profiles;
- ownership/maintenance/tuning;
- multiplayer and traffic;
- premium/non-pay-to-win business direction;
- Unreal + custom C++ simulation architecture;
- TA-P01 prototype vehicle;
- Proof-of-Physics development strategy.
