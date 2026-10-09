# Hydraulic Brake & ABS Architecture v0.1

Updated: 2026-10-08
Status: implementation-ready specification; not canonical until UE Proof-of-Physics gate passes

## 1. Objective

Replace the current direct normalized brake-input → max wheel torque path with a physically traceable actuation path:

driver pedal
→ booster/input gain
→ master cylinder pressure request
→ hydraulic circuits
→ per-corner line pressure
→ caliper clamp force
→ friction torque capacity
→ wheel rotational dynamics

The existing brake thermal/fade/wear state remains the friction-capacity layer.

ABS modulates hydraulic pressure. It never edits tire grip or wheel speed directly.

## 2. Separation of concerns

### Actuation layer owns
- pedal command;
- hydraulic pressure;
- circuit state;
- brake bias;
- pressure modulation;
- fluid condition.

### Friction/thermal layer owns
- brake temperature;
- fade factor;
- wear factor;
- friction torque capacity.

### Hub/damage layer owns
- mount/caliper efficiency;
- bearing drag;
- structural consequences.

Final brake torque must combine these once.

## 3. Configuration

Suggested runtime config:

```
FTABrakeHydraulicConfig
{
    PedalForceMaxN
    BoosterGain
    MasterCylinderAreaM2
    MaxSystemPressurePa

    FrontBias01

    FrontCaliperPistonAreaM2
    RearCaliperPistonAreaM2

    FrontEffectiveDiscRadiusM
    RearEffectiveDiscRadiusM

    FrontPadFrictionCoefficient
    RearPadFrictionCoefficient

    FrontCircuitComplianceM3PerPa
    RearCircuitComplianceM3PerPa

    PressureRiseRatePaPerSec
    PressureReleaseRatePaPerSec

    FluidDryBoilingPointC
    FluidWetBoilingPointC
    FluidWaterContamination01
    FluidThermalMassJPerC
    FluidCoolingWPerC
}
```

All vehicle-specific persistent calibration enters PhysicsConfigHash.

## 4. Runtime state

```
FTABrakeHydraulicState
{
    FrontCircuitPressurePa
    RearCircuitPressurePa

    FrontTargetPressurePa
    RearTargetPressurePa

    FluidTemperatureC
    VaporFraction01

    FrontCircuitHealth01
    RearCircuitHealth01
}
```

Per wheel:

```
FTABrakeCornerActuationState
{
    RequestedPressurePa
    EffectivePressurePa
    AbsMode
    AbsDuty01
}
```

## 5. Pedal to pressure

Driver pedal input remains normalized in the control layer.

Conceptual pedal force:

```
F_pedal = pedal01 × PedalForceMaxN
```

Boosted master force:

```
F_master = F_pedal × BoosterGain
```

Ideal pressure request:

```
P_master = F_master / A_master
```

Clamp to physically authored system limit:

```
P_request = clamp(P_master, 0, P_max)
```

No wheel-speed dependence belongs here.

## 6. Brake bias

Initial fixed-bias implementation:

```
P_front_target = P_request
P_rear_target  = P_request × RearPressureRatio
```

or equivalent bias representation derived from `FrontBias01`.

The chosen runtime representation must preserve exact front/rear axle torque intent.

Future upgrades:
- proportioning valve;
- load-sensitive valve;
- electronic brakeforce distribution.

## 7. Hydraulic dynamics

Pressure cannot teleport.

Per circuit, use a bounded discrete pressure update:

```
DeltaP = clamp(
    P_target - P,
    -PressureReleaseRatePaPerSec * DeltaTimeSec,
    +PressureRiseRatePaPerSec * DeltaTimeSec
)
P_next = clamp(P + DeltaP, 0, MaxSystemPressurePa)
```

The pressure difference and `DeltaP` are in Pa. Each pressure rate is in Pa/s
and must be multiplied by `DeltaTimeSec` (s) before it can bound a pressure
change. For a zero timestep, pressure is unchanged. Inputs must be finite;
pressure targets and rates must be non-negative, and pressure targets are
bounded by the authored system pressure limit. This is a per-step update,
not a pressure derivative.

A compliance/volume model may replace the rate limit later without changing the public interface.

Circuit health scales pressure authority:

```
P_effective = P × CircuitHealth01
```

A ruptured circuit may decay pressure toward zero using a leak rate rather than instantly disappearing.

## 8. Pressure to clamp force

For a caliper piston area:

```
F_piston = P_effective × A_piston
```

Clamp geometry factor is explicit and data-driven.

Brake friction torque seed:

```
T_raw =
F_clamp
× mu_pad
× R_effective
```

The exact piston/multi-piston factor convention must be frozen in one helper and covered by dimensional tests.

## 9. Final available wheel brake torque

The current thermal/hub factors remain authoritative:

```
T_available =
T_hydraulic_raw
× HubBrakeEfficiency
× ThermalFadeFactor
× WearTorqueFactor
```

Actual applied torque is additionally limited by direction/sign so braking cannot numerically drive a stopped wheel backwards.

## 10. Brake fluid temperature

Heat sources:
- calibrated fraction of caliper/disc energy conducted into fluid;
- ambient/engine-bay heat later.

Cooling:
```
P_cool =
CoolingWPerC
× max(0, T_fluid - T_ambient)
```

Dry/wet boiling point:

```
T_boil =
lerp(DryBoilingPoint, WetBoilingPoint, WaterContamination01)
```

## 11. Vapor/boiling consequence

Above boiling onset, vapor fraction grows smoothly.

Vapor increases effective hydraulic compliance and reduces pressure transfer.

First implementation:

```
PressureTransfer01 =
1 - VaporFraction01 × BoilPressureLossAtFullVapor
```

No binary “brakes off” threshold.

Pedal travel modelling can later derive naturally from compliance.

## 12. Circuit topology

Prototype recommendation: diagonal split.

Circuit A:
- front-left;
- rear-right.

Circuit B:
- front-right;
- rear-left.

Reason:
a single circuit failure still retains one front brake plus one rear brake and creates realistic asymmetric/yaw behavior.

Vehicle content may later choose:
- front/rear split;
- diagonal split;
- motorsport independent front/rear circuits.

## 13. Damage integration

Damage consumers should target physical circuit state.

Examples:

```
BrakeLineFrontLeft rupture
→ circuit leak area
→ pressure decay
→ reduced corner brake torque

MasterCylinder damage
→ reduced pressure authority / internal bypass

CaliperMount damage
→ existing hub/caliper mechanical efficiency path
```

Do not add a second generic brake-damage multiplier.

## 14. ABS inputs

Per corner:
- wheel angular speed;
- effective rolling radius;
- estimated vehicle/reference speed;
- longitudinal tire slip ratio;
- wheel angular deceleration;
- contact validity;
- brake pressure;
- surface/load information for diagnostics only.

ABS does not read a magic “grip available” scalar to set wheel force.

## 15. ABS state machine

Per wheel:

```
Inactive
Build
Hold
Release
```

Core trigger is excessive negative slip under braking plus wheel deceleration evidence.

Example initial thresholds are calibration seeds:
- enter modulation around braking slip magnitude 0.15–0.20;
- target region around 0.10–0.15;
- hysteresis mandatory;
- low-speed disable/blend below a few m/s.

No single threshold is treated as universal physics.

## 16. ABS modulation

ABS outputs a pressure authority/duty request.

```
P_corner_target =
P_driver_or_EBD
× ABS_modulation01
```

Pressure then still obeys hydraulic rise/release rates.

This prevents impossible instantaneous pressure cycling.

## 17. Reference-speed problem

Wheel speed cannot always be trusted during four-wheel braking.

Initial implementation:
- use max/filtered non-pathological wheel-derived speed with chassis longitudinal estimate;
- reject airborne/outlier wheels;
- preserve a decaying vehicle-speed observer during simultaneous lock tendency.

Future:
- inertial observer;
- driveline/GPS-like simulated sensors.

## 18. Split-mu behavior

ABS is independent per wheel.

On split friction:
- low-mu wheel releases earlier;
- high-mu wheel may retain higher pressure;
- resulting left/right longitudinal force imbalance naturally creates yaw;
- later ESC may counter yaw through separate intervention.

ABS itself should not directly cancel yaw.

## 19. Handbrake / parking brake

Separate actuator path.

Prototype:
- rear mechanical brake command;
- bypasses hydraulic service-brake pressure;
- thermal/wear still affects friction if it uses the same rotor/caliper.

A motorsport hydraulic handbrake can be a later authored actuator.

## 20. Telemetry

Global:
- pedal input;
- master pressure;
- front/rear circuit pressure;
- fluid temperature;
- vapor fraction;
- circuit health.

Per corner:
- requested/effective pressure;
- raw hydraulic torque;
- applied brake torque;
- ABS mode;
- ABS modulation;
- wheel slip ratio.

## 21. Required deterministic tests

BRAKE-HYD-001 pedal pressure dimensional sanity.
BRAKE-HYD-002 zero pedal → zero pressure.
BRAKE-HYD-003 pressure rise/release bounded by rates.
BRAKE-HYD-004 front/rear bias conservation.
BRAKE-HYD-005 hydraulic torque proportional to pressure.
BRAKE-HYD-006 circuit-health pressure loss.
BRAKE-HYD-007 diagonal line failure creates asymmetric braking.
BRAKE-HYD-008 hot fluid increases vapor/reduces pressure transfer.
BRAKE-HYD-009 cooling lowers fluid temperature toward ambient.
BRAKE-ABS-001 no modulation without braking.
BRAKE-ABS-002 lock tendency triggers release.
BRAKE-ABS-003 recovered slip transitions hold/build.
BRAKE-ABS-004 low-speed behavior does not chatter.
BRAKE-ABS-005 split-mu wheels modulate independently.
BRAKE-ABS-006 fixed input/state produces deterministic output.
BRAKE-INT-001 100–0 stopping trace.
BRAKE-INT-002 repeated fade/boil stopping trace.
BRAKE-INT-003 line rupture stopping/yaw trace.

## 22. Acceptance criteria

The subsystem is acceptable when:
- brake torque comes from pressure/geometry, not direct normalized torque;
- ABS only modulates actuator pressure;
- thermal fade/wear remains applied exactly once;
- line/circuit damage produces traceable pressure loss;
- split-mu behavior is independent per wheel;
- no normal braking path reverses a stopped wheel;
- all authored handling-critical fields hash;
- telemetry reports the exact applied pressure/torque state.

## 23. Non-goals v0.1

Deferred:
- flexible hose volumetric expansion by temperature/age;
- detailed pedal travel/master-cylinder piston position;
- caliper seal rollback;
- rotor coning/warping;
- pad knockback;
- brake-by-wire redundancy;
- regenerative blend;
- carbon ceramic material maps;
- full ESC yaw controller.

## 24. Promotion rule

This subsystem may be implemented in isolation after current Proof-of-Physics source closure, but must not replace the canonical direct brake actuation path until the UE 5.8 executable baseline is captured and compared.
