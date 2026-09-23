# Architecture & Design Decision Log

## ADR-001 — Unreal Engine
**Status:** accepted  
**Decision:** use Unreal Engine 5.x rather than Unity or a fully proprietary engine.  
**Reason:** rendering/world tooling/editor/platform support reduce non-core engineering burden.

## ADR-002 — Custom vehicle simulation
**Status:** accepted  
**Decision:** custom C++ Vehicle Dynamics Core for player/high-fidelity vehicles.  
**Reason:** target tire, drivetrain and damage fidelity exceeds the intended lightweight scope of generic vehicle components.

## ADR-003 — Custom structural damage solver
**Status:** accepted  
**Decision:** node/constraint structural representation coupled to a visual deformation cage.  
**Reason:** mechanical consequences require persistent geometry changes and breakable load paths.

## ADR-004 — Functional damage over cosmetic damage
**Status:** accepted  
**Decision:** every major damage category must be capable of changing vehicle function.

## ADR-005 — Hybrid geographic scale
**Status:** accepted  
**Decision:** use 1:1 only where it adds value; compress low-value transit areas.

## ADR-006 — GeoForge
**Status:** accepted  
**Decision:** build a reusable map-data import/generation pipeline instead of hand-authoring the entire world.

## ADR-007 — Data-driven vehicles
**Status:** accepted  
**Decision:** no per-car physics constants embedded in code; use versioned definitions/assets.

## ADR-008 — Fixed-step simulation
**Status:** accepted  
**Decision:** core vehicle solver runs on fixed steps independent of render rate.

## ADR-009 — Physics LOD
**Status:** accepted  
**Decision:** only nearby/relevant vehicles receive high-fidelity physics/damage.

## ADR-010 — Multiplayer damage split
**Status:** provisional accepted  
**Decision:** server authoritative functional damage; detailed visual deformation reconstructed/predicted locally with periodic correction.

## ADR-011 — Prototype before content scale
**Status:** accepted  
**Decision:** one excellent car and one proving ground before large maps or a broad vehicle roster.

## ADR-012 — Repository as canonical record
**Status:** accepted 2026-09-23  
**Decision:** `latektigroo0904/racegame` is the main versioned repository for project documentation and future code unless explicitly changed.

## ADR-013 — Initial tire solver family
**Status:** accepted for prototype  
**Decision:** start with an interpretable brush-inspired semi-empirical tire model behind a backend-neutral API, rather than making a full Magic Formula implementation a prerequisite.  
**Reason:** faster calibration, better debugging and easier low-speed/combined-slip stability while preserving a future measured-data backend.

## ADR-014 — Aquaplaning is continuous and per wheel
**Status:** accepted  
**Decision:** standing water creates a continuous hydrodynamic support fraction per tire; it is not a binary global weather threshold.  
**Reason:** split puddles, tread wear, pressure and load must produce local vehicle reactions.

## ADR-015 — XPBD-style structural compliance
**Status:** accepted for prototype  
**Decision:** use an iterative position/compliance-based structural constraint solver with semi-implicit integration rather than very stiff explicit springs.  
**Reason:** high structural stiffness needs practical timesteps and stable crash solving.

## ADR-016 — GeoForge normalized intermediate representation
**Status:** accepted  
**Decision:** source map formats are converted into a stable GeoForge schema before Unreal export. Raw OSM/source tags are not gameplay APIs.  
**Reason:** source independence, regeneration, validation, licensing provenance and regional rule packs.

## ADR-017 — Definition, instance and live state are separate domains
**Status:** accepted  
**Decision:** vehicle authored definition, persistent owned-vehicle state and high-frequency simulation state are separate data models.  
**Reason:** prevents save/configuration concerns from contaminating the deterministic solver and makes migrations explicit.

## ADR-018 — Compile Unreal assets into native runtime configuration
**Status:** accepted  
**Decision:** `UPrimaryDataAsset` vehicle data is validated and compiled into native immutable runtime configuration before simulation. The high-frequency solver does not read mutable UObjects.  
**Reason:** threading safety, performance, deterministic testing and cleaner multiplayer/replay hashing.

## ADR-019 — Hybrid suspension kinematics
**Status:** accepted for prototype  
**Decision:** undamaged suspension may use a precomputed kinematic lookup derived from hardpoints; damaged/modified geometry switches to runtime geometric solving.  
**Reason:** preserves performance while allowing structural pickup displacement to produce real alignment changes.

## ADR-020 — Incremental plugin-module activation
**Status:** accepted  
**Decision:** Unreal plugin modules are added to the plugin descriptor only when their code/dependencies exist.  
**Reason:** maintain a buildable repository skeleton instead of committing placeholder module references that immediately break UBT discovery.

## ADR-021 — Conventional reduction-ratio semantics
**Status:** accepted 2026-09-23  
**Decision:** gearbox/final-drive ratios use the conventional reduction definition `G = omega_input / omega_output`; speed divides by ratio and torque multiplies by ratio times efficiency.  
**Reason:** prevents ambiguous drivetrain math and keeps telemetry, part swapping and over-rev calculations consistent.

## ADR-022 — Functional damage uses typed events, not global HP
**Status:** accepted 2026-09-23  
**Decision:** structural/contact/thermal/wear systems emit deterministic typed damage signals routed to component-specific models. No single vehicle-health scalar drives physics.  
**Reason:** a radiator, steering rack, brake line and gearbox fail through different physical state and consequences.

## ADR-023 — Structural prototype begins with XPBD distance constraints
**Status:** accepted for prototype 2026-09-23  
**Decision:** first executable structure module implements compliant distance constraints, plastic rest-state change and fracture before adding bending/joint families.  
**Reason:** establishes numerical stability and regression tests before increasing structural complexity.

## ADR-024 — Integrated vehicle solver remains engine-agnostic
**Status:** accepted 2026-09-23  
**Decision:** the high-frequency vehicle step is a pure native solver that consumes compiled runtime configuration and explicit wheel-contact inputs, and returns forces/state without applying them directly to an Unreal rigid body.  
**Reason:** deterministic testing, threading, future alternative chassis backends and clean separation from engine-side scene queries.

## ADR-025 — Wheel braking clamps toward zero
**Status:** accepted for prototype 2026-09-23  
**Decision:** brake torque is integrated as an angular impulse that reduces wheel speed toward zero and is bounded so numerical overshoot cannot reverse wheel rotation.  
**Reason:** avoids unphysical sign-flip chatter near standstill.

## ADR-026 — Engine restart requires state transition and starter torque
**Status:** accepted 2026-09-23  
**Decision:** combustion torque is available only in the Running state. Stalled/stopped engines require finite starter torque and sufficient cranking speed before returning to Running.  
**Reason:** prevents zero-rpm engines from self-starting through the torque curve.

## ADR-027 — Suspension reference state includes static spring compression
**Status:** accepted 2026-09-23  
**Decision:** reference ride height is not zero spring force; spring force includes authored static compression/preload plus dynamic travel through motion ratio.  
**Reason:** a stationary vehicle must be statically supported at its reference ride height.

## ADR-028 — Native chassis solver owns 6-DOF reference motion
**Status:** accepted for prototype 2026-09-23  
**Decision:** vehicle translation and rotation are integrated in a pure native chassis solver using explicit force/torque accumulation, body principal inertia and quaternion orientation.  
**Reason:** tire, suspension and aero forces must have a deterministic engine-agnostic motion target before any Unreal rigid-body wrapper is chosen.

## ADR-029 — Tire and suspension forces are applied at physical world points
**Status:** accepted 2026-09-23  
**Decision:** each wheel contact supplies contact position and basis directions; tire and suspension reaction forces are applied at that point to generate both chassis force and torque.  
**Reason:** asymmetric grip and load must create pitch/roll/yaw through lever arms, not scripted yaw multipliers.

## ADR-030 — Telemetry observes but never drives physics
**Status:** accepted 2026-09-23  
**Decision:** `TA_Telemetry` depends on public vehicle state and captures samples into a fixed-capacity ring buffer; the vehicle solver does not depend on telemetry.  
**Reason:** logging/regression instrumentation must not become part of simulation truth or introduce solver-side allocations.

## ADR-031 — Damaged front suspension uses a 3D constraint solve
**Status:** accepted for prototype 2026-09-23  
**Decision:** front double-wishbone runtime geometry is solved from physical upper/lower inner pickups, ball joints, tie rod and a rigid upright constraint set, with wheel travel as the remaining mechanism coordinate.  
**Reason:** structural pickup displacement and steering must alter alignment through geometry rather than artificial camber/toe damage scalars.

## ADR-032 — Motion ratio comes from current suspension geometry
**Status:** accepted for prototype 2026-09-23  
**Decision:** a chassis damper mount and lower-arm damper mount are part of the suspension geometry. The damaged-mode motion ratio is estimated from the local derivative of damper length with respect to wheel travel.  
**Reason:** a constant 1.0 motion ratio becomes physically inconsistent when arms or chassis mounts move.

## ADR-033 — High-fidelity contact derives load and velocity
**Status:** accepted for front prototype 2026-09-23  
**Decision:** the canonical high-fidelity front path derives road reach, suspension travel, normal load, contact position and patch velocity from chassis/suspension/road state. Manually supplied VerticalLoadN and contact velocity remain test/LOD interfaces only.  
**Reason:** weight transfer and damaged geometry must emerge from the same physical state used by the tire solver.

## ADR-034 — Structural node displacement directly drives suspension pickup displacement
**Status:** accepted 2026-09-23  
**Decision:** structure nodes retain reference positions; compiled weighted bindings map current-minus-reference node displacement into suspension pickup offsets.  
**Reason:** crash deformation can therefore change alignment without a generic suspension-damage percentage.

## ADR-035 — Tire grip state is thermal, pressure and wear dependent
**Status:** accepted for prototype 2026-09-23  
**Decision:** slip/rolling energy drives a two-layer tire thermal model, internal-air temperature drives pressure, dissipated energy drives wear/tread loss, and effective friction includes reversible temperature/pressure effects plus permanent wear/thermal degradation.  
**Reason:** tire behavior must evolve during use and wet-weather behavior must depend on actual tread state.

## ADR-036 — Anti-roll bars transfer equal/opposite axle load
**Status:** accepted for prototype 2026-09-23  
**Decision:** left/right suspension travel difference creates bounded equal/opposite normal-load adjustments at an axle.  
**Reason:** independent corner springs alone do not provide the required roll-stiffness/load-transfer behavior.

## ADR-037 — One physical steering rack drives both front tie rods
**Status:** accepted for prototype 2026-09-23  
**Decision:** left/right front hardpoint positions mirror across the vehicle centerline, but the steering rack translation axis remains one shared chassis-local physical axis.  
**Reason:** one rack translates both inner tie-rod points together; mirroring the rack axis would make the front wheels steer in opposing directions.

## ADR-038 — Rear suspension is a true five-link rigid-upright mechanism
**Status:** accepted for prototype 2026-09-23  
**Decision:** TA-P01 rear kinematics use five independent chassis-to-upright link constraints plus one wheel-travel coordinate rather than reusing the front double-wishbone abstraction.  
**Reason:** individual rear link geometry and damage must produce their own toe/camber effects.

## ADR-039 — Canonical proving-ground runtime resolves all four wheel contacts
**Status:** accepted 2026-09-23  
**Decision:** the high-fidelity prototype route is FL/FR front wishbone + RL/RR rear multi-link, resolved before every vehicle step and fed directly into the tire/powertrain/chassis solver.  
**Reason:** static support, acceleration, braking and steering must come from one vehicle-wide physical chain rather than manually supplied wheel loads.

## ADR-040 — High-fidelity tire contact includes radial compliance
**Status:** accepted for prototype 2026-09-23  
**Decision:** the canonical four-wheel path uses a pressure-dependent radial tire spring/damper with progressive stiffness and bottoming detection. Rigid-radius contact remains only as an LOD/test path.  
**Reason:** an infinitely stiff tire cannot reproduce transient load, curb response or future wheel-hop behavior.

## ADR-041 — Anti-roll reaction participates in tire/suspension equilibrium
**Status:** accepted 2026-09-23  
**Decision:** compliant left/right axle contact is solved iteratively with anti-roll reaction included in each corner’s vertical force equilibrium.  
**Reason:** applying anti-roll load after solving tire deflection creates an inconsistent normal load that is not represented by the tire’s actual deformation.

## ADR-042 — Vehicle assets compile complete runtime-ready physics
**Status:** accepted 2026-09-23  
**Decision:** authored vehicle data is validated and compiled into one immutable package containing both `FTAVehicleRuntimeConfig` and `FTAFourWheelRuntimeConfig`. Authored hardpoints are transformed into COM-local coordinates during compilation.  
**Reason:** solver truth must come from versioned content rather than test-only constants or mutable UObject reads.

## ADR-043 — PhysicsConfigHash includes handling-critical compiled fields
**Status:** accepted for prototype 2026-09-23  
**Decision:** the vehicle configuration hash includes effective tire, wheel, drivetrain, suspension, steering, geometry, mass and inertia fields used by the prototype runtime.  
**Reason:** regression traces, replay diagnostics and multiplayer configuration checks need an identity for the physics actually simulated.

## ADR-044 — Collision external momentum and structural deformation modes are separated
**Status:** accepted for prototype 2026-09-23  
**Decision:** the full external collision impulse is applied once to chassis rigid-body motion. Structural nodes receive a spatially weighted deformation excitation from which rigid translation and rigid rotation modes are removed.  
**Reason:** the structure must deform without adding the same collision linear/angular momentum a second time.

## ADR-045 — Unsprung dynamics is staged behind the quasi-static compliant-contact baseline
**Status:** accepted for prototype 2026-09-23  
**Decision:** explicit unsprung vertical mass/travel integration exists first as an isolated tested primitive. The canonical contact path remains tire/suspension equilibrium until the dynamic model is validated and coupled without force double-counting.  
**Reason:** wheel-hop dynamics add another state and force path; introducing them only after a stable compliant-contact baseline reduces numerical risk.

