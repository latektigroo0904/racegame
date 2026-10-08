# Differential & Traction Architecture v0.1

Updated: 2026-10-08
Status: implementation-ready specification; canonical promotion gated by UE Proof-of-Physics

## 1. Objective

Replace the temporary quasi-static open-differential splitter with a backend-neutral differential system that supports physically distinct torque-distribution mechanisms without changing tire physics.

Supported target backends:
- Open
- Spool / Locked
- Clutch LSD
- Helical / torque-biasing
- Viscous
- Electronically controlled clutch differential
- Active torque vectoring

## 2. Core rule

The differential distributes driveline torque and reacts to left/right shaft speed.

It does not:
- increase tire friction;
- clamp wheel speed artificially;
- add yaw torque directly to the chassis;
- hide unavailable tire reaction as discarded magic torque.

Untransmitted/absorbed energy and reaction torque must be traceable.

## 3. Common interface

Suggested config header:

```
FTADifferentialConfig
{
    Type

    CarrierInertiaKgm2
    LeftSideInertiaKgm2
    RightSideInertiaKgm2

    FinalDriveRatio
    MechanicalEfficiency

    PreloadTorqueNm
    LockingRampDrive
    LockingRampCoast
    MaxLockTorqueNm

    TorqueBiasRatio
    ViscousCoefficientNms
    ViscousMaxTorqueNm

    ActiveMaxBiasTorqueNm
    ActiveResponseRateNmPerSec
}
```

Runtime input:

```
FTADifferentialInput
{
    CarrierInputTorqueNm
    CarrierAngularSpeedRadPerSec
    LeftAngularSpeedRadPerSec
    RightAngularSpeedRadPerSec
    LeftReactionCapacityNm
    RightReactionCapacityNm
    RequestedActiveBiasNm
    DriveOrCoastState
    DeltaTimeSeconds
}
```

Output:

```
FTADifferentialOutput
{
    LeftDriveTorqueNm
    RightDriveTorqueNm
    InternalLockTorqueNm
    UntransmittedInputTorqueNm
    DissipatedPowerW
}
```

## 4. Open differential

Ideal kinematic relation:

```
omega_carrier = (omega_left + omega_right) / 2
```

Quasi-static first version:
- equal side torque;
- bounded by weaker reaction capacity;
- excess/untransmitted torque explicitly reported.

Future dynamic version:
- carrier/side inertias;
- shaft compliance;
- reaction torque iteration.

Do not lock left/right speed.

## 5. Spool / locked differential

Constraint target:

```
omega_left ≈ omega_right
```

Implementation should be a finite constraint/lock torque, not hard assignment.

```
T_lock =
K_lock × speed_error
+ C_lock × speed_error_rate
```

Clamp to an authored structural capacity if damage/failure is desired.

A true spool may use a very high effective locking stiffness, but still preserve numerical stability.

## 6. Clutch LSD

Relative speed:

```
delta_omega = omega_left - omega_right
```

Available locking torque depends on:
- preload;
- input torque;
- drive/coast ramp;
- friction package state;
- temperature/wear/damage.

Conceptual:

```
T_lock_capacity =
Preload
+ abs(T_input) × RampGain
```

Then:

```
T_lock =
clamp(
    K_slip × delta_omega,
    -T_lock_capacity,
    +T_lock_capacity
)
```

Lock torque is equal and opposite between side shafts.

## 7. Helical / torque-biasing differential

Use torque bias ratio TBR.

Within traction capacity:

```
T_high / T_low <= TBR
```

The device cannot create torque when both sides have effectively zero reaction.

Zero-load behavior must therefore remain weak unless preload/friction is explicitly authored.

## 8. Viscous differential

```
T_viscous =
C_viscous × (omega_left - omega_right)
```

Clamp to max torque.

Optional temperature state later:
- viscous heating;
- viscosity change;
- fade/failure.

## 9. Electronically controlled clutch diff

Same physical clutch capacity model as clutch LSD, but commanded lock fraction:

```
T_lock_capacity =
lerp(
    Preload,
    MaxLockTorque,
    Command01
)
```

Command source is a controller, not the differential solver.

Rate-limit actuator changes.

## 10. Torque vectoring

Active bias torque:

```
T_left  = T_base_left  - T_bias
T_right = T_base_right + T_bias
```

with equal/opposite transfer.

The actuator has:
- max transfer torque;
- response rate;
- thermal/energy limits;
- damage hooks.

The controller may request bias from yaw/steering/driver state, but the differential backend enforces actuator physics.

## 11. Energy accounting

For lock devices:

```
P_dissipated =
abs(T_lock × delta_omega)
```

This feeds:
- clutch LSD temperature;
- viscous unit temperature;
- active clutch thermal state.

No thermal energy is invented from equal shaft speed.

## 12. Driveline compliance interaction

The differential sits downstream of final drive and upstream of half-shafts.

Long-term path:

engine
→ clutch
→ gearbox
→ final drive
→ carrier inertia
→ differential
→ left/right shaft compliance
→ wheel inertias
→ tire reaction

The current single driveline-compliance state is a prototype simplification.

## 13. Damage

Possible damage states:
- worn clutch plates;
- reduced preload;
- reduced max lock torque;
- broken side gear;
- broken CV/half-shaft;
- seized differential;
- overheating;
- electronic actuator failure.

Damage changes physical capacity/state, not vehicle HP.

## 14. Authoring

Vehicle definition should eventually author:
- differential type;
- backend parameters;
- front/rear/center units for AWD;
- thermal package;
- actuator package;
- damage thresholds.

All handling-critical values enter PhysicsConfigHash.

## 15. AWD extension

Use three independently modeled units:
- front differential;
- rear differential;
- center coupling/differential.

Center types:
- fixed split;
- open;
- viscous;
- clutch;
- planetary bias;
- active transfer case.

Do not represent AWD as four wheels receiving a fixed torque percentage regardless of shaft speed/reaction.

## 16. Controller boundary

Traction/stability controller may request:
- engine torque reduction;
- brake pressure;
- active differential command.

The differential solver receives a bounded actuator request.

It does not calculate driver-assist policy itself.

## 17. Telemetry

Per differential:
- input torque;
- left/right output torque;
- left/right shaft speed;
- relative speed;
- lock torque;
- commanded lock;
- untransmitted torque;
- dissipated power;
- temperature/wear when available.

## 18. Tests

DIFF-001 open diff equal side torque.
DIFF-002 open diff permits speed difference.
DIFF-003 weak side limits open-diff torque.
DIFF-004 spool resists speed difference.
DIFF-005 clutch LSD preload produces finite lock.
DIFF-006 drive/coast ramps differ when authored.
DIFF-007 clutch lock torque is equal/opposite.
DIFF-008 helical TBR is respected.
DIFF-009 helical zero-load side cannot create arbitrary torque.
DIFF-010 viscous torque scales with speed difference.
DIFF-011 active bias conserves axle torque apart from explicit losses.
DIFF-012 lock energy dissipation has correct sign.
DIFF-013 fixed state/input deterministic.
DIFF-INT-001 split-mu launch.
DIFF-INT-002 power-on corner.
DIFF-INT-003 lift-off/coast corner.
DIFF-INT-004 one-wheel-low-load curb case.

## 19. Promotion gate

Do not replace the existing open-diff prototype until:
- UE executable baseline is archived;
- open-backend new solver reproduces current expected split-traction behavior;
- torque/energy sign tests pass;
- no hidden speed clamp exists;
- differential changes affect PhysicsConfigHash;
- telemetry proves torque conservation and explicit losses.
