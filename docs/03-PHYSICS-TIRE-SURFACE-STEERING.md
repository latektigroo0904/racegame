# Tire, Surface & Steering Simulation — v0.1

## Principle
The tire contact system is the central handling model. No constant "grip" scalar controls the car.

Per-wheel force depends on:
- vertical load;
- slip ratio;
- slip angle;
- camber;
- compound;
- pressure;
- temperature;
- wear;
- road material;
- road temperature;
- water/snow/ice/dirt;
- damage.

## Tire state
```
AngularVelocity
VerticalLoad
SlipRatio
SlipAngle
CamberAngle
SurfaceTemperature
CarcassTemperature
InternalAirTemperature
Pressure
TreadDepth
Wear
WaterFilmDepth
Damage
```

## Slip ratio
```
kappa = (Vwheel - Vground) / max(abs(Vground), Vmin)
```
Use a separate low-speed friction regime near standstill and blend into full slip equations to avoid numerical instability.

## Slip angle
```
alpha = atan2(Vlateral, abs(Vlongitudinal))
```

## Lateral response
Near zero slip:
```
Fy ≈ C_alpha * alpha
```
Then transition progressively through peak grip to saturation. No binary grip/loss-of-grip behaviour.

## Combined slip
Longitudinal and lateral force share a finite envelope:
```
(Fx/FxMax)^n + (Fy/FyMax)^n <= 1
```
Shape parameters vary by tire type.

## Load sensitivity
Effective friction does not scale linearly with vertical load. This is required for realistic weight transfer, aero behaviour and anti-roll effects.

## Camber
Camber contributes lateral force but excessive camber reduces straight-line contact efficiency. Store camber-thrust and contact-loss behaviour per tire family.

## Pressure
Track:
- cold pressure;
- internal air temperature;
- hot pressure;
- leak rate;
- effective volume.

Low pressure increases deflection/heat/rolling resistance and sidewall risk. High pressure sharpens response but reduces compliance/contact behaviour.

## Thermal model
At minimum:
- tread/surface;
- carcass;
- internal air.

Heat sources:
- longitudinal slip;
- lateral slip;
- carcass flex;
- brake radiation/conduction.

Cooling:
- airflow;
- road conduction;
- rain/water.

Each compound has cold, optimal and overheating regions.

## Wear
```
WearRate =
BaseWear
* SlipFactor
* TemperatureFactor
* LoadFactor
* SurfaceFactor
```

Track tread depth for road tires. Low tread strongly increases standing-water sensitivity.

## Failure states
```
Healthy
SlowLeak
RapidLeak
Flat
Destroyed
```

Wheel/sidewall impacts can cause punctures independently of tread wear.

## Surface contact
A road cell exposes:
```
Material
DryFriction
WetFriction
Roughness
Temperature
Wetness
WaterDepth
SnowDepth
IceFraction
Dirt/Rubber
Drainage
```

Surface examples:
- fresh asphalt;
- polished asphalt;
- concrete;
- cobbles;
- gravel;
- dirt;
- mud;
- sand;
- snow;
- ice.

## Water
Rain generates a local water film controlled by precipitation, drainage, slope and evaporation.

Standing-water behaviour must depend on:
- water depth;
- vehicle speed;
- tire pressure;
- tread depth;
- tire width;
- load.

Aquaplaning is progressive and per-wheel; it is not a global weather modifier.

## Road evolution
Optional advanced states:
- rubbered racing line;
- drying line;
- puddle accumulation;
- dirt carried onto asphalt;
- snow compaction;
- ice formation.

## Steering model
```
SteeringWheel
→ SteeringRatio
→ Rack
→ TieRod
→ Knuckle
→ Wheel
```
Damage can alter steering geometry.

## Force feedback
Primary FFB derives from:
- tire aligning moment;
- caster/geometry;
- steering-rack forces;
- suspension kickback;
- friction;
- mechanical damage.

Avoid canned effects as the main steering force source.

## Next calibration work
- choose first mathematical tire curve family;
- define reference street tire;
- define wet-road coefficients;
- validate against analytical and published reference data;
- build skidpad/braking regression tests.
