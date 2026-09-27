# 53 — Automation runner parameter contract V01

Status: implementation-ready  
Scope: `Scripts/Verify-Unreal.ps1`, `Scripts/verify-unreal.sh`, validator regression closure  
Branch baseline: `automation/harden-ue-verification` @ `a70037ce27b66bdb2eb68d50a9a8d466610e1428`

## Decision

The Automation namespace and optional expected test count are runner inputs and MUST be single-source values. The same resolved values MUST drive:

1. Unreal `Automation RunTest`;
2. `validate_automation_report.py --prefix`;
3. validator `--expected-count` when supplied;
4. verification metadata.

Default namespace remains `TorqueAtlas.`. Expected count is optional because the dynamic root suite can grow; fixed-count ladder gates may opt in.

## PowerShell contract

Add parameters:

- `[string]$AutomationPrefix = "TorqueAtlas."`
- `[Nullable[int]]$ExpectedCount = $null`

Reject non-positive `ExpectedCount` before Unreal is launched.

Metadata MUST include:

- `automation_filter = $AutomationPrefix`
- `expected_test_count = $ExpectedCount`

Construct `-ExecCmds=Automation RunTest $AutomationPrefix` from the resolved parameter.

Construct validator arguments once. Append `--expected-count $ExpectedCount` only when a value is supplied.

Preserve current stable `started_utc` and terminal `finished_utc` semantics.

## Bash contract

Add arguments:

- `--automation-prefix <prefix>`
- `--expected-count <positive-int>`

Defaults:

- `AUTOMATION_PREFIX="TorqueAtlas."`
- `EXPECTED_COUNT=""`

Reject missing argument values, non-integers, zero and negative counts before Unreal is launched.

Use `$AUTOMATION_PREFIX` for RunTest, validator and metadata. Append validator expected-count arguments only when non-empty.

Metadata MUST include:

- stable `started_utc` captured once before first metadata write;
- terminal `finished_utc` only for passed/failed states;
- `automation_filter`;
- nullable `expected_test_count`.

This replaces the current Bash-only `updated_utc` model and brings provenance into parity with PowerShell.

## Regression closure

`Scripts/test_validate_automation_report.py` must:

- allow `run_validator(..., expected_count=None)`;
- make the notRun fixture aggregate-consistent: succeeded=1, warnings=1, notRun=1 and one test state NotRun;
- make the inProcess fixture aggregate-consistent: succeeded=1, warnings=1, inProcess=1 and one test state InProcess;
- prove expected_count=3 succeeds and is written to the summary;
- prove expected_count=2 fails closed;
- accept grammar: `TorqueAtlas.`, `TorqueAtlas.Vehicle.`, `TorqueAtlas._Internal.`;
- reject grammar: `TorqueAtlas`, `TorqueAtlas..Vehicle.`, `TorqueAtlas.Vehicle-Load.`, `TorqueAtlas.9Vehicle.`, wildcards and leading/trailing whitespace.

For narrower valid prefixes, rewrite all fixture test paths into that prefix so grammar validation is isolated from report-membership validation.

## Acceptance

Before UE execution:

1. Python validator regression suite passes.
2. Source sanity passes.
3. No independent hardcoded `TorqueAtlas.` remains in runner execution/validator/metadata paths except the declared default.
4. PowerShell and Bash metadata expose equivalent provenance fields.

Then execute: UE 5.8 UHT/UBT -> 2 -> 3 -> 3 -> 9 -> 9 -> dynamic `TorqueAtlas.` root.

Physics/tire/aero/suspension/load-sign/sampler/acceptance tuning remains frozen until executable evidence is collected.
