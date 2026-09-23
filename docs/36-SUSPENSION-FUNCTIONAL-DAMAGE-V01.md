# Suspension Functional Damage v0.1

Status: source-implemented, UE-build-unverified  
Updated: 2026-09-23

## 1. Scope

This layer adds crash-driven mechanical degradation for suspension force-producing components that the current kinematic suspension model can represent without inventing fake alignment damage.

Implemented consumers:

- `SuspensionCorner`;
- `AntiRollLink`.

Not yet implemented as discrete failures:

- broken upper/lower control arm;
- broken tie rod;
- broken rear multi-link arm;
- detached upright/wheel.

Those failures require a solver capable of removing geometric constraints and solving a partially free upright. They are not approximated with arbitrary camber/toe multipliers.

## 2. Geometry truth remains separate

Continuous structural deformation remains the only source of pickup displacement:

```
structure node displacement
→ suspension hardpoint displacement
→ camber / toe / travel geometry
```

Functional suspension damage changes force capacity only:

```
typed damage signal
→ spring / damper / stop efficiency
→ suspension force response
```

No second alignment-damage path is added.

## 3. Persistent per-wheel state

`FTASuspensionFunctionalDamageState` stores:

- `SpringDamperDamage01`;
- `SpringEfficiency01`;
- `DampingEfficiency01`;
- `StopEfficiency01`;
- `AntiRollLinkDamage01`;
- `AntiRollLinkEfficiency01`.

Damage is monotonic until a future explicit repair operation.

## 4. Suspension-corner consumer

A `SuspensionCorner` route targets one wheel index.

Severity can be driven by:

- impact energy;
- structural displacement;
- structural fracture.

At severity `s`, efficiency moves from 1.0 toward the authored minimum.

Runtime application:

```
effective spring rate
= base spring rate × spring efficiency

effective bump damping
= base bump damping × damping efficiency

effective rebound damping
= base rebound damping × damping efficiency

effective bump-stop rate
= base bump-stop rate × stop efficiency

effective droop-stop rate
= base droop-stop rate × stop efficiency
```

The authored geometry and kinematic cache are unchanged.

## 5. Residual spring support gate

The current canonical contact solver is quasi-static and assumes a constrained suspension mechanism with a force-bearing suspension stack.

Therefore a `SuspensionCorner` route currently requires:

```
MinimumSpringEfficiency01 > 0
```

The default is 0.10.

A true spring/damper detachment with zero support is deferred until the runtime can represent the corresponding free motion without forcing an invalid equilibrium solve.

Damping may fall to zero.

Stops are independently configurable and default to remaining intact.

## 6. Anti-roll link consumer

An `AntiRollLink` route targets a wheel/drop-link.

Per axle:

```
anti-roll efficiency
= min(left link efficiency, right link efficiency)
```

Then:

```
effective coupling rate
= base coupling rate × axle efficiency

effective max transfer force
= base max transfer force × axle efficiency
```

A complete drop-link fracture may therefore reduce axle anti-roll transfer to zero.

This is intentional: both links lie in the anti-roll load path.

## 7. Authoring

`FTAVehicleDamageRouteAuthoringDefinition` now supports:

- `SuspensionCorner`;
- `AntiRollLink`.

New route calibration:

- minimum spring efficiency;
- minimum damping efficiency;
- minimum stop efficiency;
- minimum anti-roll-link efficiency.

Wheel-scoped routes must reference an existing compiled wheel.

All new route fields are included in `PhysicsConfigHash`.

## 8. Crash integration

Source regression now covers:

```
collision
→ internal structure displacement
→ mount displacement signal
→ SuspensionCorner / AntiRollLink route
→ persistent suspension damage state
```

This exists alongside:

- steering-rack damage;
- wheel-hub damage;
- radiator damage;
- direct structure-to-hardpoint deformation.

## 9. Telemetry

Per wheel, telemetry now records:

- suspension spring/damper damage;
- spring efficiency;
- damping efficiency;
- stop efficiency;
- anti-roll-link damage;
- anti-roll-link efficiency.

These fields are:

- exported to CSV;
- available as regression metrics;
- suitable for future crash/proving-ground scenario baselines.

## 10. Source-level regressions

Coverage includes:

### Router
- suspension-corner impact severity;
- monotonic non-healing behavior;
- structural fracture reaching authored minima;
- anti-roll link impact degradation;
- anti-roll link complete fracture;
- rejection of zero spring support in the current solver.

### Four-wheel runtime
- reduced spring efficiency reduces support at the damaged corner;
- asymmetric suspension degradation creates a different chassis roll response;
- broken front anti-roll link reduces asymmetric-road load transfer.

### Vehicle definition
- authored suspension routes compile;
- wheel targets/calibration survive compilation;
- calibration changes alter physics hash;
- out-of-range wheel targets are rejected.

### Crash pipeline
- crash-generated structural displacement reaches suspension-corner state;
- crash-generated structural displacement reaches anti-roll-link state.

### Telemetry/regression
- damage state maps into telemetry;
- CSV exposes the new channels;
- regression envelopes can directly evaluate spring and anti-roll-link health.

## 11. Current limitation

This is not yet a general broken-suspension topology solver.

A future discrete link-failure model must remove/disable actual constraints and solve the resulting upright degrees of freedom.

Until then:

**do not simulate a broken arm or tie rod by injecting arbitrary camber/toe.**
