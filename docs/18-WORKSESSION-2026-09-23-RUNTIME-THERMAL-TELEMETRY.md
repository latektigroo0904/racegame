# Worksession checkpoint — integrated runtime, thermal damage and telemetry

Date: 2026-09-23
Status: clean checkpoint; source changes committed on `main`

## Completed

### 1. Integrated vehicle runtime v0.1
Added a deterministic native `TA_Vehicle` fixed-step orchestrator that closes the first real physics chain:

`engine -> clutch -> gearbox -> final drive -> open differential -> wheel inertia -> tire -> wheel reaction`

The runtime accepts wheel-local contact/kinematic inputs rather than inventing unfinished suspension/chassis contact behavior.

Implemented:
- driver throttle/brake/clutch/gear controls;
- engine torque curve, idle control, limiter;
- engine stall/cranking/running state and starter torque;
- two-driven-wheel open-differential coupling for TA-P01;
- wheel angular inertia;
- brake integration that clamps at zero instead of numerically reversing a stopped wheel;
- tire reaction torque;
- total tire-force aggregation.

### 2. Tire/drivetrain coupling
Added a public longitudinal tire force-capacity estimate. The open differential can therefore be reaction-limited by the lower-grip driven wheel without reaching into tire-solver internals.

### 3. Suspension runtime v0.1
Added:
- precomputed suspension kinematic sample cache;
- interpolation of wheel-center offset, camber, toe and motion ratio;
- spring force with static spring compression at reference ride height;
- separate bump/rebound damping;
- bump and droop stops;
- explicit cache invalidation hook for damaged geometry.

Important boundary: damaged-hardpoint kinematics are not faked. Once geometry is structurally displaced, the undamaged cache can be invalidated and the future damaged-geometry solver must take over.

### 4. Radiator damage -> engine consequence chain
Connected existing radiator damage state to a new engine thermal model:

`radiator impact -> crush/puncture -> coolant loss -> cooling efficiency loss -> coolant temperature rise -> thermal torque derate -> thermal engine damage`

This is causal state propagation, not a scripted post-impact power penalty.

### 5. Engine thermal model
Added:
- effective thermal mass;
- load/RPM-sensitive heat generation;
- cooling capacity scaled by radiator efficiency;
- temperature-based torque derate;
- cumulative thermal damage;
- corrected behavior for stopped/stalled engines so they do not generate running-engine heat.

### 6. Engine start/stall state
Corrected an important physical inconsistency: a zero-rpm engine can no longer remain `Running` and self-restart from the torque curve.

States now support:
- Stopped;
- Cranking;
- Running;
- Stalled;
- Seized.

Starter torque is explicit and bounded.

### 7. Telemetry runtime module
Added `TA_Telemetry` and registered it in the plugin descriptor.

Telemetry currently captures:
- simulation tick;
- engine RPM;
- coolant temperature;
- thermal torque factor;
- radiator cooling efficiency;
- coolant mass;
- clutch slip;
- aggregate longitudinal/lateral force;
- aggregate aligning moment;
- left/right driven-wheel torque.

The telemetry ring buffer allocates at initialization and overwrites in place during steady-state capture.

## Tests added

### Vehicle integration
- powertrain transmits torque to driven wheels;
- neutral decouples drive torque;
- brake does not numerically reverse wheel direction;
- split-mu open differential is limited by low-grip side;
- radiator damage eventually produces engine thermal derate.

### Suspension
- cache interpolation;
- invalid cache refuses undamaged lookup;
- static ride height retains positive spring support;
- bump/rebound damping sign behavior.

### Powertrain
- engine stall transition;
- starter torque only when appropriate;
- cranking -> running transition above combustion-start RPM.

### Telemetry
- fixed-capacity ring-buffer wrap;
- chronological retrieval after overwrite.

## Assumptions

1. TA-P01 v0.1 is RWD with exactly two driven wheels.
2. Differential model remains quasi-static open diff until carrier/side-gear inertias are implemented.
3. Wheel contact loads/velocities are external inputs until chassis + suspension contact solving is integrated.
4. Thermal constants are calibration seeds, not claimed measured production values.
5. Suspension kinematic samples must be sorted by travel when authored; validation for monotonic ordering is still needed.

## Risks / known gaps

### Build verification
No GitHub Actions workflow currently exists in the repository and this session did not have an Unreal Engine 5.8 compiler/runtime available. Source-level integration is committed, but a real UBT compile and Unreal Automation Test run remains mandatory before calling this milestone build-green.

### Chassis closure
The vehicle runtime still does not integrate chassis 6-DOF state. Tire forces are aggregated but not yet applied at contact patches to update body linear/angular momentum.

### Suspension closure
The suspension runtime calculates kinematics/forces but is not yet connected to chassis contact/load solving. Structural hardpoint displacement is not yet converted into damaged suspension geometry.

### Numerical coupling
The current driveline/wheel/tire solve is explicit single-pass. A small local iterative solve may be required once chassis and suspension are coupled, especially for high clutch stiffness and low-speed tire slip.

### Telemetry export
The ring buffer exists, but CSV/JSON export and named scenario comparison are not yet implemented.

## Deliverables in this session

- `docs/17-INTEGRATED-VEHICLE-RUNTIME-V01.md`
- `TA_Vehicle/Public/TAVehicleSimulation.h`
- `TA_Vehicle/Private/TAVehicleSimulation.cpp`
- `TA_Vehicle/Public/TASuspensionRuntime.h`
- `TA_Vehicle/Private/TASuspensionRuntime.cpp`
- engine torque/thermal/starter extensions in `TA_Powertrain`
- tire reaction-capacity API in `TA_Tire`
- integration/suspension/powertrain regression tests
- new `TA_Telemetry` runtime module + ring-buffer test

## Exact next checkpoint

Next work session starts with **build-readiness and chassis closure**, in this order:

1. Perform a compile-oriented API audit across all newly touched headers/modules and fix dependency/include/export issues.
2. Add authoring validation for suspension cache ordering and physical parameter ranges.
3. Define `FTAChassisRuntimeState` with SI linear/angular state and force-at-point accumulation.
4. Implement pure 6-DOF chassis integration without Unreal rigid-body side effects.
5. Apply each wheel tire/suspension force at its contact position so pitch/yaw/roll moments emerge physically.
6. Feed chassis wheel-point velocity back into tire inputs.
7. Add a deterministic straight-line acceleration scenario and braking scenario using the closed chassis loop.
8. Add telemetry samples for chassis velocity, acceleration, yaw rate, wheel speed/slip and suspension travel.
9. Run UBT + Unreal Automation Tests as soon as an UE 5.8-capable build environment is available.

Do not begin ABS/TCS, FFB, advanced aero or graphics work before this chassis loop is closed and testable.
