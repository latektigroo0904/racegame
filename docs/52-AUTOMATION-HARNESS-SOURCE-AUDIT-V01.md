# Automation Harness Source Audit V01

Updated: 2026-09-27

## Purpose
Freeze the live-source findings for the UE 5.8 verification harness before implementation. This document is descriptive; it does not claim source changes that have not landed.

## Live source state
Audited on branch `automation/harden-ue-verification`:
- `Scripts/validate_automation_report.py` blob `e2a827ad9ae9b5a6e2d7b50df24ae51cdc929308`
- `Scripts/test_validate_automation_report.py` blob `9b402ae36db2fbacafa7cac17dac4132c4230aa8`
- `Scripts/Verify-Unreal.ps1` blob `9c48c0a14796862f117d508937fa14e4bee4cc33`
- `Scripts/verify-unreal.sh` blob `471e925bc1b276a85e66fcd2bfa595c2c0288705`
- `docs/CHECKPOINT.md` blob `246f6dd674c72030fb16804c2222e7fc9cb6ee09`

## Validator closure
`--expected-count` is already implemented: it rejects non-positive values, enforces an exact matching-test count, and records `expected_count` in the emitted summary. Do not reimplement it.

The remaining validator defect is namespace grammar. Replace the current starts-with/wildcard guard with a full match against:

`^TorqueAtlas\.(?:[A-Za-z_][A-Za-z0-9_]*\.)*$`

Accepted examples:
- `TorqueAtlas.`
- `TorqueAtlas.Vehicle.`
- `TorqueAtlas._Internal.`

Rejected examples:
- `TorqueAtlas`
- `TorqueAtlas..Vehicle.`
- `TorqueAtlas.Vehicle-Load.`
- `TorqueAtlas.9Vehicle.`
- prefixes containing whitespace, `*`, or `?`

## Regression closure
The current `notRun` and `inProcess` fixtures accidentally make aggregate counters total four while `tests[]` contains three entries. Each fixture must preserve an aggregate total of three:
- not-run fixture: succeeded=1, succeededWithWarnings=1, notRun=1
- in-process fixture: succeeded=1, succeededWithWarnings=1, inProcess=1

Add independent regressions for:
- expected_count=3 succeeds and summary preserves 3;
- expected_count=2 fails closed;
- every accepted/rejected namespace example above.

Each regression should prove one gate rather than fail earlier on aggregate reconciliation.

## Runner closure
Both runners currently duplicate `TorqueAtlas.` in three places: Unreal RunTest, validator --prefix, and metadata automation_filter. Replace this with one validated AutomationPrefix input. Add an optional positive ExpectedCount and forward it to the validator and metadata.

Required interfaces:
- PowerShell: `Verify-Unreal.ps1 -AutomationPrefix <prefix> [-ExpectedCount <positive-int>]`
- Bash: `verify-unreal.sh --automation-prefix <prefix> [--expected-count <positive-int>]`

No arguments must preserve the existing dynamic `TorqueAtlas.` root behavior.

Bash provenance must converge on PowerShell semantics: stable `started_utc`, terminal `finished_utc`, plus git SHA, UE version, platform/configuration, automation filter, expected count, build exit code, automation exit code, and terminal status. Do not replace start time on intermediate metadata writes.

## Diagnostic execution ladder
After Python regressions and source sanity pass, executable UE 5.8 verification is:
1. `TorqueAtlas.Suspension.Contact.Compliant.` — expected 2
2. `TorqueAtlas.Vehicle.StaticMassBalance.` — expected 3
3. `TorqueAtlas.Vehicle.StaticLoadComparison.` — expected 3
4. `TorqueAtlas.Vehicle.StaticLoad.` — expected 9
5. `TorqueAtlas.Suspension.Contact.` — expected 9
6. `TorqueAtlas.` — dynamic root count

An exact-count mismatch is a hard failure. Never fall back automatically to dynamic discovery.

## Checkpoint drift
`docs/CHECKPOINT.md` still describes the older execution sequence and must be synchronized after the harness source changes land. The physics state itself remains frozen: no suspension, tire, aero, load-sign, sampler, or provisional 2/3/3/2% acceptance parameter changes are justified before executable evidence.

## Exact continuation
1. Land strict validator grammar.
2. Repair and extend validator regressions.
3. Parameterize PowerShell.
4. Parameterize Bash and normalize provenance.
5. Run Python regressions and source sanity.
6. Synchronize CHECKPOINT.md.
7. Run UE 5.8 UHT/UBT and the 2/3/3/9/9 diagnostic ladder, then dynamic root.
8. Validate the stationary evidence contract and capture the first 120 Hz / 120-sample FL/FR/RL/RR baseline.
