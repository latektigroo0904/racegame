# Changelog

## 2026-09-23 — Four-wheel compliant contact, content compilation and collision coupling session

Added:
- `docs/23-FRONT-AXLE-STEERING-V01.md`;
- `docs/24-REAR-MULTILINK-GEOMETRY-V01.md`;
- `docs/25-TIRE-VERTICAL-COMPLIANCE-V01.md`;
- `docs/26-UNSPRUNG-VERTICAL-DYNAMICS-V01.md`;
- `docs/27-VEHICLE-CONTENT-COMPILATION-V01.md`;
- `docs/28-COLLISION-STRUCTURE-COUPLING-V01.md`;
- shared physical front steering rack runtime;
- mirrored front-left double-wishbone geometry;
- Ackermann and bump-steer outputs;
- true five-link rear rigid-upright geometry solver;
- rear road-contact resolver and rear axle runtime;
- complete FL/FR/RL/RR four-wheel runtime;
- canonical four-wheel self-derived contact path;
- tire radial spring/damper/progressive stiffness model;
- pressure-dependent tire radial stiffness;
- tire bottoming state;
- compliant tire/suspension equilibrium solver;
- coupled anti-roll/tire equilibrium for front and rear axles;
- isolated unsprung vertical-mass integrator;
- authored vehicle physics compiled into immutable runtime-ready configs;
- COM-local hardpoint transformation during vehicle compilation;
- handling-critical physics config hashing;
- end-to-end vehicle asset → compiled config → four-wheel physics test;
- four-wheel suspension/tire telemetry and CSV export;
- physics config hash + tire radial deflection telemetry;
- structural impact distributor with spatial weighting;
- rigid translation/rotation mode removal from structural impact excitation;
- instantaneous chassis impulse-at-point response;
- chassis/structure collision coupling that avoids rigid momentum double counting.

New/expanded Automation source covers:
- front geometry mirroring and shared rack axis;
- rack input clamping and steering sign;
- Ackermann behavior;
- front/rear asymmetric-road anti-roll;
- rear five-link reference/bump/droop/damage behavior;
- four-wheel self-support;
- four-wheel static stability;
- four-wheel acceleration;
- four-wheel braking;
- steering-generated yaw;
- tire radial compliance, pressure effect, damping and bottoming;
- anti-roll load represented by matching compliant tire deflection;
- unsprung force direction/reference-frame/travel limits;
- vehicle compiled-config hash sensitivity;
- degenerate authored suspension rejection;
- COM-local hardpoint compilation;
- compiled asset executing the canonical four-wheel runtime;
- telemetry CSV/config-hash/radial-deflection mapping;
- structural impact spatial weighting;
- zero duplicate structural linear/angular rigid momentum;
- structural impact velocity limiting;
- chassis center/off-center impulse response;
- full chassis impulse plus internal deformation coupling.

Corrections / consistency:
- `TA_Telemetry` now declares its public `TA_Vehicle` dependency;
- tire aligning moment contributes to chassis torque;
- front/rear compliant anti-roll is solved inside force equilibrium rather than post-applied;
- internal contact-solver probes do not commit tire/suspension history;
- a telemetry CSV guard regression found by source review was repaired before checkpoint;
- prototype static spring preload was rebalanced around the current symmetric COM/axle seed rather than hiding front/rear weight distribution in spring bias.

Still unverified:
- Unreal Engine 5.8 UHT/UBT/C++ compilation;
- Automation execution;
- actual numerical convergence of all coupled source tests in UE;
- runtime/profile budgets;
- cross-machine determinism;
- calibrated real-world tire/suspension/impact parameters.

## 2026-09-23 — Suspension geometry, contact-load and tire-state session

Added:
- `docs/20-DOUBLE-WISHBONE-GEOMETRY-V02.md`;
- `docs/21-CONTACT-LOAD-PIPELINE-V01.md`;
- `docs/22-TIRE-THERMAL-WEAR-V01.md`;
- full 3D front double-wishbone positional constraint solver;
- physical upper/lower wishbone inner pickups;
- tie-rod steering constraint;
- rigid upright reconstruction;
- wheel center/orientation reconstruction;
- solved camber/toe from geometry;
- chassis damper mount + lower-arm damper mount;
- damaged-mode damper length and finite-difference motion-ratio estimation;
- road-plane travel/contact resolver;
- automatic full-droop contact loss;
- automatic full-bump/penetration reporting;
- contact-patch velocity from chassis linear/angular velocity;
- self-derived vertical load from suspension force;
- conversion from resolved contact into `FTAWheelContactInput`;
- structure-node reference positions;
- weighted structure-node → suspension-pickup bindings;
- anti-roll bar equal/opposite axle load transfer;
- tire surface/carcass/internal-air thermal model;
- temperature-dependent tire grip;
- pressure-dependent tire grip;
- ideal-gas pressure update;
- energy-based tire wear and tread-depth loss;
- irreversible tire thermal degradation;
- tire thermal/wear update in integrated vehicle step;
- tire aligning moment now applied to chassis torque.

New/expanded Automation test source covers:
- reference wishbone geometry;
- bump travel;
- rack displacement changing toe;
- damaged pickup changing camber;
- degenerate geometry rejection;
- reference road contact and self-derived normal load;
- chassis velocity → contact velocity;
- full-droop contact loss;
- damaged contact alignment;
- structural pickup binding and weighted displacement;
- anti-roll transfer;
- resolved suspension contact → tire → chassis integration;
- tire heat transfer, pressure rise, wear/tread loss and temperature grip window.

Corrections / consistency:
- one-dimensional undamaged kinematic cache is now invalidated while steering or when structural pickup displacement exists;
- road surface state is retained through the contact resolver into the tire input;
- damaged-mode motion ratio no longer assumes 1.0;
- tire aligning moment is no longer telemetry-only and contributes to chassis torque.

Validation note:
- the selected TA-P01 front-right seed geometry was numerically sanity-checked across reference travel, bump/droop, steering-rack displacement and pickup displacement before being committed as a test fixture;
- current hardpoints are engineering seeds, not measured production-car geometry.

Current limitation:
- Unreal Engine 5.8 UHT/UBT/C++ build and Automation execution are still not available in this environment;
- front-left production definition and driver steering-rack mapping are still pending;
- rear multi-link geometry/contact remains pending;
- tire vertical compliance and unsprung mass are not yet implemented.

## 2026-09-23 — Chassis, suspension runtime and telemetry session

Added:
- `docs/18-CHASSIS-DYNAMICS-V01.md`;
- `docs/19-TELEMETRY-V01.md`;
- native 6-DOF chassis state/integrator;
- force-at-point and torque accumulation;
- body-axis inertia and gyroscopic term;
- quaternion orientation integration;
- chassis regression tests for centered force, off-center force and pure torque;
- wheel contact world positions/directions in integrated vehicle input;
- tire + suspension reaction forces applied at physical contact points;
- integrated tests for tire-force-driven chassis motion and asymmetric-grip yaw;
- fixed-capacity telemetry ring buffer;
- vehicle telemetry capture for engine, clutch, coolant, chassis and four-wheel tire channels;
- telemetry wraparound/reset Automation test.

Also refined:
- integrated engine starter/stall state;
- suspension static preload/reference ride-height support;
- radiator → cooling → engine thermal derate integration.

Verification:
- Unreal 5.8 documentation was checked for `TVector::GetClampedToMaxSize`, quaternion axis-angle construction, normalization and inverse vector rotation APIs used by the prototype.

Current limitation:
- no real UE 5.8 UHT/UBT/C++ build or Automation execution has yet been performed;
- vertical contact load is still externally supplied to the tire solver;
- damaged-hardpoint geometric suspension solve remains incomplete;
- collision/manifold-to-structure impulse transfer is not connected.

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
