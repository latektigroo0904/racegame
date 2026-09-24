# 45 — UE 5.8 UHT / module dependency audit V0.1

Status: source audit complete; executable UE 5.8 verification still required.

## Scope

This audit is the pre-UBT gate after canonical closure of compliant-contact patch 44 and aero ownership patch 41. It checks the dependency direction and UHT-facing include structure that can be established from source without claiming an engine build.

## Findings

### 1. Module topology

`TA_Vehicle` declares `Core`, `CoreUObject`, `Engine`, `TA_Core`, `TA_Surface`, `TA_Tire`, `TA_Powertrain`, `TA_Damage`, and `TA_Structure` as public dependencies. This is consistent with the vehicle module exposing public USTRUCT/API types that compose those subsystems.

The game module `TorqueAtlas` depends privately on `CoreUObject`, `Engine`, `TA_Core`, and `TA_Vehicle`, with only `Core` public. This keeps plugin implementation dependencies from leaking through the game module.

No new module dependency is required by the P1.1 static-load evidence classes because they live inside `TA_Vehicle` and consume vehicle-owned contact/chassis types.

### 2. UHT include ordering

`TAVehicleDefinition.h` includes `TAAerodynamicsDefinition.h` before `TAVehicleDefinition.generated.h`; the generated header remains the final include. This is the required UHT-facing ordering.

`TAAerodynamicsDefinition.h` includes its own generated header after ordinary includes and defines `FTAAerodynamicsDefinition` as a `USTRUCT(BlueprintType)` exported by `TA_VEHICLE_API`.

The canonical authored-aero ownership therefore does not introduce a cross-module reflected-type dependency beyond `TA_Vehicle` itself.

### 3. Dependency direction

Authored aero definition -> compiled aero runtime remains internal to `TA_Vehicle`. `TAVehicleDefinition` owns the authored definition, while compilation produces runtime configuration. This avoids a dependency from a lower-level module back into the game module.

The static-load oracle/evidence/comparison/acceptance chain is also contained in `TA_Vehicle`; it must remain evidence-only and must not become a solver dependency.

### 4. Risks that source inspection cannot close

Source inspection cannot prove:

- UnrealHeaderTool parsing under UE 5.8;
- UnrealBuildTool unity/non-unity compilation;
- generated-code symbol/export correctness;
- include-order differences exposed by engine headers;
- Automation module discovery and runtime execution.

Therefore repository status remains **UE 5.8 build-unverified**.

## Required executable gate

Run in this order on a UE 5.8-capable runner/workstation:

1. Generate/project build or invoke UBT for `TorqueAtlasEditor` Development.
2. Treat any UHT failure as a hard gate; do not continue to physics acceptance.
3. Run `TorqueAtlas.Suspension.Contact.Compliant.*`.
4. Run `TorqueAtlas.Vehicle.StaticLoad.*`.
5. Run all suspension/contact tests.
6. Run `Automation RunTest TorqueAtlas.`.

Capture engine version, target, configuration, compiler, command, exit code, and failing test names in the checkpoint.

## Decision

No speculative Build.cs or include edits are justified by this audit. The current dependency graph is internally consistent. The next useful work is executable UE 5.8 verification; if that environment is unavailable, source-only work should move to a deterministic build/test-runbook and failure-triage contract rather than changing physics code without evidence.
