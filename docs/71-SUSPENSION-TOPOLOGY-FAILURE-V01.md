# Topology-Changing Suspension Failure Solver v0.1

Updated: 2026-10-08
Status: advanced R&D design; not yet implemented

## 1. Problem

Current suspension geometry assumes a constrained upright path.

A broken:
- control arm;
- tie rod;
- toe link;
- multi-link member

changes the mechanism topology.

It cannot be represented correctly by arbitrary camber/toe offsets.

## 2. Target representation

Model the upright/wheel carrier as a rigid body or reduced generalized body constrained by suspension links.

Each link constraint knows:
- chassis pickup;
- upright pickup;
- nominal length;
- axial stiffness/compliance;
- joint limits;
- health;
- broken/connected state.

## 3. Wishbone

Double wishbone:
- upper arm constraints;
- lower arm constraints;
- tie rod;
- spring/damper actuation;
- wheel spindle orientation.

Healthy constraint set determines upright pose.

## 4. Multi-link

Each link contributes a distance/axis constraint.

Geometry solver must tolerate:
- all links healthy;
- one compliant/broken link;
- underconstrained states.

## 5. Failure

On fracture:
- disable/remove affected constraint;
- preserve current upright position/velocity;
- solve remaining constraints dynamically;
- allow uncontrolled DOF rather than injecting alignment.

## 6. Underconstrained handling

If remaining constraints do not uniquely locate upright:
- promote affected corner to dynamic rigid-body/constraint solve;
- bound numerical energy;
- allow wheel/knuckle movement/contact.

Never silently snap back to nominal geometry.

## 7. Tie-rod failure

Steering command no longer constrains toe.
Wheel toe then responds to:
- tire forces;
- remaining geometry;
- joint/bushing/friction limits.

This can cause severe instability naturally.

## 8. Arm failure

Broken wishbone/link may permit:
- camber collapse;
- longitudinal wheel movement;
- body/tire contact;
- loss of load support.

Spring/damper may remain attached depending actual mount topology.

## 9. Wheel detachment

Detachment occurs only when all load/path retention constraints necessary for wheel/upright attachment fail.

Detached wheel becomes separate rigid body/actor.

Do not detach from generic damage threshold alone.

## 10. Collision/contact

Loose upright/wheel still receives:
- road/tire contact;
- chassis/link constraints;
- body contact later.

## 11. Solver approach

Candidate:
- position-based/constraint projection;
- sequential impulse;
- reduced multibody;
- Chaos joint integration.

Selection requires UE prototype/profiling.

Public damage topology interface should not depend on solver choice.

## 12. State transition

Healthy kinematic corner
→ degraded compliant corner
→ topology failure
→ dynamic constrained upright
→ possible detachment.

Promotion can be local to one corner.

## 13. Performance

Only failed/severely deformed corners require expensive dynamic topology solve.

Healthy traffic remains on simpler kinematics.

## 14. Network

Server authoritative for:
- broken link IDs;
- detached state;
- functional alignment/support consequences.

Clients can reconstruct detailed visual motion.

## 15. Tests

SUSP-FAIL-001 healthy dynamic solver matches kinematic pose.
SUSP-FAIL-002 tie-rod removal frees steering constraint.
SUSP-FAIL-003 one arm failure creates expected additional DOF.
SUSP-FAIL-004 no artificial energy explosion.
SUSP-FAIL-005 remaining links still constrain upright.
SUSP-FAIL-006 detachment only after retention path failure.
SUSP-FAIL-007 transition preserves position/velocity.
SUSP-FAIL-008 wheel contact continues after topology failure.
SUSP-FAIL-009 deterministic failure transition.

## 16. Gate

Do not implement against speculative Unreal joint assumptions before current UE 5.8 build works.
The first executable experiment should compare reduced custom constraint solve vs Chaos joints for one failed double-wishbone corner.
