# Aerodynamics Canonical Closure Contract v0.1

Updated: 2026-09-23

## Purpose
This contract freezes the exact remaining integration boundary for Proof-of-Physics aerodynamics before implementation is allowed to expand into aero maps, active aero, split front/rear devices or gameplay tuning.

## Audited current state
The low-level `TAAerodynamics` solver, `FTAAerodynamicsDefinition`, `TAVehicleAerodynamicsBridge`, per-step environment input, step-output ownership, telemetry channels and scalar regression projection already exist.

The canonical vehicle path still has two gaps:
1. `UTAVehicleDefinition` does not own an authored `FTAAerodynamicsDefinition`, therefore `BuildCompiledConfig` leaves `VehicleRuntime.Aerodynamics` at runtime defaults and the physics hash does not identify authored aero changes.
2. `TAVehicleSimulation::Step` integrates the chassis immediately after wheel/suspension force accumulation and does not invoke the aero bridge.

## Frozen ownership rules
- Vehicle asset owns: reference area, drag coefficient, lift coefficient and vehicle-origin-local application point.
- Compile step owns: validation and vehicle-origin-local -> COM-local conversion.
- Runtime config owns: effective compiled aero coefficients and COM-local application point.
- Step environment owns: air density and world-space wind velocity.
- Aero solver owns: relative air velocity, dynamic pressure, force, application point and torque.
- Shared chassis accumulator owns: the sum of tire, suspension, aligning and aerodynamic loads.
- Step output owns: the exact aero result actually applied during that step.
- Telemetry/reporting may observe that result but must never recompute aero.

## Canonical compile sequence
`UTAVehicleDefinition::BuildCompiledConfig` must perform the following sequence:
1. validate `Aerodynamics` with `TAAerodynamicsDefinition::Validate`;
2. fail compilation with a vehicle validation error when invalid;
3. after COM is known, compile with `TAAerodynamicsDefinition::Compile(Aerodynamics, CenterOfMassVehicleLocalM, VehicleRuntime.Aerodynamics)`;
4. treat compile failure as an error, not as a silent fallback;
5. fold `TAAerodynamicsDefinition::HashRuntimeConfig(Hash, VehicleRuntime.Aerodynamics)` into the canonical `PhysicsConfigHash` exactly once.

The hash must use effective runtime values. Consequently a COM change that changes the effective aero lever arm changes the hash even when the authored application point is unchanged.

## Canonical simulation sequence
Within `TAVehicleSimulation::Step`:
1. reset output;
2. validate step shape;
3. calculate powertrain/wheel/tire state;
4. add tire forces, suspension forces and aligning moments to `ChassisForces`;
5. call `TAVehicleAerodynamicsBridge::AddToChassis(Config, Input, InOutState.Chassis, ChassisForces, OutOutput)` exactly once;
6. abort the step if the bridge/solver rejects invalid data;
7. call `TAChassisDynamics::Integrate` exactly once using that combined accumulator;
8. finish thermal/output bookkeeping.

Aero must therefore use the pre-integration chassis pose and velocity for the current fixed step. This avoids a hidden half-step and makes force ownership deterministic.

## Required regression coverage
### Asset/compiler
- default asset compiles default authored aero into runtime;
- non-zero COM converts application point exactly once;
- changing Cd changes `PhysicsConfigHash`;
- changing Cl changes `PhysicsConfigHash`;
- changing application point changes `PhysicsConfigHash`;
- invalid area/Cd/non-finite data rejects the vehicle asset.

### Full vehicle step
Use zero tire/suspension force inputs to isolate aero.
- zero relative air speed produces zero aero and no aero-induced velocity change;
- positive chassis forward speed produces drag opposite travel and changes chassis longitudinal velocity;
- headwind increases dynamic pressure and drag versus still air at identical chassis speed;
- negative Cl produces downforce in the existing sign convention;
- an off-COM application point produces a non-zero pitch response;
- `OutOutput.Aerodynamics` matches the force/torque actually fed into chassis integration.

### Reporting
Once the full-step path is green, machine-readable regression output must serialize the existing scalar channels from `TAAeroRegressionMetrics` without recomputation.

## Invariants
- No speed-dependent tire-grip multiplier.
- No direct modification of tire friction coefficient from vehicle speed.
- No duplicate aero force path.
- No authored wind or air density in the vehicle asset.
- No world-axis assumptions in telemetry regression projection.
- No promotion of front/rear aero maps until executable UE 5.8 evidence exists.

## Risks
- UHT may reject reflected-property changes even when source-level structure is correct.
- Existing default runtime aero values currently mask missing asset compilation because they numerically match authored defaults; tests must deliberately use non-default authored values.
- An application-point coordinate mistake can produce plausible drag while silently corrupting pitch moment.
- A bridge call after chassis integration would introduce a one-step lag; placement is therefore part of the acceptance contract.

## Definition of done
Canonical aero physics closure is complete only when authored values compile and hash, `TAVehicleSimulation::Step` applies aero through the shared accumulator before chassis integration, full-step regressions cover speed/wind/downforce/pitch response, and source-sanity is green. UE 5.8 UHT/UBT/Automation execution remains a separate final acceptance gate.

## Next checkpoint
Implement this contract in `TAVehicleDefinition.h/.cpp` and `TAVehicleSimulation.cpp`, then add the asset/hash and full-step regressions before touching a new physics subsystem.
