# Vehicle Calibration Authoring v2

Status: source-implemented, build-unverified  
Updated: 2026-09-23

## 1. Goal

Torque Atlas vehicle behavior must be driven by versioned vehicle content rather than hidden C++ solver defaults.

For TA-P01, all currently handling-, powertrain-, thermal- and crash-relevant vehicle-specific coefficients are now either:

1. explicitly authored in `UTAVehicleDefinition`; or
2. deterministically derived from authored physical geometry.

World/scenario state such as global chassis gravity remains outside the vehicle asset by design.

## 2. Tire authoring

`FTAPrototypeTireDefinition` now authors the complete current `FTATireRuntimeConfig` calibration surface:

### Geometry/load
- unloaded radius;
- reference load;
- dry peak friction;
- load sensitivity.

### Force generation
- longitudinal stiffness;
- cornering stiffness;
- camber stiffness;
- saturation exponent;
- pneumatic trail;
- rolling resistance.

### Pressure/vertical compliance
- reference pressure;
- reference pressure temperature;
- radial stiffness;
- progressive radial stiffness;
- radial damping;
- max radial deflection;
- pressure/stiffness exponent.

### Tread/grip
- new/minimum tread;
- optimal/hot temperatures;
- cold/hot grip multipliers;
- pressure grip sensitivity;
- end-of-life grip loss.

### Thermal/wear
- surface/carcass thermal mass;
- conductances;
- internal air time constant;
- slip/rolling heat fractions;
- wear-energy capacity;
- thermal degradation threshold/rate.

### Slip/hydro
- reference slip velocity;
- dynamic blend range;
- hydro onset speed;
- hydro reference water depth.

The compiler no longer relies on the runtime struct defaults for these values.

## 3. Powertrain authoring

The vehicle asset now explicitly controls:

### Engine
- crank inertia;
- constant/linear/quadratic friction;
- idle/redline/limiter;
- stall and combustion-start speeds;
- starter torque/max speed;
- idle-control gain/max torque;
- ordered torque curve.

The previously declared `FTAPrototypeDrivetrainDefinition()` constructor is now defined and populates the TA-P01 default torque curve and gear ratios.

The authored torque curve is copied into `FTAEngineRuntimeConfig::TorqueCurve` and is therefore consumed by the real engine solver.

### Clutch
- torque capacity;
- coupling stiffness;
- thermal mass/cooling;
- ambient/fade temperatures;
- wear-energy capacity.

### Gearbox/driveline
- forward ratios;
- reverse ratio;
- final drive;
- mechanical efficiency;
- driveline torsional stiffness/damping.

## 4. Engine thermal/cooling authoring

Separate authoring structures now control:

### Engine thermal model
- ambient and initial coolant temperature;
- effective thermal mass;
- base/full-load heat generation;
- cooling capacity;
- derate start/full temperature;
- minimum thermal torque factor;
- damage start/rate.

### Radiator/coolant damage model
- initial coolant mass;
- puncture energy;
- full-leak energy;
- max leak area;
- leak mass-flow calibration;
- minimum airflow efficiency.

These values now feed the same runtime used by crash damage and thermal derate.

## 5. Suspension stop authoring

Front and rear suspension assets now explicitly author:

- bump-stop rate;
- droop-stop rate.

Travel positions continue to derive from authored min/max suspension travel.

## 6. Derived kinematic LUT

`FTASuspensionRuntimeConfig::KinematicSamples` is deliberately **not** hand-authored.

At compile time, the vehicle compiler derives 17 deterministic travel samples independently for:

- front left;
- front right;
- rear left;
- rear right.

Each sample contains:

- travel;
- wheel-center offset;
- camber;
- toe;
- motion ratio.

The data is produced by the exact double-wishbone/five-link solver using the authored hardpoints.

This preserves one geometry source of truth:

```
authored hardpoints
→ exact solver
→ derived LUT
```

not:

```
authored hardpoints
+ separately authored LUT
```

The LUT is also included in configuration identity.

## 7. Vehicle structure gravity ownership

Compiled vehicle structure nodes are chassis-local internal deformation state.

Therefore:

```
compiled vehicle structure gravity = 0
```

World gravity remains on the rigid chassis.

The generic structure solver still supports gravity for standalone use.

This prevents:

```
chassis gravity
+
local structure gravity
= duplicate gravity / artificial relative sag
```

## 8. Validation

The compiler rejects invalid:

- tire force/thermal/tread/slip/hydro ranges;
- non-increasing or non-finite engine torque curves;
- engine speed/friction/starter/idle-control ranges;
- clutch fade/thermal/wear ranges;
- gearbox ratios/efficiency;
- driveline compliance;
- engine thermal/derate data;
- cooling leak/damage thresholds;
- suspension spring/damper/stop calibration.

## 9. Configuration hash coverage

`PhysicsConfigHash` now includes the complete effective current vehicle calibration for:

- tire;
- wheel;
- drivetrain layout;
- engine + torque curve;
- engine thermal;
- cooling/radiator;
- clutch;
- gearbox;
- driveline;
- suspension force model;
- derived suspension LUTs;
- steering;
- exact hardpoint geometry;
- structural/damage configuration.

## 10. Version policy

Current defaults:

```
SchemaVersion      = 2
PhysicsVersion     = 2
DamageModelVersion = 2
```

Policy:

- current version: compile normally;
- older positive version: compile with explicit legacy/review warning;
- newer-than-runtime version: reject;
- non-positive version: reject.

Trusted regression baselines must be regenerated/reviewed after version-affecting physics changes.

## 11. Source-level tests

New tests cover:

- full tire coefficient compilation;
- full engine/clutch/thermal/driveline compilation;
- real authored torque-curve solver use;
- hidden-coefficient hash sensitivity;
- invalid tire blend rejection;
- invalid torque-curve ordering rejection;
- suspension stop compilation/hash;
- cooling compilation/hash;
- invalid cooling threshold rejection;
- derived front/rear LUT generation;
- LUT interpolation versus direct solvers;
- chassis-owned gravity versus zero local structure gravity;
- v2 defaults;
- legacy warning;
- unsupported future-version rejection.

No UE build/test pass is claimed until UHT/UBT/Automation runs.
