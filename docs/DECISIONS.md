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

