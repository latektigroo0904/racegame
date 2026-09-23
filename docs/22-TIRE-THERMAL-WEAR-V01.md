# Tire Thermal / Pressure / Wear Model v0.1

Status: implemented prototype baseline
Updated: 2026-09-23

## Purpose
Make tire state persistent and physically consequential rather than treating grip as a fixed coefficient.

Runtime state:
- surface temperature;
- carcass temperature;
- internal air temperature;
- pressure;
- tread depth;
- wear;
- thermal degradation;
- physical damage.

## Heat generation
Prototype dissipated slip power:

```
P_slip =
abs(Fx * V_slip_longitudinal)
+ abs(Fy * V_lateral)
```

Rolling contribution:

```
P_roll =
abs(F_rolling * V_longitudinal)
```

Only calibrated fractions of these terms enter the tire thermal model.

## Two-mass thermal model
Thermal layers:
1. tread/surface;
2. carcass.

Heat exchange:
```
Surface → Carcass
Carcass → Ambient
```

The internal tire air temperature follows the carcass with a configurable first-order time constant.

## Pressure
Pressure is updated from internal-air temperature using an absolute-pressure gas-law approximation.

```
P_abs,current =
P_abs,reference *
T_current / T_reference
```

Gauge pressure is recovered by subtracting atmospheric pressure.

This is a sealed-volume approximation; tire-volume growth is deferred.

## Temperature grip
Grip factor:
- reduced when cold;
- peaks near configured optimal surface temperature;
- falls again above the hot range.

The factor multiplies the same effective friction calculation used by force capacity and the main tire solve.

## Pressure grip
Deviation from reference pressure reduces the pressure grip factor.

This is currently a symmetric first-order calibration model.

Future measured tire data may replace it.

## Wear
Wear integrates dissipated energy:

```
Wear += DissipatedEnergy / WearEnergyCapacity
```

Tread depth then moves from new-tire depth toward configured minimum depth.

Consequences:
- modest direct wear grip loss;
- shallower tread increases standing-water aquaplaning susceptibility.

## Thermal degradation
Above a configured high-temperature threshold:
- irreversible thermal degradation accumulates;
- grip is permanently reduced.

This is separate from reversible hot-tire grip loss.

## Current force multiplier stack
Effective friction currently includes:
- load sensitivity;
- surface material/wetness/ice;
- temperature;
- pressure;
- wear;
- tire damage;
- thermal degradation.

## Implemented tests
Tests cover:
- cold vs optimal vs overheated grip factor;
- sustained slip heating surface;
- heat propagation into carcass;
- internal-air warming raising pressure;
- dissipated energy increasing wear;
- tread-depth reduction.

## Calibration status
All thermal masses, conductances, wear-energy budgets and grip multipliers are engineering seeds.

They are not claimed as measured tire-manufacturer data.

## Future work
- compound-specific data;
- carcass flex heat;
- speed-dependent convection;
- road-temperature heat flux;
- rain/water cooling;
- tire vertical stiffness;
- flat spots;
- puncture/pressure loss;
- blowout;
- tread blocks/compound aging;
- data-import backend for measured tire curves.

## Acceptance
v0.1 is accepted when tire use can create a repeatable causal chain:

```
slip/load
→ heat
→ carcass/internal-air temperature
→ pressure + temperature grip change
→ wear
→ tread loss
→ altered dry/wet behavior
```
