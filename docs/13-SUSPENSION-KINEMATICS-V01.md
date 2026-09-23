# Suspension Kinematics v0.1

> Implementation status update — 2026-09-23: the front double-wishbone damaged-geometry path, damper motion-ratio derivation, road-contact travel solve, structural pickup bindings and self-derived contact load/velocity are now implemented as the v0.2 prototype baseline. See `20-DOUBLE-WISHBONE-GEOMETRY-V02.md` and `21-CONTACT-LOAD-PIPELINE-V01.md`. This document remains the original architectural specification.

Status: prototype specification
Updated: 2026-09-23

## 1. Objective
Suspension geometry must arise from physical pickup points and constraints, not from arbitrary wheel travel curves.

Core outputs per wheel:
- wheel center pose;
- camber;
- toe;
- caster/trail-related steering geometry;
- spring/damper length and velocity;
- motion ratio;
- jacking/anti geometry effects;
- contact-load path.

## 2. Coordinate convention
All authored hardpoints are stored in chassis-local SI coordinates.

Recommended axes inside the simulation layer:
- +X forward;
- +Y right;
- +Z up.

Unreal conversion is isolated at the engine boundary.

## 3. Runtime architecture
Each corner owns:
```
FTASuspensionCornerConfig
FTASuspensionCornerState
```

Config contains immutable geometry/material/calibration.
State contains solved upright/wheel pose, travel, forces and damaged hardpoint offsets.

## 4. Geometry source
Author actual hardpoints:
- upper/lower arm chassis pivots;
- upright pivots;
- tie-rod inner/outer;
- damper mounts;
- spring mounts;
- anti-roll attachment;
- bump/droop stops.

Do not author camber-vs-travel as the primary truth.

## 5. Solver strategy
Prototype uses two modes:

### Normal undamaged mode
A precomputed kinematic lookup generated from the hardpoint geometry can accelerate repetitive solving.

### Damaged/modified mode
Use runtime geometric constraint solving from current hardpoints.

This hybrid preserves performance while allowing crash-deformed pickups to change alignment correctly.

## 6. Cache generation
At asset compilation:
1. validate hardpoint topology;
2. solve suspension across travel range;
3. sample camber, toe, motion ratio, wheel-center path;
4. detect singular/invalid regions;
5. store lookup table in compiled vehicle config.

Cache is invalidated if:
- pickup geometry changes;
- suspension part changes;
- structural damage exceeds configured tolerance.

## 7. Double-wishbone front
TA-P01 front corner consists of:
- upper arm;
- lower arm;
- upright;
- tie rod;
- spring/damper;
- optional anti-roll connection.

The upright pose is solved from arm constraints, then steering tie-rod geometry constrains toe/steer.

## 8. Multi-link rear
Represent links individually where they meaningfully constrain upright degrees of freedom.

Do not collapse every multi-link design into a fake double-wishbone unless explicitly using a lower-fidelity LOD.

## 9. Wheel travel
Define:
```
Travel = WheelCenterZ_relative_to_reference
```

Positive convention must be fixed globally; recommendation:
- positive = bump/compression;
- negative = droop.

## 10. Spring force
Base:
```
F_spring = K_spring * compression
```

Allow:
- progressive spring curve;
- tender/helper springs later;
- bump stop as separate nonlinear element.

Spring force is applied through its actual axis/motion ratio, not directly as vertical chassis force.

## 11. Damper force
```
F_damper = f(shaftVelocity)
```

Separate compression and rebound curves.

Prototype may use piecewise-linear tables:
- low-speed compression;
- high-speed compression;
- low-speed rebound;
- high-speed rebound.

This supports meaningful tuning without unnecessary fluid-valve simulation.

## 12. Bump and droop stops
Stops are nonlinear force elements near travel limits.

Requirements:
- no hard teleport/position clamp under normal solving;
- high but stable progressive force;
- structural overload can damage mounts rather than producing infinite constraint force.

## 13. Anti-roll bar
Model left/right coupling from relative suspension displacement.

Simple torsional approximation:
```
T_ARB = K_ARB * (phi_left - phi_right)
```

Map torque through actual lever geometry to corner forces.

Damage/disconnection of one link disables or strongly reduces bar action.

## 14. Steering tie rod
Toe/steer must emerge from rack position + tie-rod/upright geometry.

Consequences:
- bump steer is measurable;
- damaged tie rod changes toe;
- shifted steering rack mount changes both sides;
- steering lock depends on real geometry.

## 15. Alignment outputs
At each substep expose:
```
Camber_deg
Toe_deg
Caster_deg_or_equivalent
KingpinInclination
MechanicalTrail
ScrubRadius
RideHeight
```

Some values can be derived at lower frequency for telemetry rather than every inner solver iteration.

## 16. Tire coupling
The tire solver receives wheel orientation and wheel-center velocity after suspension kinematics.

Order:
```
Chassis pose
→ damaged pickup positions
→ suspension/upright solution
→ wheel kinematics
→ contact patch
→ tire forces
```

## 17. Structural damage coupling
Each suspension pickup may bind to one or more structural nodes.

Runtime pickup:
```
P_runtime =
P_authored
+ StructuralDisplacement
+ PartDeformationOffset
```

Do not directly apply an arbitrary "damaged toe" multiplier when geometry information exists.

## 18. Component deformation
For early prototype, structural chassis pickup movement is primary.

Later part-level deformation can add:
- bent control arm length/shape;
- bent tie rod;
- damaged upright;
- subframe displacement;
- wheel-bearing play.

## 19. Wheel separation
Wheel/hub attachment has failure constraints.

Possible states:
```
Normal
Loose
SeverelyMisaligned
Detached
```

Detached wheels become independent rigid/structural objects at appropriate LOD.

## 20. Static setup solve
Before simulation:
1. apply mass and gravity;
2. solve static spring compression;
3. determine ride height;
4. compute alignment at settled state;
5. verify no stop/preload conflicts.

Vehicle spawn should not begin with a large unresolved suspension impulse.

## 21. Setup parameters
User-tunable setup may include:
- spring rate;
- ride height/preload;
- damper curves;
- anti-roll stiffness;
- static camber;
- static toe;
- steering lock;
- caster only where hardware permits;
- corner weighting where vehicle hardware supports it.

Adjustments must modify geometry/forces rather than cosmetic values.

## 22. Suspension LOD
### LOD0
Full geometric kinematics, high-frequency force solution.

### LOD1
Lookup/cached kinematics + reduced damper detail.

### LOD2
Single travel DOF with precomputed alignment curves.

### LOD3
Simplified rigid wheel/body relation for distant traffic.

## 23. Validation plots
Editor tool should generate:
- camber vs travel;
- toe vs travel;
- wheel-center path;
- motion ratio vs travel;
- spring/damper length vs travel;
- bump steer vs steering angle/travel.

Warnings flag discontinuities and singularities.

## 24. Required tests
- static ride-height convergence;
- symmetric left/right vehicle remains symmetric;
- pure bump;
- pure roll;
- steering sweep;
- bump-steer sweep;
- curb impact displacement;
- tie-rod bend;
- chassis pickup displacement;
- anti-roll link disconnect;
- wheel detach path.

## 25. TA-P01 calibration goal
Front: double wishbone, modest negative camber gain in bump, low bump steer.
Rear: multi-link, stability-biased toe behaviour under bump/load.

These are handling intentions, not final numeric geometry.

## 26. Acceptance
v0.1 is accepted when:
- static and dynamic geometry is stable;
- alignment changes continuously through travel;
- damaged pickups alter wheel alignment naturally;
- tire solver receives consistent wheel velocities/orientations;
- normal undamaged mode can use cached kinematics without behavioural discontinuity when switching to damaged runtime solving.
