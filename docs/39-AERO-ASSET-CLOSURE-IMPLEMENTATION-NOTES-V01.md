# Aero Asset Closure Implementation Notes V01

Updated: 2026-09-23

## Purpose
Freeze the exact remaining source edits needed to close the canonical vehicle-asset aerodynamic ownership path without allowing runtime defaults to masquerade as authored data.

## Current verified source state
- `FTAAerodynamicsDefinition` exists and validates/compiles vehicle-origin-local authoring into `FTAAerodynamicsConfig` using a COM-local application point.
- `FTAVehicleRuntimeConfig` already owns the effective aerodynamic runtime config.
- `TAVehicleSimulation::Step` already applies that runtime config through `TAVehicleAerodynamicsBridge` before the single chassis integration.
- `UTAVehicleDefinition` still has no `Aerodynamics` property.
- `UTAVehicleDefinition::BuildCompiledConfig` therefore still leaves `VehicleRuntime.Aerodynamics` at its struct defaults and does not fold aero into `PhysicsConfigHash`.

## Required canonical edits
1. Include `TAAerodynamicsDefinition.h` from `TAVehicleDefinition.h` before the generated-header include.
2. Add `UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physics|Aerodynamics") FTAAerodynamicsDefinition Aerodynamics;` to `UTAVehicleDefinition`.
3. During early authoring validation, call `TAAerodynamicsDefinition::Validate(Aerodynamics)`. On failure emit one error with code `Vehicle.InvalidAerodynamics` and stop before runtime promotion in the same manner as other invalid authoring.
4. After `CenterOfMassVehicleLocalM` is known and before the compiled config is considered complete, call `TAAerodynamicsDefinition::Compile(Aerodynamics, CenterOfMassVehicleLocalM, VehicleRuntime.Aerodynamics)`. A false result is an error even if prior validation unexpectedly passed.
5. In the canonical physics-hash sequence, fold `TAAerodynamicsDefinition::HashRuntimeConfig(Hash, VehicleRuntime.Aerodynamics)` exactly once. Hash the effective runtime config, never the raw authored application point.

## Acceptance regressions
The asset-level test must use deliberately non-default values so runtime defaults cannot create a false pass:
- reference area: 2.37 m^2;
- Cd: 0.287;
- Cl: -0.219;
- authored application point: (0.46, -0.04, 0.52) m;
- COM: (0.14, -0.01, 0.19) m.

Expected COM-local application point: (0.32, -0.03, 0.33) m.

Required assertions:
1. `BuildCompiledConfig` succeeds for valid non-default aero.
2. compiled area/Cd/Cl exactly match authored values;
3. compiled application point equals the expected COM-local point;
4. changing Cd changes `PhysicsConfigHash`;
5. changing Cl changes `PhysicsConfigHash`;
6. changing effective application point changes `PhysicsConfigHash`;
7. shifting both vehicle-origin application point and COM by the same vector preserves the effective runtime aero hash contribution;
8. invalid reference area or negative Cd produces `Vehicle.InvalidAerodynamics` and compilation failure.

## Invariants
- Air density and wind remain transient step/environment inputs and are not asset-hashed.
- The authored application point is vehicle-origin-local; runtime is COM-local.
- COM subtraction occurs exactly once.
- No second aero solver invocation is permitted for telemetry or regression reporting.
- No speed-dependent tire-grip multiplier is permitted; aero grip must emerge through physical chassis loading.

## Risk controls
The highest immediate false-positive risk is default-value masking. Every asset integration regression must therefore use non-default coefficients and non-zero COM. The highest coordinate risk is double COM subtraction; the expected-point assertion above catches it directly. The highest reproducibility risk is hashing authored coordinates rather than effective runtime coordinates; the coordinate-invariance regression locks the intended behavior.

## Continuation
Implement the five canonical edits, add the asset-level regressions, then wire existing aero scalar metrics into the machine-readable scenario report. Do not open a new major physics subsystem before the asset compile/hash path is source-level closed.
