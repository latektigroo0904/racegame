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

RPM:
```
RPM = omega * 60 / (2*pi)
```

## 3. Combustion torque
```
T_combustion =
TorqueMap(RPM, normalizedLoad)
* AirDensityFactor
* ThermalDerate
* DamageFactor
```

The authored torque map describes crankshaft output before driveline losses.

Throttle pedal is not identical to engine load. The solver applies pedal map, throttle response, idle control, boost availability and protection logic.

## 4. Engine friction
Use a tunable combination:
```
T_friction =
C0
+ C1 * omega
+ C2 * omega^2
```

The friction torque opposes crank rotation. This supports realistic coast-down and engine braking without a fixed negative-torque constant.

## 5. Idle, starter and stall
Idle control is finite and bounded:
```
T_idle = controller(RPM_target - RPM)
```

Engine states:
```
Stopped
Cranking
Running
Stalled
Seized
```

Starter torque is finite. A loaded engine may stall.

## 6. Turbocharger v0.1
State:
```
TurboSpeedNormalized
BoostPressure
WastegatePosition
```

First-order spool seed:
```
dS/dt =
(ExhaustEnergyTarget - S) / TauSpool
- WastegateLoss
```

Requirements:
- lag;
- boost decay on lift;
- wastegate control;
- thermal/damage hooks;
- no instantaneous torque multiplier.

## 7. Rev limiter
Electronic limiter reduces combustion torque above its threshold.

It does **not** prevent mechanical over-rev caused by wheel-driven crankshaft speed after an incorrect downshift.

## 8. Clutch
```
omega_slip = omega_engine - omega_gearbox_input
```

Requested clutch torque:
```
T_raw = K_clutch * omega_slip
```

Capacity:
```
|T_clutch| <= T_capacity
```

Capacity depends on engagement, clamp/friction calibration, temperature, wear and damage.

## 9. Clutch heat and wear
Slip power:
```
P_slip = abs(T_clutch * omega_slip)
```

Thermal integration:
```
dE/dt = P_slip - Cooling
```

Wear integrates friction work through a calibrated energy budget.

Failure progression:
```
Normal → Hot → Fading → Slipping → Failed
```

## 10. Gearbox ratio convention
Torque Atlas uses conventional reduction ratio:
```
G = omega_input / omega_output
```

Therefore:
```
omega_output = omega_input / G
T_output ≈ T_input * G * efficiency
```

This corrects an earlier draft equation that incorrectly multiplied angular speed by the reduction ratio.

Gear convention:
- -1 reverse;
- 0 neutral;
- 1..N forward.

Prototype includes six forward gears, reverse, neutral and shift/damage hooks.

## 11. Final drive
Final-drive ratio is solved separately from the selected gearbox ratio:
```
omega_axle = omega_gearbox_output / G_final
T_axle ≈ T_gearbox_output * G_final * efficiency
```

Keeping gearbox and final drive transforms separate improves telemetry, part swapping and damage modelling.

## 12. Mechanical over-rev
During a wrong downshift:
1. wheel speed implies differential/final-drive speed;
2. gearbox ratio implies gearbox-input speed;
3. clutch attempts synchronization;
4. crankshaft can be accelerated beyond electronic redline;
5. over-rev damage accumulates.

Do not clamp crankshaft RPM to redline.

## 13. Driveline compliance
Avoid a perfectly rigid algebraic drivetrain.

State:
```
theta_twist
omega_relative
```

Torque:
```
T_shaft =
K_torsion * theta_twist
+ C_torsion * omega_relative
```

This can represent gearbox-output shaft compliance and later half-shaft compliance.

## 14. Open differential v0.1
An open differential permits left/right wheel-speed difference and tends to transmit equal side torque in the ideal quasi-static case.

Prototype approximation:
- requested input torque is split into equal side torque;
- common transmitted torque is bounded by the lower available wheel reaction capacity;
- untransmitted torque is explicitly reported rather than silently discarded.

This is intentionally temporary. The full version should couple carrier, side-gear and wheel inertias rather than treating the differential as a static splitter.

Do **not** implement an open differential as a 50/50 wheel-speed lock.

## 15. Future differential backends
Interface must support:
- clutch LSD;
- helical/torque-biasing;
- viscous;
- locked/spool;
- active torque vectoring.

## 16. Solver ordering per vehicle substep
1. sample wheel angular states;
2. propagate differential/final-drive/gearbox kinematic targets;
3. update driver throttle, clutch and gear command;
4. update engine combustion/friction/turbo;
5. solve clutch reaction;
6. solve gearbox/final-drive transforms and compliant shafts;
7. solve differential torque distribution;
8. apply axle torques to driven wheels;
9. integrate rotational inertias;
10. update thermal/wear/damage state.

Tire reaction and driveline torque are coupled. The implementation may require a small local iterative solve; measure before increasing global iteration count.

## 17. Damage hooks
Powertrain modifiers come from:
- cooling;
- oil pressure;
- engine internals;
- clutch;
- gearbox;
- driveshaft/CV;
- differential.

Damage changes physical parameters, not a generic HP scalar.

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

Values are calibration seeds, not final claims.

## 19. Required tests
- free rev;
- engine coast-down;
- idle stabilization;
- clutch launch;
- clutch thermal abuse;
- stall against brake;
- gear-ratio speed/torque transforms;
- wrong-gear over-rev;
- neutral decoupling;
- open-diff split traction;
- torque/energy sign sanity;
- bounded driveline oscillation.

## 20. Acceptance
v0.1 is acceptable when:
- launch occurs from torque balance rather than a speed assist;
- clutch slip and stall emerge naturally;
- ratio transforms are dimensionally and physically consistent;
- mechanical over-rev is possible;
- split-traction open-diff behaviour is plausible;
- fixed-step tests repeat within numerical tolerance;
- car-specific calibration remains data-driven.
