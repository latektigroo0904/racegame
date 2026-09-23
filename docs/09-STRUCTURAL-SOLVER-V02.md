# Structural Solver Specification v0.2

Status: implementation-oriented R&D baseline.

## 1. Objective
Simulate persistent crash deformation and mechanically meaningful mount movement without requiring a render-mesh vertex to be a physics particle.

Prototype target: ~350–500 structural nodes and ~1,500–3,000 constraints for the high-fidelity player vehicle.

## 2. Representation
Nodes carry mass, position and velocity. Constraints represent load paths and joints. Separate constraint families are allowed for axial members, bending approximations, mounts, hinges/latches and breakable joints.

Render geometry is driven by a lower-resolution deformation cage derived from the structural state.

## 3. Integration
Baseline integrator: semi-implicit Euler for node velocity/position because it is simple, robust and compatible with iterative constraint solving.

Per structural substep:
1. accumulate external forces;
2. predict velocities/positions;
3. solve constraints iteratively;
4. update plastic rest states;
5. evaluate fracture/fatigue;
6. update velocities from corrected positions;
7. emit damage/mount events.

A position-based/XPBD-style compliance formulation is preferred for stiff constraints because raw explicit springs would require impractically small timesteps.

## 4. Constraint compliance
Represent stiffness through compliance alpha rather than unbounded spring constants where possible. Compliance is material/section dependent and scaled consistently with timestep.

Solver iterations are quality-scalable. Initial target:
- normal driving: 6–10 iterations;
- active severe impact: 12–20 iterations;
- distant/AI damage LOD: much lower or event-based only.

These are profiling seeds.

## 5. Plasticity
Each deformable constraint tracks elastic strain, accumulated plastic strain and current rest length/orientation.

When equivalent strain exceeds yield threshold:
- calculate excess strain;
- move rest state toward current deformation using material plastic-flow coefficient;
- cap per-step plastic change for stability.

Permanent deformation must remain after load removal.

## 6. Fracture
A connection can fail from:
- instantaneous strain above fracture threshold;
- impulse/energy above joint capacity;
- accumulated fatigue/damage;
- thermal weakening where relevant.

Fracture produces a deterministic event containing IDs, timestamp/substep, failure mode and local energy estimate.

## 7. Joints
Do not treat every connection as homogeneous metal. Joint profiles include:
- spot weld;
- seam weld;
- bolt;
- adhesive;
- rubber bushing;
- hinge;
- latch.

This permits doors, bonnets, exhausts and suspension assemblies to fail plausibly.

## 8. Structural zones
Tag nodes/constraints by zone:
- front crash structure;
- front subframe;
- passenger cell;
- floor;
- roof;
- doors;
- rear crash structure;
- rear subframe.

Zone tags support telemetry, damage summaries and different authoring rules; they do not apply arbitrary hit-point multipliers.

## 9. Mechanical mount coupling
Critical mount transforms are derived from structural nodes:
- suspension pickups;
- steering rack;
- engine mounts;
- radiator support;
- drivetrain mounts;
- body hinges/latches.

The vehicle solver consumes the damaged transforms directly. Example: a displaced control-arm pickup changes actual suspension kinematics and alignment.

## 10. Collision load injection
Unreal collision/contact information is converted into impulses distributed to nearby structural nodes using spatial weights and local stiffness/load-path metadata.

Avoid applying an entire impact to one node. Distribution radius and weights depend on contacted component and geometry.

## 11. Adaptive simulation
Normal driving uses base structural frequency. Severe impact activates a temporary high-frequency/high-iteration mode for affected vehicles and can prioritize the impacted region.

Exit high-impact mode only after contact impulses and structural kinetic energy fall below hysteresis thresholds.

## 12. Damage LOD
LOD0 player/hero crash: full structure.
LOD1 nearby important vehicle: reduced iterations and/or reduced structure.
LOD2 traffic: rigid vehicle plus authored mechanical damage events.
LOD3 far traffic: no physical deformation simulation.

A vehicle promoted from lower LOD receives a plausible structural state generated from accumulated damage descriptors rather than reconstructing every historical collision.

## 13. Numerical safeguards
- finite-value assertions in development builds;
- maximum correction per iteration;
- energy/velocity sanity monitors;
- sleeping for settled structural regions;
- deterministic ordering of constraints;
- no hidden frame-rate-dependent constants.

## 14. Performance budgets
Prototype goals on reference desktop:
- normal structural solve: <1.0 ms average for player vehicle;
- severe crash burst: <4 ms structural portion where practical;
- total player vehicle physics normal: <2.5 ms target;
- severe crash total may temporarily reach ~6–8 ms.

Targets are provisional until profiling.

## 15. Authoring tools required
Editor visualization for:
- nodes;
- constraints;
- material/joint type;
- zone;
- yield/fracture limits;
- suspension/mechanical mounts;
- current stress;
- broken constraints.

Need validation command that detects isolated nodes, duplicate constraints, impossible rest lengths, unsupported components and mass inconsistencies.

## 16. Regression suite
STRUCT-001 symmetric frontal barrier.
STRUCT-002 40% offset frontal.
STRUCT-003 narrow pole.
STRUCT-004 side impact.
STRUCT-005 curb/wheel strike.
STRUCT-006 rollover roof load.
STRUCT-007 repeated low-energy fatigue.
STRUCT-008 door hinge/latch failure.
STRUCT-009 deterministic replay tolerance.
STRUCT-010 settled damaged-vehicle stability.

## 17. Key risk
The largest technical risk is coupling visually convincing deformation, stable high-stiffness solving and multiplayer-friendly state at acceptable CPU cost. Therefore the structural solver must be proven before broad vehicle production begins.
