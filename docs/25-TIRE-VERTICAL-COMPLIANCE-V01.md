# Tire Vertical Compliance v0.1

Status: next implementation layer
Updated: 2026-09-23

## 1. Problem
The current road-contact resolver uses a rigid-radius tire support point.

That is useful for proving suspension geometry but vertically behaves like an infinitely stiff tire.

A high-fidelity vehicle requires:
- radial tire deflection;
- radial stiffness;
- tire damping;
- pressure dependence;
- curb/rough-road compliance;
- later unsprung-mass wheel-hop dynamics.

## 2. Phase A — radial tire contact
Add persistent tire state:
```
RadialDeflectionM
RadialDeflectionVelocityMps
```

Prototype force:
```
F_tire =
K_effective * x
+ K_progressive * x^2
+ C_radial * x_dot
```

with:
```
F_tire >= 0
```

No road adhesion/tension is allowed.

## 3. Pressure dependence
Effective radial stiffness:
```
K_effective =
K_reference *
(Pressure / ReferencePressure)^PressureExponent
```

bounded to a safe prototype range.

A severely underinflated tire therefore becomes more compliant.

## 4. Maximum deflection
A configured maximum useful radial deflection marks tire bottoming/rim-impact territory.

Above it:
- radial state is clamped for the normal tire model;
- a bottomed flag is exposed;
- future rim/contact damage receives excess impact energy.

## 5. Loaded contact point
With deflection `x`:
```
LoadedRadius =
UnloadedRadius - x
```

The physical road patch is based on loaded radius, not unloaded radius.

## 6. Suspension/tire equilibrium — massless-unsprung transition
Before explicit unsprung mass is introduced, the high-fidelity contact resolver can solve:

```
F_tire(travel)
≈
F_suspension(travel)
```

through the available suspension travel.

This is a transitional model but is physically better than rigid-radius contact.

## 7. Airborne
If full droop still leaves a positive tire-road gap:
- deflection = 0;
- normal load = 0;
- tire force = 0.

## 8. Bump-stop/bottomed state
If no force-equilibrium root exists before full bump:
- suspension reaches its physical stop;
- tire may still compress;
- remaining contact force is reported;
- future unsprung/contact solver handles the acceleration/impulse.

## 9. Phase B — unsprung mass
Next layer introduces a wheel/upright generalized coordinate with:
- unsprung mass;
- vertical/generalized velocity;
- suspension force;
- tire radial force;
- unsprung gravity;
- road excitation.

Then:
```
m_unsprung * q_ddot =
Q_tire - Q_suspension + Q_gravity
```

This enables:
- wheel hop;
- curb response;
- rough-road compliance;
- loss/recovery of road contact;
- damper tuning effects at higher frequency.

## 10. Chassis force
In the Phase-A massless-unsprung equilibrium:
- road normal force and suspension force are approximately equal;
- the normal reaction can continue to be applied to the chassis at the contact point.

In Phase B:
- chassis receives suspension/link reactions;
- unsprung state receives tire/road reaction;
- force paths must avoid double counting.

## 11. Tests
Phase A:
- zero compression gives zero normal force;
- more deflection increases force;
- compression velocity adds damping;
- underpressure lowers radial stiffness;
- force never becomes tensile;
- maximum deflection flags bottoming.

Integration:
- reference static vehicle settles with finite tire deflection;
- road step compresses tire before hard suspension stop;
- vertical load remains continuous across small road-height changes.

## 12. Acceptance
Phase A passes when tire normal load is no longer produced by an infinitely rigid radius constraint and pressure/deflection can change vertical compliance.

Phase B passes when wheel-hop and unsprung vertical acceleration are explicit state rather than quasi-static equilibrium.
