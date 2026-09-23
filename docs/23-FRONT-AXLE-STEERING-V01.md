# Front Axle / Steering Rack Runtime v0.1

Status: implementation specification
Updated: 2026-09-23

## 1. Goal
Turn the existing single-corner front suspension into one physically coherent front axle.

Canonical chain:

```
driver steering [-1..1]
→ rack displacement
→ shared steering rack motion
→ left/right tie-rod inner points
→ left/right double-wishbone solve
→ left/right wheel toe/steer
→ road contacts
→ anti-roll coupling
→ tire inputs
```

## 2. Mirror rule
TA-P01 authors one canonical right-front geometry.

Left-front geometry is mirrored across chassis centerline:
```
Y_left = -Y_right
```

Mirrored:
- chassis pickup positions;
- upright points;
- wheel center;
- damper points.

Not mirrored:
- chassis coordinate convention;
- physical steering rack translation axis.

Reason:
one steering rack translates both inner tie-rod points in the same chassis-local direction.

## 3. Steering rack model
Initial runtime config:
- max rack displacement left/right;
- steering input exponent for center sensitivity;
- optional steering sign.

Mapping:
```
steeringShaped =
sign(input) * abs(input)^exponent

rackDisplacement =
steeringSign *
steeringShaped *
maxRackDisplacement
```

No speed-sensitive steering is applied inside the physics truth layer.

Assists may shape driver command before this layer later.

## 4. Steering geometry
The rack does not directly command wheel yaw.

Each side solves:
```
TieInnerRuntime =
TieInnerReference
+ RackAxis * RackDisplacement
+ StructuralOffset
```

Tie-rod length + rigid upright constraints create steering angle.

Therefore:
- Ackermann can emerge;
- bump steer is measurable;
- rack-mount damage affects both sides;
- tie-rod/pickup damage affects individual sides.

## 5. Front axle state
Persistent:
- left double-wishbone state;
- right double-wishbone state;
- left suspension force/travel state;
- right suspension force/travel state.

## 6. Front axle output
Per step:
- rack displacement;
- left/right resolved contacts;
- left/right toe/steering angle;
- left/right camber;
- left/right travel;
- left/right normal load;
- Ackermann delta;
- bump-steer values;
- convergence state.

## 7. Ackermann telemetry
For a non-zero steering input:
- identify inside/outside wheel from turn sign;
- compare absolute steering angles.

Prototype metric:
```
AckermannDelta =
abs(innerAngle) - abs(outerAngle)
```

Positive = inner wheel steers more than outer.

No claim is made that the seed geometry has ideal Ackermann percentage.

## 8. Bump-steer telemetry
At current travel, solve a steering-neutral comparison.

```
BumpSteer =
Toe(travel, rack=0)
- Toe(referenceTravel=0, rack=0)
```

This separates suspension-travel-induced toe from commanded steering.

## 9. Anti-roll ordering
Per axle:
1. solve left/right geometric road contact independently;
2. calculate each base spring/damper load;
3. apply anti-roll equal/opposite adjustment;
4. build final tire contact inputs.

## 10. Damage
Left/right pickup offsets are independent.

Shared rack/subframe damage will later be represented by bindings that move both sides through their authored mounts.

## 11. Required tests
- mirrored reference geometry converges;
- left/right reference wheel centers mirror;
- zero steering produces near-symmetric toe;
- positive rack displacement steers both wheels in same turn direction;
- inside wheel has different angle from outside wheel;
- steering input clamps to physical rack travel;
- symmetric bump produces near-zero anti-roll transfer;
- asymmetric travel produces equal/opposite anti-roll load transfer;
- one-side pickup damage alters only intended corner directly.

## 12. Acceptance
v0.1 passes when one normalized steering command can drive both front corners through a shared rack, produce two valid road contacts, expose Ackermann/bump-steer metrics, and deliver final left/right tire inputs with anti-roll load transfer.
