# Powertrain Solver v0.1

Status: prototype specification
Updated: 2026-09-23

## 1. Objective
Model the energy/torque path:

```
Engine
→ crankshaft
→ clutch
→ gearbox
→ final drive
→ differential
→ half-shafts
→ driven wheels
```

The prototype must already support thermal clutch slip, driveline inertia, engine braking and mechanical over-rev.

## 2. Engine rotational dynamics
Primary crankshaft state:
```
omega_engine [rad/s]
```

Integration:
```
I_engine * domega/dt =
T_combustion
- T_friction
- T_accessories
- T_clutch_reaction
```

RPM is a display/authoring representation:
```
RPM = omega * 60 / (2*pi)
```

## 3. Combustion torque
Prototype:
```
T_combustion =
TorqueMap(RPM, normalizedLoad)
* AirDensityFactor
* ThermalDerate
* DamageFactor
```

The authored torque map describes crankshaft output before driveline losses.

Throttle pedal is not identical to engine load. The solver applies:
- pedal map;
- throttle-body response;
- idle controller;
- boost availability;
- engine protection.

## 4. Engine friction
Use a tunable combination:
```
T_friction =
C0
+ C1 * omega
+ C2 * omega^2
```

Sign opposes crank rotation.

This enables realistic coast-down and engine braking without a fixed negative torque constant.

## 5. Idle control
Below target idle speed:
```
T_idle = controller(RPM_target - RPM)
```

Controller output is limited and blended with driver throttle.

A damaged/stalled engine must not receive infinite anti-stall torque.

## 6. Starter/stall
States:
```
Stopped
Cranking
Running
Stalled
Seized
```

Starter applies finite torque below a maximum starter speed.

Engine stalls when combustion cannot sustain crank speed against load/friction.

## 7. Turbocharger v0.1
Do not model CFD.

State:
```
TurboSpeedNormalized
BoostPressure
WastegatePosition
```

First-order spool model:
```
dS/dt =
(ExhaustEnergyTarget - S) / TauSpool
- WastegateLoss
```

Boost derives from turbo state, RPM and compressor calibration.

Requirements:
- visible lag;
- boost decay on lift;
- wastegate control;
- thermal/damage hooks;
- no instantaneous torque multiplier.

## 8. Rev limiter
Electronic limiter can reduce combustion torque above limiter threshold.

It does **not** prevent mechanical over-rev caused by wheel-driven crankshaft speed after an incorrect downshift.

## 9. Clutch
Define slip:
```
omega_slip = omega_engine - omega_gearbox_input
```

Requested clutch torque:
```
T_raw = K_clutch * omega_slip
```

Clamp:
```
|T_clutch| <= T_capacity
```

Capacity depends on:
- pedal engagement;
- clamp force;
- friction coefficient;
- temperature;
- wear/damage.

Torque sign opposes slip.

## 10. Clutch energy and temperature
Slip power:
```
P_slip = abs(T_clutch * omega_slip)
```

Thermal integration:
```
dE/dt = P_slip - Cooling
```

Temperature changes friction/fade and accelerates wear.

Failure progression:
```
Normal → Hot → Fading → Slipping → Failed
```

## 11. Gearbox
For selected ratio `G`:
```
omega_out ≈ omega_in * G
T_out ≈ T_in * G * efficiency
```

Prototype includes:
- neutral;
- reverse;
- six forward gears;
- finite input/output inertia;
- shift interruption;
- gear damage hook.

Full synchronizer cone simulation is deferred, but invalid shift requests must be representable and later extensible.

## 12. Mechanical over-rev
During clutch engagement after a downshift:
1. compute wheel-imposed gearbox input speed;
2. clutch attempts synchronization;
3. reaction torque accelerates crankshaft;
4. RPM may exceed electronic redline;
5. over-rev damage accumulates.

Do not clamp engine RPM to redline.

## 13. Driveline compliance
Avoid a perfectly rigid algebraic drivetrain.

Prototype introduces torsional shaft state:
```
theta_twist
omega_relative
```

Torque:
```
T_shaft = K_torsion * theta_twist + C_torsion * omega_relative
```

Use for:
- gearbox-output to differential input;
- optional half-shaft compliance later.

This creates driveline lash/oscillation in a controlled way.

## 14. Open differential v0.1
Conserve input torque and permit wheel-speed difference.

For equal ideal outputs:
```
T_left ≈ T_right
```

Actual transmitted wheel torque is limited by tire reaction and drivetrain constraints.

Do not implement an open diff as a simple 50/50 wheel-speed lock.

## 15. Future differential backends
Interface must allow:
- clutch LSD;
- helical/torque-biasing;
- viscous;
- locked/spool;
- active torque vectoring.

## 16. Solver ordering per vehicle substep
1. sample wheel angular states;
2. propagate gearbox/differential kinematic targets;
3. update driver throttle/clutch/gear command;
4. update engine combustion/friction/turbo;
5. solve clutch torque;
6. solve gearbox and driveline compliance;
7. solve differential torque distribution;
8. apply axle torques to driven wheels;
9. integrate rotational inertias;
10. compute thermal/wear/damage updates.

Because tire reaction and driveline torque interact, the final implementation may require 2–4 local iterations; measure before increasing globally.

## 17. Damage hooks
Powertrain receives state modifiers from:
- engine cooling;
- oil pressure;
- crank/internal damage;
- clutch condition;
- gearbox condition;
- driveshaft/CV condition;
- differential condition.

A damaged component changes physical parameters rather than only multiplying a generic HP value.

## 18. TA-P01 baseline
Provisional authored values:
```
Engine: 2.0 L turbo I4
Peak torque: ~410 Nm
Peak power: ~220 kW
Idle: 850 rpm
Redline: 7200 rpm
Limiter: 7400 rpm

Gear ratios:
1  3.45
2  2.15
3  1.52
4  1.16
5  0.92
6  0.76
Final drive 3.90
```

Values are calibration starting points, not final claims.

## 19. Unit/regression tests
Required:
- no-load free rev;
- engine coast-down;
- idle stabilization;
- clutch launch;
- clutch sustained slip/overheat;
- stall against brake;
- each gear ratio speed relationship;
- wrong-gear over-rev;
- neutral decoupling;
- open-diff wheel-speed difference;
- torque/energy sign sanity;
- driveline oscillation remains bounded.

## 20. Acceptance targets
The v0.1 solver is acceptable when:
- launch can occur without artificial speed clamps;
- stall and clutch slip emerge from torque balance;
- shift/downshift effects are physically plausible;
- over-rev can happen mechanically;
- identical fixed-step tests remain repeatable within numerical tolerance;
- all car-specific calibration resides in data, not solver branches.
