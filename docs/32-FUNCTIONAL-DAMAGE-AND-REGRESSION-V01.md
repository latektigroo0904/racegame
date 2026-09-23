# Functional Damage & Regression v0.1

Status: source-implemented, build-unverified  
Updated: 2026-09-23

## 1. Purpose

This layer closes the gap between structural crash deformation and persistent mechanical consequences that are not purely geometric.

Canonical split:

```
continuous structure displacement
→ suspension hardpoint offsets
→ camber/toe/load geometry

typed damage signals
→ discrete/threshold functional consumers
→ steering authority / free play / hub efficiency / bearing drag / radiator damage
```

The second path must never duplicate the first by adding a second artificial camber/toe offset.

## 2. Steering-rack functional damage

Persistent state:

- `Damage01`;
- `CommandAuthority01`;
- `FreePlayM`.

Effects:

```
driver steering
→ rack mapping
→ command-authority scaling
→ mechanical free-play/deadband
→ physical rack displacement
→ tie-rod geometry
→ wheel steer
```

Structural tie-rod/chassis pickup displacement remains independent and is still applied by the structural suspension binding path.

## 3. Wheel/hub functional damage

Persistent per-wheel state:

- `Damage01`;
- `BrakeEfficiency01`;
- `DriveEfficiency01`;
- `BearingDragTorqueNm`.

Effects in the vehicle step:

- delivered drive torque is reduced by hub drive efficiency;
- maximum brake torque is reduced by brake efficiency;
- bearing drag opposes current wheel rotation;
- tire rolling resistance remains separate.

No generic wheel-alignment scalar is generated here.

## 4. Typed route authoring

`FTAVehicleDamageRouteAuthoringDefinition` now supports:

- Radiator;
- SteeringRack;
- WheelHub.

Versioned route parameters include:

- accepted signal types;
- impact-energy scaling;
- full-damage energy;
- full-crush displacement;
- target wheel index where relevant;
- minimum steering authority;
- maximum steering free play;
- minimum hub brake efficiency;
- minimum hub drive efficiency;
- maximum bearing drag torque.

Wheel-hub routes must reference an existing compiled wheel.

## 5. End-to-end crash path

Source regression now covers:

```
collision impulse
→ structural internal deformation
→ mount displacement signal
→ damage router
→ steering-rack degradation
→ wheel-hub degradation
→ persistent vehicle runtime state
```

Existing direct structural bindings simultaneously preserve:

```
structure displacement
→ suspension pickup displacement
→ alignment change
→ tire force change
```

## 6. Physics configuration identity

The structural portion of `PhysicsConfigHash` now includes:

- structure solver gravity/iterations/correction/plasticity settings;
- impact-distribution parameters;
- node reference positions/mass/pinning;
- structural constraints;
- damage-bridge impact target;
- per-constraint target mapping;
- mount bindings, weights and thresholds;
- route consumer and signal acceptance flags;
- all functional damage route calibration;
- all front/rear suspension structure bindings.

Any handling/crash-relevant authored change must therefore alter configuration identity.

## 7. Telemetry

Vehicle telemetry now records:

Steering:
- steering-rack damage;
- steering command authority;
- rack free play.

Per wheel:
- hub damage;
- hub brake efficiency;
- hub drive efficiency;
- hub bearing drag.

These channels are exported to CSV and are available to regression envelopes/profiles.

## 8. Scenario regression reporting

Two complementary regression layers exist.

### Lightweight envelopes
Single statistic per metric:
- minimum;
- maximum;
- mean;
- final;
- absolute maximum.

### Rich profiles
Per metric:
- observed minimum;
- observed maximum;
- observed mean;
- observed trailing steady-state mean;
- expected min range;
- expected max range;
- expected steady-state range;
- pass/fail.

Reports can be emitted as:
- CSV;
- JSON Lines.

Each record includes:
- scenario ID;
- baseline version;
- trusted/provisional state;
- expected physics config hash;
- observed physics config hash.

## 9. Initial proving-ground scenarios

Six provisional scenarios are defined:

1. Static settle
2. Acceleration
3. Braking
4. Constant steer
5. Asymmetric road
6. Synthetic front-right crash

All remain `bTrustedBaseline = false` until real Unreal Engine execution produces reviewed reference traces.

The synthetic crash scenario now explicitly watches:

- yaw;
- front-right toe;
- front-right camber;
- front-right lateral tire force;
- cooling efficiency;
- steering-rack damage;
- steering command authority;
- front-right hub damage;
- front-right hub brake efficiency.

## 10. Telemetry API cleanup

The former compact telemetry buffer conflicted by name with the newer full vehicle telemetry buffer.

Resolved naming:

- `FTACompactTelemetryRingBuffer` = original lightweight output buffer;
- `FTATelemetryRingBuffer` = canonical full vehicle/scenario telemetry buffer.

This avoids C++ type redefinition and Automation-test class-name collisions.

## 11. Verification status

Source-level tests were added for:

- steering functional damage routing;
- hub functional damage routing;
- monotonic non-healing damage;
- steering authority/free-play effect on geometry;
- hub drive efficiency;
- hub brake efficiency;
- hub bearing drag;
- authored route compilation;
- invalid hub wheel rejection;
- damage-route hash sensitivity;
- crash-to-steering/hub routing;
- damage telemetry capture/export;
- damage regression metrics;
- synthetic crash baseline coverage.

No claim is made that these tests pass until UE 5.8 UHT/UBT/Automation has actually run.
