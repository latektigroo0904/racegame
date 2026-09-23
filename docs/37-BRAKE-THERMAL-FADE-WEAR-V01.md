# Brake Thermal, Fade & Wear v0.1

Status: source-implemented, UE-build-unverified  
Updated: 2026-09-23

## 1. Purpose

The wheel brake model now has persistent thermal and wear state rather than a permanently available fixed brake torque.

Canonical brake capacity:

```
available brake torque
=
authored max brake torque
× hub brake efficiency
× brake thermal fade factor
× brake wear torque factor
```

This gives independent mechanical consequences for:

- hub/caliper mounting damage;
- brake overheating;
- cumulative brake wear.

## 2. Per-wheel configuration

`FTABrakeThermalConfig` contains:

- thermal mass;
- cooling conductance to ambient;
- ambient temperature;
- friction-energy heat fraction;
- fade start temperature;
- fade end temperature;
- minimum thermal torque factor;
- wear-energy capacity;
- end-of-life torque loss.

TA-P01 currently authors one prototype wheel/brake calibration and compiles it to all four wheels.

Future content may split front/rear or per-corner brake packages without changing solver semantics.

## 3. Per-wheel persistent state

`FTABrakeThermalState` stores:

- brake temperature;
- wear fraction;
- thermal torque factor;
- wear torque factor.

State persists across fixed-step simulation.

## 4. Heat generation

For each wheel:

```
friction power
=
applied brake torque
× average absolute wheel angular speed
```

Then:

```
heat power
=
friction power
× heat fraction
```

At zero wheel speed, holding brake pressure does not create friction heat.

## 5. Cooling

Cooling power is proportional to temperature above ambient:

```
cooling power
=
cooling coefficient
× max(0, T_brake - T_ambient)
```

The current model does not cool below ambient.

Future extensions may add:

- vehicle-speed airflow;
- brake ducting;
- rain/water cooling;
- disc material/radiation.

## 6. Temperature integration

Net heat energy per step:

```
E_net
=
(P_heat - P_cooling) × dt
```

Temperature change:

```
ΔT
=
E_net / thermal mass
```

## 7. Thermal fade

Below fade-start temperature:

```
thermal factor = 1
```

Between fade start/end, the factor interpolates toward the authored minimum.

At or above fade-end temperature:

```
thermal factor = minimum fade factor
```

The current default minimum is 0.35.

## 8. Wear

Wear accumulates from generated brake heat energy:

```
wear01 += generated heat energy / wear-energy capacity
```

Wear is clamped to [0,1].

Available torque loss from wear is linear in this first model:

```
wear torque factor
=
1 - wear01 × end-of-life torque loss
```

Default end-of-life torque loss is 25%.

## 9. Runtime integration

Per wheel, the vehicle solver performs:

1. solve tire force;
2. apply hub drive efficiency;
3. calculate current brake thermal/wear factor;
4. apply hub brake efficiency × thermal/wear factor to max brake torque;
5. integrate wheel angular speed;
6. estimate brake friction work from effective torque and average wheel speed;
7. update brake temperature and wear;
8. continue chassis/tire integration.

Hub brake damage and thermal fade are therefore multiplied once in the same torque-capacity path.

## 10. Vehicle authoring

`FTAPrototypeWheelDefinition` now authors:

- max brake torque;
- brake thermal mass;
- brake cooling;
- ambient temperature;
- heat fraction;
- fade start/end;
- minimum fade torque factor;
- wear-energy capacity;
- end-of-life wear torque loss.

All effective brake calibration is included in `PhysicsConfigHash`.

Invalid calibration, including a reversed/non-increasing fade range, is rejected at vehicle compilation.

## 11. Telemetry

Per wheel telemetry now records:

- brake temperature;
- thermal fade torque factor;
- wear fraction;
- wear torque factor.

These channels are:

- exported in vehicle CSV;
- selectable as regression metrics;
- suitable for future braking/endurance proving-ground baselines.

## 12. Source-level regressions

Coverage includes:

### Brake thermal module
- default config validation;
- friction work raises temperature;
- high temperature causes fade;
- fade respects configured minimum;
- hot brakes cool toward ambient;
- friction energy accumulates wear;
- end-of-life wear reduces torque capacity.

### Vehicle runtime
- same wheel and brake input: hot/faded brake slows less than cold brake;
- same wheel and brake input: fully worn brake slows less than fresh brake;
- existing hub brake-efficiency behavior remains in the same capacity chain.

### Vehicle definition
- brake calibration compiles into wheel runtime;
- brake calibration changes physics hash;
- invalid fade range blocks compilation.

### Telemetry/regression
- temperature/fade/wear state maps to telemetry;
- CSV exposes brake channels;
- regression envelopes can evaluate brake temperature and fade.

## 13. Known simplifications

Current model does not yet include:

- separate disc and pad temperatures;
- front/rear brake bias authoring;
- hydraulic pressure/booster/ABS;
- pad compound curves;
- boiling brake fluid;
- rotor thickness/cracking/warping;
- speed-dependent cooling airflow;
- brake duct tuning;
- regenerative braking blending.

These are future layers. The current model establishes a persistent, energy-based brake consequence without adding hidden per-car handling branches.
