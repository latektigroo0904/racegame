# Aerodynamics authoring compilation v0.1

Updated: 2026-09-23

## Purpose
Close the coordinate/ownership ambiguity between vehicle-authored aero calibration and the low-level runtime solver before canonical `TAVehicleSimulation::Step` wiring.

## Added source
- `FTAAerodynamicsDefinition`: authored reference area, drag coefficient, lift coefficient and vehicle-origin-local resultant application point.
- `TAAerodynamicsDefinition::Validate`: rejects non-finite data, non-positive reference area and negative drag coefficient.
- `TAAerodynamicsDefinition::Compile`: converts authored vehicle-origin-local application point to the COM-local coordinate expected by `FTAAerodynamicsConfig`.
- `TAAerodynamicsDefinition::HashRuntimeConfig`: hashes the effective runtime area/Cd/Cl and all three COM-local application-point components.
- Automation regressions for COM conversion, hash sensitivity and invalid authoring.

## Decisions
1. Authoring uses vehicle-origin-local coordinates because vehicle geometry, suspension hardpoints and structure content already use that convention.
2. Runtime aero uses COM-local coordinates because `TAChassisDynamics::AddForceAtWorldPoint` derives physical moments relative to the chassis center of mass.
3. Physics hashing is over effective compiled values. This means a COM change that changes the effective aero lever arm necessarily changes the aero hash contribution.
4. Negative lift coefficient means downforce, matching the existing low-level solver convention.
5. No separate front/rear aero split is introduced in v0.1. A single resultant is sufficient for Proof-of-Physics but is explicitly not considered final for high-downforce or active-aero vehicles.

## Risks / incomplete integration
- `FTAAerodynamicsDefinition` is deliberately isolated in its own header to make the authoring/compile contract testable before touching the large vehicle-definition asset file.
- It is not yet a `UTAVehicleDefinition` property, so compiled production assets still use runtime aero defaults.
- `PhysicsConfigHash` does not yet call `HashRuntimeConfig` from the canonical vehicle compiler.
- `TAVehicleSimulation::Step` still does not invoke `TAVehicleAerodynamicsBridge` before chassis integration.
- UE 5.8 compile/UHT/Automation execution remains unverified.

## Acceptance sequence
1. Include `TAAerodynamicsDefinition.h` from `TAVehicleDefinition.h` and add `UPROPERTY FTAAerodynamicsDefinition Aerodynamics`.
2. Call `Validate` during `BuildCompiledConfig` validation.
3. Call `Compile(Aerodynamics, CenterOfMassMeters, VehicleRuntime.Aerodynamics)`.
4. Fold `HashRuntimeConfig` into the canonical `PhysicsConfigHash` immediately after other vehicle-level force calibration.
5. Invoke `TAVehicleAerodynamicsBridge::AddToChassis` after tire/suspension force accumulation and immediately before `TAChassisDynamics::Integrate`.
6. Add a full-step regression with nonzero chassis speed and zero wheel forces, proving drag changes chassis longitudinal velocity and downforce/application-point offset produces the expected force/moment sign.
7. Add telemetry channels from `FTAVehicleStepOutput::Aerodynamics` without recomputation.

## Clean checkpoint
The standalone authoring compiler is source-complete. Next run should perform the three canonical call-site edits above, then full-step regression and telemetry wiring. Do not promote an aero baseline until UE 5.8 executable verification passes.
