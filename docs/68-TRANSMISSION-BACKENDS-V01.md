# Transmission Backend Architecture v0.1

Updated: 2026-10-08
Status: implementation-ready design

## 1. Goal

One powertrain interface must support:
- manual;
- torque-converter automatic;
- DCT;
- sequential/manual automated;
- CVT;
- EV single/multi-speed reduction.

The transmission backend transforms torque/speed through physical states. Shift policy is separate.

## 2. Common interface

Input:
- input-shaft speed/torque;
- output-shaft speed/reaction;
- requested gear/ratio;
- clutch/lockup requests;
- fluid/thermal state;
- timestep.

Output:
- transmitted torque;
- input/output reaction torque;
- effective ratio;
- slip power;
- selected/engaged state;
- thermal energy;
- failure state.

## 3. Manual

Existing convention remains:

```
ratio = omega_input / omega_output
```

Clutch is separate from gearbox.

Add later:
- synchronizer;
- shift sleeve;
- dog engagement;
- gear damage;
- missed shift.

## 4. Torque-converter automatic

Path:

engine
→ torque converter
→ planetary/ravigneaux-like reduced gear representation
→ final drive

Torque converter state:
- pump speed;
- turbine speed;
- speed ratio;
- torque ratio;
- lockup clutch.

Use data curves:
- capacity factor;
- torque ratio vs speed ratio.

Slip energy becomes transmission fluid heat.

Lockup is a finite clutch, not instant rigid lock.

## 5. Automatic gearbox

Prototype may model each commanded gear as an effective ratio plus clutch/brake transition package rather than simulating every planetary member.

Requirements:
- finite shift time;
- torque interruption/overlap;
- no instantaneous RPM teleport;
- thermal work from slipping elements.

## 6. DCT

Two input shafts/clutches:
- odd gears;
- even gears.

State:
- active clutch;
- preselected gear;
- clutch A/B temperature;
- shaft speeds.

Shift:
1. preselect next gear;
2. ramp outgoing clutch down;
3. ramp incoming clutch up;
4. torque overlap limited by clutch capacities.

Bad calibration may create torque hole or tie-up; both should emerge physically.

## 7. Sequential

Dog gearbox:
- one clutch for launch;
- shifts can unload via ignition/torque cut;
- dog engagement requires bounded speed mismatch;
- missed/harsh shifts can damage dogs/gears.

## 8. CVT

State:
- current ratio;
- ratio rate;
- belt/chain clamp capacity;
- temperature.

Ratio:
```
G_min <= G(t) <= G_max
```

Command is rate-limited.

Transmitted torque is limited by belt/clamp capacity; slip produces heat/wear.

No instant arbitrary ratio jump.

## 9. Shift controller

Controller owns policy:
- throttle/load;
- RPM;
- requested mode;
- kickdown;
- economy/sport;
- manual override.

It requests gear/ratio.

Transmission physics decides whether/how it engages.

## 10. Thermal

Sources:
- converter slip;
- clutch slip;
- CVT slip;
- gear mesh losses.

Cooling through:
- transmission fluid cooler;
- ambient;
- coolant circuit later.

Overtemperature can reduce clutch capacity and accelerate wear.

## 11. Damage

- worn clutch packs;
- low fluid;
- overheated fluid;
- damaged synchro/dog;
- gear tooth failure;
- valve/actuator failure;
- lockup failure;
- CVT belt failure.

No generic gearbox HP.

## 12. Authoring

Transmission component:
- backend type;
- gear ratios;
- reverse;
- efficiencies;
- inertias;
- shift elements;
- thermal package;
- controller defaults.

All physics-affecting values hash.

## 13. Tests

TRANS-001 ratio dimensional sanity.
TRANS-002 neutral transmits no drive torque.
TRANS-003 manual selected ratio.
TRANS-004 automatic shift finite time.
TRANS-005 converter stall/near-lock behavior.
TRANS-006 lockup clutch slip/heat.
TRANS-007 DCT torque overlap bounded.
TRANS-008 DCT cannot engage impossible simultaneous ratios without tie-up reaction.
TRANS-009 CVT ratio rate limit.
TRANS-010 CVT slip above capacity.
TRANS-011 thermal energy sign.
TRANS-012 deterministic fixed-step.

## 14. Promotion

Manual 6MT remains canonical until UE baseline.

New backend must pass isolated tests and a matched acceleration/coast trace before vehicle content uses it.
