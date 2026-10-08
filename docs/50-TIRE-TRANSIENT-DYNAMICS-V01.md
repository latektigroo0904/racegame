# Tire Transient Dynamics v0.1

Updated: 2026-10-08
Status: implementation-ready specification; steady-state tire law remains canonical until gated promotion

## 1. Objective

Add time-dependent tire-force build-up between kinematic slip input and the existing steady-state force target.

Current steady-state tire solver responds effectively immediately to slip.

Real tires have carcass/contact-patch memory.

The first transient model must improve:
- steering response;
- slalom phase lag;
- ABS/TCS behavior;
- wheel-hop coupling;
- curb/pothole response;
- FFB continuity.

## 2. Architecture

Keep the current tire backend as target-force generator:

```
kinematics/surface/state
→ steady-state target Fx/Fy/Mz
→ transient response
→ applied Fx/Fy/Mz
```

Do not bake transient state into surface grip multipliers.

## 3. State

Per tire:

```
FTATireTransientState
{
    RelaxedLongitudinalSlip
    RelaxedLateralSlipRad

    AppliedLongitudinalForceN
    AppliedLateralForceN
    AppliedAligningMomentNm

    ContactAgeSeconds
    WasInContact
}
```

A more advanced backend may store contact-patch shear state instead.

## 4. Relaxation length

A standard first-order distance-domain idea:

```
dX/dt =
V_effective / L_relax
× (X_target - X)
```

Where:
- X may be slip or force state;
- L_relax is relaxation length [m];
- V_effective avoids singular behavior near zero speed.

Equivalent time constant:

```
tau = L_relax / max(|Vx|, V_floor)
```

## 5. Longitudinal transient

Prototype:

```
d(kappa_relaxed)/dt =
(kappa_target - kappa_relaxed) / tau_x
```

Then feed relaxed slip into the steady-state force law.

Alternative later:
relax force directly.

Slip-state relaxation is preferred initially because it preserves existing force saturation/combined-slip logic.

## 6. Lateral transient

```
d(alpha_relaxed)/dt =
(alpha_target - alpha_relaxed) / tau_y
```

Lateral relaxation length may differ from longitudinal.

## 7. Aligning moment

Mz should not simply jump independently.

Initial rule:
- compute target pneumatic trail from relaxed lateral state;
- derive Mz from applied/relaxed lateral force and trail.

Future:
- dedicated trail/transient state;
- turn-slip/camber effects.

## 8. Low-speed behavior

Pure relaxation-time formulation explodes at near-zero speed.

Use bounded effective speed:

```
V_effective =
max(abs(Vx), RelaxationSpeedFloor)
```

Below the existing low-speed transition range:
- blend toward creep/static friction;
- decay stale transient state when contact is quasi-static;
- avoid long “memory” after stopping.

## 9. Contact loss/re-entry

When tire becomes airborne:
- no road friction force;
- transient contact age resets/decays;
- do not preserve arbitrary saturated shear forever.

On re-contact:
- initialize state continuously from current wheel/ground velocity;
- optional short contact-build window later.

## 10. Load dependence

Relaxation length may depend on:
- vertical load;
- pressure;
- carcass temperature;
- tire construction.

v0.1 can start with constant authored Lx/Ly.

The interface should permit scaling later.

## 11. Pressure and damage

Low pressure:
- generally larger deflection;
- altered relaxation;
- more carcass lag.

Tire structural damage may:
- increase response lag;
- create asymmetric force;
- destabilize trail.

These are later calibration paths, not v0.1 mandatory behavior.

## 12. Combined slip

Use relaxed longitudinal/lateral slip values as the demand state entering the existing combined-slip envelope.

This ensures:
- no separate transient grip bonus;
- combined force still respects the finite friction envelope.

## 13. Integration

Prefer exponential first-order integration for timestep robustness:

```
blend = 1 - exp(-dt / tau)
X_next = X + blend × (X_target - X)
```

This is more stable than raw Euler at high rate.

All exponent inputs must be finite/bounded.

## 14. Fixed-step/substep

Transient update occurs at tire solver cadence.

If tire solver substeps:
- transient state substeps with it;
- do not update only once per render frame.

## 15. Authoring

Per tire compound/construction:

```
LongitudinalRelaxationLengthM
LateralRelaxationLengthM
RelaxationSpeedFloorMps
ContactReentryBlendSeconds
```

These enter PhysicsConfigHash.

## 16. Telemetry

Per wheel:
- raw slip ratio/angle;
- relaxed slip ratio/angle;
- target force;
- applied transient force;
- target/applied aligning moment;
- transient lag error.

This permits direct phase-lag plots.

## 17. Tests

TIRE-TR-001 zero target stays zero.
TIRE-TR-002 step slip force rises monotonically.
TIRE-TR-003 longer relaxation length responds slower.
TIRE-TR-004 higher speed shortens time response for fixed relaxation length.
TIRE-TR-005 fixed distance response is approximately speed invariant.
TIRE-TR-006 no overshoot for first-order backend.
TIRE-TR-007 low-speed state remains finite.
TIRE-TR-008 contact loss clears applied road force.
TIRE-TR-009 re-contact has no impulse-like force teleport.
TIRE-TR-010 combined-slip envelope remains respected.
TIRE-TR-011 deterministic at fixed dt.
TIRE-INT-001 steering step response.
TIRE-INT-002 slalom phase response.
TIRE-INT-003 ABS cycle interaction.
TIRE-INT-004 road-step/unsprung interaction.

## 18. Calibration strategy

Do not guess “realistic feel” by eye only.

Use:
- published tire transient literature;
- generic passenger/sport tire response bands;
- normalized steering-step traces;
- phase lag at multiple speeds.

TA-P01 values remain provisional until executable proving-ground traces exist.

## 19. Non-goals v0.1

Deferred:
- full brush bristle state;
- belt modal dynamics;
- enveloping/contact-patch geometry;
- turn slip;
- transient camber thrust;
- flat-spot periodic force;
- conicity/ply steer;
- high-frequency acoustic tire model.

## 20. Promotion gate

The steady-state solver remains canonical until:
- UE current baseline exists;
- transient module passes isolated tests;
- skidpad steady-state force is preserved;
- steering/slalom transients improve without numerical oscillation;
- ABS/TCS tests remain stable;
- CPU cost is profiled.
