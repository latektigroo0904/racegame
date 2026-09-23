# Aero Asset Call-Site Patch Manifest v0.1

Updated: 2026-09-23

## Purpose
Close the final source-level ownership gap between `UTAVehicleDefinition` authoring and the already-established aerodynamic runtime/compiler path without duplicating validation, coordinate conversion, or hashing.

## Required canonical changes

### `TAVehicleDefinition.h`
1. Include `TAAerodynamicsDefinition.h` before the generated header.
2. Add exactly one authored field to `UTAVehicleDefinition`:
   - type: `FTAAerodynamicsDefinition`
   - name: `Aerodynamics`
   - category: `Physics|Aerodynamics`
3. Do not expose wind or air density here. Those remain transient step-environment inputs.

### `TAVehicleDefinition.cpp`
1. Include `TAVehicleAerodynamicsAssetCompiler.h`.
2. Before the existing early `OutValidation.HasErrors()` return, call `Aerodynamics.Validate(...)`. If validation fails, emit one vehicle-level error with code `Vehicle.InvalidAerodynamics`. Do not compile/hash invalid aero.
3. Keep authored application point vehicle-origin-local. Use the already-computed `CenterOfMassVehicleLocalM` as the only coordinate transform input.
4. After the existing base physics hash has been assembled, but before assigning `OutConfig.PhysicsConfigHash`, call exactly once:
   `TAVehicleAerodynamicsAssetCompiler::CompileValidatedAndHash(Aerodynamics, CenterOfMassVehicleLocalM, Hash, VehicleRuntime.Aerodynamics)`.
5. If that call unexpectedly fails after pre-validation, emit `Vehicle.InvalidAerodynamics` and return false. Do not assign a final physics hash.
6. Do not separately hash area/Cd/Cl/application point at the vehicle-definition call site; the adapter already hashes effective runtime values.

## Frozen regression fixture
Use deliberately non-default values so runtime defaults cannot mask a missing call site:
- reference area: `2.37 m^2`
- Cd: `0.287`
- Cl: `-0.219`
- authored application point: `(0.46, -0.04, 0.52) m`
- vehicle COM: `(0.14, -0.01, 0.19) m`
- expected runtime COM-local application point: `(0.32, -0.03, 0.33) m`

## Required asset-level regressions
1. **Non-default propagation:** `BuildCompiledConfig` succeeds and `VehicleRuntime.Aerodynamics` exactly matches the frozen coefficients and expected COM-local point.
2. **Validation:** invalid authored aero causes `BuildCompiledConfig` to fail and includes `Vehicle.InvalidAerodynamics`.
3. **Hash sensitivity:** changing Cd, Cl, reference area, or the effective COM-local application point changes `PhysicsConfigHash`.
4. **Coordinate invariance:** translating authored application point and COM by the same vehicle-local offset preserves both effective runtime aero and final `PhysicsConfigHash`.
5. **No double hash:** rebuilding the same definition is deterministic and yields the same final hash.

## Invariants
- COM subtraction occurs exactly once.
- Hash identity follows effective compiled physics, not raw authored coordinates.
- Wind and air density are not asset-hashed.
- No speed-dependent tire-grip multiplier is introduced.
- Aero continues through the shared chassis force accumulator before the single chassis integration.
- Telemetry/regression consume applied step output; they do not recompute aero.

## Acceptance gate
Source-level closure is complete only when the property, validation, adapter call, final hash assignment and asset-level tests all exist together. Executable closure additionally requires UHT/UBT plus `Automation RunTest TorqueAtlas.` under UE 5.8. Until then the repository remains UE-build-unverified.
