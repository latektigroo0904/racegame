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
