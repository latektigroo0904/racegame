# Automation Harness Hardening Contract V0.1

Updated: 2026-09-26

## Purpose
This contract closes the remaining ambiguity between the Unreal Automation invocation, JSON report validation, verification metadata, and staged Proof-of-Physics gates.

## Canonical prefix grammar
Every externally supplied Automation prefix MUST match:

`^TorqueAtlas\.(?:[A-Za-z_][A-Za-z0-9_]*\.)*$`

Examples accepted:
- `TorqueAtlas.`
- `TorqueAtlas.Vehicle.`
- `TorqueAtlas.Vehicle.StaticLoad.`

Examples rejected:
- `TorqueAtlas`
- `TorqueAtlas.Vehicle`
- `TorqueAtlas..Vehicle.`
- `TorqueAtlas.*`
- prefixes containing whitespace or command separators.

The same validated prefix is the single source for Unreal `Automation RunTest`, report-validator `--prefix`, metadata `automation_filter`, and human-readable summary output.

## Expected-count contract
`ExpectedCount` is optional. When present it MUST be a positive integer and the report MUST contain exactly that many tests under the requested prefix.

Source-inventoried staged gates:
1. `TorqueAtlas.Suspension.Contact.Compliant.` = 2
2. `TorqueAtlas.Vehicle.StaticMassBalance.` = 3
3. `TorqueAtlas.Vehicle.StaticLoadComparison.` = 3
4. `TorqueAtlas.Vehicle.StaticLoad.` = 9
5. `TorqueAtlas.Suspension.Contact.` = 9

The root `TorqueAtlas.` suite intentionally has no exact count so adding unrelated valid tests does not create false failures.

## Report invariants
The validator is fail-closed:
- JSON root is an object.
- `tests` is a non-empty array of objects.
- aggregate counters are non-negative integers; booleans are invalid.
- aggregate counter total equals `len(tests)`.
- every test path belongs to the requested prefix.
- exact count matches when supplied.
- `failed == 0`, `notRun == 0`, and `inProcess == 0`.
- at least one test succeeded or succeeded with warnings.
- each matching test has a non-negative integer `errors` field.
- failing/not-run/in-process per-test states or nonzero errors fail the gate.

## Regression requirements
The regression suite MUST independently exercise:
- valid root report;
- valid exact count;
- exact-count mismatch / disappeared test;
- aggregate-correct `failed`, `notRun`, and `inProcess` failures;
- test outside requested prefix;
- empty tests;
- non-object JSON root;
- non-object `tests[]` entry;
- missing, negative, boolean, and malformed aggregate counters;
- aggregate-total mismatch;
- malformed per-test `errors`;
- accepted and rejected namespace forms.

Negative fixtures for `notRun` and `inProcess` MUST preserve the aggregate total. Otherwise the aggregate-reconciliation gate masks the intended assertion.

## Runner contract
Canonical runners:
- Windows: `Scripts/Verify-Unreal.ps1`
- Linux/macOS: `Scripts/verify-unreal.sh`

Both runners MUST accept one Automation prefix and an optional expected count, validate them before Unreal is launched, and propagate them without re-hardcoding.

Metadata MUST record:
- `automation_filter`
- `expected_test_count` (null for dynamic root suite)
- Git SHA
- Unreal version
- target/platform/configuration
- build/Automation exit codes
- final status/failure.

## Execution order
After source sanity and UE 5.8 UHT/UBT:
1. Compliant (2)
2. StaticMassBalance (3)
3. StaticLoadComparison (3)
4. StaticLoad (9)
5. Suspension.Contact (9)
6. dynamic `TorqueAtlas.` root suite
7. independent static-load baseline validator
8. first real 120 Hz / 120 consecutive sample FL/FR/RL/RR baseline.

## Non-goals
This hardening work does not change suspension, aero, tire, sampler, or physical acceptance parameters. Numerical evidence-consistency tolerances and the physical 2/3/3/2% acceptance envelope remain separate contracts as defined by docs/46-48.

## Risks
- UE 5.8 execution is still required to prove UHT/UBT, module loading, and Automation behavior.
- Runner/validator drift remains possible until both runners consume the same validated inputs.
- Exact-count gates become stale when tests are intentionally added or removed; such changes require source-inventory review and a deliberate count update.
