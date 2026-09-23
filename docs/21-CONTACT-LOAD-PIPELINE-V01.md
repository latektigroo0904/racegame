# Suspension / Road Contact Load Pipeline v0.1

Status: implemented prototype baseline
Updated: 2026-09-23

## Purpose
Remove hand-authored tire contact speed and vertical load from the high-fidelity front-suspension path.

Canonical front-corner chain:

```
chassis pose/velocity
→ double-wishbone geometry
→ wheel center/orientation
→ road-plane contact
→ suspension travel
→ damper motion ratio
→ spring/damper reaction
→ tire vertical load
→ contact-patch velocity
→ tire solver
```

## Road representation
Prototype contact uses:
- world-space point;
- normalized world-space road normal;
- surface state.

The road-plane abstraction is a proving-ground interface. Production world contact will later come from collision/road surface queries.

## Travel solve
For the double-wishbone front corner:
1. solve geometry at full droop;
2. solve geometry at full bump;
3. classify road reachability;
4. when bracketed, bisect suspension travel;
5. each candidate uses the full 3D wishbone constraint solver;
6. accept when tire support point is sufficiently close to the road plane.

Current tire support approximation:
```
ContactPoint = WheelCenter - RoadNormal * TireRadius
```

Tire carcass vertical deflection is not yet included.

## Contact states

### In contact
Road is reachable within suspension travel.

### Full droop / airborne
Even at maximum droop the tire remains above the road:
- tire normal load = 0;
- suspension geometry is still solved;
- no fake road-support force is applied.

### Full bump / overtravel
Even at maximum bump the nominal tire support point penetrates the road:
- travel clamps at bump limit;
- penetration is reported;
- bump-stop force participates;
- future tire-deflection/contact solver must resolve remaining penetration.

## Damper motion ratio
The double-wishbone geometry now contains:
- chassis damper mount;
- lower-arm damper mount.

The lower-arm mount is reconstructed from the actual current wishbone geometry.

Motion ratio is estimated from the derivative:
```
MotionRatio ≈ abs(d DamperLength / d WheelTravel)
```

Prototype uses a small geometric travel probe.

This replaces the temporary assumption that damaged suspension always has a motion ratio of 1.0.

## Chassis point velocity
Contact-patch velocity is derived from rigid-body state:
```
v_point =
v_COM + omega × r
```

The result is projected into the solved tire tangent basis:
- longitudinal velocity;
- lateral velocity.

These values can be passed directly to the tire solver.

## Vertical load
Prototype uses a massless-unsprung approximation:

```
NormalLoad =
max(0, Spring + Damper + Stops + AntiRollAdjustment)
```

The equal/opposite road-support reaction is applied to the chassis at the physical contact point.

Future work:
- unsprung masses;
- tire vertical stiffness/damping;
- wheel hop;
- contact impulse/compliance;
- road roughness normal dynamics.

## Anti-roll coupling
Left/right suspension travel may be coupled by:
```
DeltaF = K_ARB * (TravelLeft - TravelRight)
```

with equal/opposite load adjustment and a configured maximum.

Positive travel means bump/compression.

## Structural damage
Structure nodes retain reference positions.

Compiled pickup bindings resolve:
```
PickupOffset =
weighted average(
    CurrentNodePosition - ReferenceNodePosition
)
```

Bindings exist for:
- upper inner A/B;
- lower inner A/B;
- tie-rod inner;
- damper chassis mount.

The offsets enter the same double-wishbone geometry solver used for undamaged motion.

## Cache rule
The simple 1D travel lookup is invalid when:
- structural pickup displacement is present; or
- the steering rack is displaced.

Reason: the existing cache has only one independent dimension (travel).

Future optimized caches may be multidimensional.

## Implemented tests
Tests now cover:
- reference road contact;
- self-derived static vertical load;
- contact speed from chassis velocity;
- full-droop contact loss;
- damaged pickup changing camber;
- anti-roll equal/opposite load transfer;
- structural node displacement → pickup offset;
- weighted structural binding;
- resolved suspension contact → tire force → chassis acceleration.

## Known limitations
- road plane rather than production collision geometry;
- no tire vertical compliance yet;
- front double wishbone only;
- rear multi-link path still pending;
- no unsprung mass;
- steering rack input not yet connected to driver controls;
- motion-ratio finite difference is R&D quality, not final optimized implementation.

## Acceptance
This pipeline is accepted as the v0.1 front-corner baseline when it can provide all of the following without manually supplied tire load/speed:
- wheel center;
- camber/toe;
- contact point;
- longitudinal/lateral contact speed;
- normal load;
- suspension reaction force;
- tire surface state.
