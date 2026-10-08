# Weather & Dynamic Surface Runtime v0.1

Updated: 2026-10-08
Status: implementation-ready design

## 1. Ownership
Weather owns atmospheric conditions.
Surface runtime owns accumulated road state.
Tire solver owns tire forces.
Aerodynamics owns air-relative forces.

## 2. Atmosphere sample
Per world cell/zone:
- ambient temperature C;
- pressure Pa;
- relative humidity;
- precipitation rate mm/h;
- wind vector m/s;
- visibility/fog;
- solar/radiative proxy.

## 3. Air density
Derive from pressure/temperature with bounded approximation.
Humidity correction later.

Aero receives density and wind through environment input.

## 4. Surface state
Per surface cell:
- material;
- temperature;
- wetness;
- water depth mm;
- snow depth;
- ice fraction;
- loose material;
- rubber;
- dirt;
- drainage.

## 5. Rain accumulation
Water budget:
precipitation
- drainage
- runoff
- evaporation
- tire displacement approximation later
= water-depth change.

Do not instantiate water particles for road film.

## 6. Drainage
Depends on:
- authored drainage01;
- macrotexture;
- slope;
- local depression/puddle capacity;
- road class/material.

## 7. Evaporation
Depends on:
- air/surface temperature;
- wind;
- humidity;
- solar proxy.

First model can be calibrated lumped rate.

## 8. Puddles
Localized depression cells have larger retention capacity.
Standing water depth directly feeds tire hydro model.

## 9. Drying line
Traffic/tire passage may accelerate removal on heavily used lane regions.
Advanced feature; keep optional.

## 10. Road temperature
Energy inputs:
- ambient convection;
- solar;
- precipitation cooling;
- ground thermal inertia.

Use low-frequency update.

## 11. Snow/ice
Later state transitions:
rain/snow precipitation
→ snow accumulation
→ compaction
→ melt water
→ refreeze/ice.

Not required for first wet-weather prototype.

## 12. Contamination
Surface zones can accumulate:
- dirt;
- gravel;
- oil-like contamination later;
- rubber.

Tire solver consumes normalized state.

## 13. Weather fronts
World weather can be spatially varying.
Use interpolated cells/volumes, not one global rain bool.

## 14. Time
Day/night influences:
- temperature;
- solar;
- visibility;
- traffic demand.

Physics uses explicit sampled values, not visual sky state.

## 15. Networking
Server authoritative for gameplay-relevant weather/surface state.
Clients interpolate visual effects.

Surface state may replicate at coarse cells/deltas.

## 16. Telemetry
- precipitation;
- wind;
- air density;
- road temperature;
- wetness;
- water depth;
- hydro fraction per wheel;
- drainage/runoff rates.

## 17. Tests
WEATHER-001 no rain stable dry surface.
WEATHER-002 rain increases water depth.
WEATHER-003 drainage reduces water.
WEATHER-004 higher drainage dries faster.
WEATHER-005 evaporation cannot create negative water.
WEATHER-006 puddle retains more.
WEATHER-007 wind sample reaches aero unchanged.
WEATHER-008 water depth reaches tire surface query unchanged.
WEATHER-009 deterministic fixed-step/low-frequency integration.
WEATHER-010 render VFX state does not alter physics.

## 18. MVP
- dry;
- damp;
- rain;
- standing water;
- fog/visibility;
- wind;
- day/night temperature influence.
