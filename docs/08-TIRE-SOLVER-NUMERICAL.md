# Tire Solver Numerical Specification v0.1

Status: engineering baseline; calibration values are provisional.

## 1. Solver choice
Use a brush-inspired semi-empirical force model for the first playable solver rather than implementing a full Pacejka/Magic Formula immediately. Reasons: coefficients remain interpretable, combined-slip behaviour can be controlled explicitly, low-speed behaviour is easier to stabilize, and the architecture remains replaceable by measured-data tire models later.

The public solver API must not expose the internal curve family. This permits future per-tire backends.

## 2. State and units
Per wheel, SI units internally:
- Fz [N]
- longitudinal velocity Vx [m/s]
- lateral velocity Vy [m/s]
- wheel angular velocity omega [rad/s]
- effective rolling radius Re [m]
- slip ratio kappa [-]
- slip angle alpha [rad]
- camber gamma [rad]
- pressure p [kPa]
- surface/carcass/internal-air temperatures [deg C]
- tread depth [mm]
- water depth [mm]

## 3. Slip calculation
For normal-speed operation:

kappa = (omega * Re - Vx) / max(abs(Vx), Vref)

alpha = atan2(-Vy, max(abs(Vx), Vref))

Use Vref = 0.5 m/s initially. Below 2 m/s blend toward a static/creep friction model; complete transition to dynamic tire equations by 5 m/s. These thresholds are tuning values, not physical constants.

Clamp only pathological numerical inputs. Do not clamp normal player-generated slip before force evaluation.

## 4. Reference street tire
Initial TA-P01 reference:
- size class: 245/40 R18 equivalent
- unloaded radius: ~0.327 m
- cold pressure: 230 kPa
- new tread: 7.5 mm
- reference load Fz0: 3,500 N
- nominal dry peak mu at reference condition: 1.05
- nominal wet peak mu: 0.72 before standing-water penalties
- optimum surface-temperature band: 65–90 C

These are simulation calibration seeds, not claims about a specific commercial tire.

## 5. Load sensitivity
Use an initial power law:

mu(Fz) = mu0 * (Fz/Fz0)^(-lambda)

Initial lambda = 0.08 for the reference street tire.

Maximum pure force scale:

Fmax = mu(Fz) * Fz

This gives diminishing grip per unit load as vertical load rises.

## 6. Pure-slip force curves
Use normalized smooth saturation:

Sx = Cx * kappa / max(FxMax, epsilon)
Sy = Cy * tan(alpha) / max(FyMax, epsilon)

For prototype curve function:

G(s) = s / (1 + abs(s)^q)^(1/q)

with q initially 2.0.

Fx_pure = FxMax * G(Sx)
Fy_pure = FyMax * G(Sy)

Cx and Cy are effective longitudinal/cornering stiffnesses stored in the tire definition and adjusted for load, pressure and temperature.

## 7. Combined slip
Compute normalized demand:

d = sqrt((Fx_pure/FxMax)^2 + (Fy_pure/FyMax)^2)

If d <= 1, retain pure forces. If d > 1, scale both by 1/d initially.

Later introduce anisotropic p-norm envelopes and measured combined-slip curves. The first implementation prioritizes stability and inspectability.

## 8. Camber thrust
Add a bounded camber contribution before final combined-slip saturation:

Fy_camber = Cgamma * gamma * LoadScale

Cgamma is tire-specific. Excess camber additionally reduces longitudinal and peak lateral efficiency through a contact-patch modifier.

## 9. Aligning moment
First model:

Mz = -Fy * pneumaticTrail

Pneumatic trail decreases with increasing normalized lateral slip and collapses beyond peak slip. Mechanical trail/caster effects belong to the steering geometry layer, not the tire model.

## 10. Rolling resistance
Frr = Crr * Fz * sign(Vx)

Crr depends on pressure, temperature, surface and damage. At very low speed use a smooth sign approximation to avoid chatter.

## 11. Temperature
Three lumped thermal masses:
- tread/surface
- carcass
- internal air

Slip heating power approximation:

Qslip = abs(Fx * Vslip_x) + abs(Fy * Vslip_y)

Only a calibrated fraction enters each thermal mass. Carcass flex adds load/speed-dependent heat. Cooling includes air convection, road conduction and strong water-film cooling.

Grip multiplier is a smooth curve around each compound's optimum band. Overheating also accelerates wear.

## 12. Pressure
Use an ideal-gas-inspired update based on absolute temperature with a compliance correction rather than assuming constant tire volume exactly.

Pressure affects:
- stiffness Cx/Cy
- rolling resistance
- carcass heating
- contact efficiency
- hydroplaning resistance
- impact/sidewall damage risk.

## 13. Wear
Wear energy driver:

Ewear += (abs(Fx*Vslip_x) + abs(Fy*Vslip_y)) * dt

Wear rate multiplies this by compound, temperature, load and surface factors. Track both tread depth and irreversible thermal degradation.

## 14. Surface contact model
A queried road sample returns:
- material ID
- dry friction baseline
- wet friction baseline
- roughness
- macrotexture/drainage
- temperature
- wetness [0..1]
- water depth [mm]
- snow depth
- ice fraction
- loose-material depth
- rubber/dirt contamination.

Initial dry reference multipliers:
- fresh asphalt: 1.00
- polished asphalt: 0.90
- concrete: 0.96
- cobble: 0.82
- compact gravel: 0.68
- loose gravel: 0.58
- dirt: 0.55
- mud: dynamic, typically 0.30–0.50
- packed snow: ~0.30
- ice: ~0.10

All values are calibration seeds and must be validated.

## 15. Wetness and standing water
Separate damp/wet friction loss from hydrodynamic lift.

Water state progression:
dry -> damp -> wet film -> standing water

Road drainage is driven by rainfall input, local slope, drainage coefficient, evaporation and runoff. Do not model every water particle.

## 16. Aquaplaning v0.1
Do not use a binary threshold. Calculate a per-wheel hydro-lift fraction H in [0,1] from:
- speed
- water depth
- tread depth
- tire pressure
- vertical load
- tire width
- drainage effectiveness.

Conceptual onset-speed seed may use the known pressure-dependent hydroplaning relationship only as a calibration sanity check; gameplay force reduction uses the continuous H model.

Effective road-supported load:

Fz_road = Fz * (1 - H)

Tire friction forces use Fz_road. H rises smoothly, with deeper water and worn tread lowering onset speed. Full hydroplaning remains uncommon unless conditions are severe.

## 17. Water displacement force
Standing water also adds longitudinal drag and a steering disturbance when left/right wheels encounter different depths. This is separate from grip loss and is required for believable puddle strikes.

## 18. Steering/FFB coupling v0.2
Steering rack torque is assembled from:
- tire aligning moments
- mechanical trail/caster
- scrub radius forces
- steering ratio
- rack friction/damping
- suspension kickback
- damaged geometry.

FFB output is derived from rack torque, then passed through device-safe scaling, clipping protection and optional smoothing. Artificial road texture may be layered separately but must never replace physical rack torque.

## 19. Required regression tests
TIRE-001: straight-line acceleration slip sweep.
TIRE-002: locked-wheel braking sweep.
TIRE-003: constant-radius lateral sweep.
TIRE-004: combined braking/cornering.
TIRE-005: load sensitivity at 0.5x/1x/1.5x/2x reference load.
TIRE-006: cold/optimal/overheated comparison.
TIRE-007: pressure sweep.
TIRE-008: tread-depth wet braking.
TIRE-009: standing-water speed sweep.
TIRE-010: split-water-depth yaw disturbance.
TIRE-011: zero-to-low-speed stability.
TIRE-012: FFB rack-torque continuity.

## 20. Performance target
All four player tires, including contact queries and thermal updates, should target substantially below 0.5 ms average CPU time on the prototype reference desktop at the selected physics rate. Profile before optimizing; vectorization and batched surface queries are later options.

## 21. Open calibration work
- determine Cx/Cy/Cgamma from reference literature/test data;
- calibrate peak-slip locations;
- validate wet multipliers;
- validate aquaplaning response;
- tune pneumatic-trail decay;
- establish deterministic regression tolerances.
