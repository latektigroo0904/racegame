# Automation Harness Implementation Manifest V0.1

Updated: 2026-09-27

## Purpose

This manifest turns docs/49 and docs/50 into an executable source-change checklist. It exists so the harness can be landed without re-deriving behavior after an interrupted session.

## Source changes

### Scripts/validate_automation_report.py

- Import `re`.
- Define one compiled namespace grammar:
  `^TorqueAtlas\.(?:[A-Za-z_][A-Za-z0-9_]*\.)*$`
- Validate `--prefix` with a full match before reading report data.
- Keep `--expected-count` optional and strictly greater than zero when supplied.
- Do not weaken the existing fail-closed checks for aggregate counters, foreign tests, failures, not-run tests, in-process tests, per-test errors, or terminal failure states.

### Scripts/test_validate_automation_report.py

Extend `run_validator()` with optional `expected_count` and append `--expected-count` only when supplied.

Required independent regressions:
1. valid three-test report succeeds;
2. expected count 3 succeeds and summary records 3;
3. expected count 2 fails;
4. malformed/foreign prefix fails;
5. namespace matrix accepts `TorqueAtlas.`, `TorqueAtlas.Vehicle.`, and `TorqueAtlas.Vehicle.StaticLoad.`;
6. namespace matrix rejects missing trailing dots, empty segments, whitespace, wildcard characters, leading digits in a segment, and non-identifier punctuation;
7. failed aggregate rejects with aggregate total still equal to `tests[]` length;
8. notRun rejects with aggregate total still equal to `tests[]` length;
9. inProcess rejects with aggregate total still equal to `tests[]` length;
10. malformed aggregate counters, malformed `tests[]`, non-object root, empty report, foreign test paths, per-test errors and failure states each fail independently.

For the existing three-test fixture, negative aggregate tests must preserve total=3. Example notRun fixture: succeeded=1, succeededWithWarnings=1, notRun=1.

### Scripts/Verify-Unreal.ps1

Add:
- `[string]$AutomationPrefix = "TorqueAtlas."`
- optional positive integer `$ExpectedCount`

Validate the prefix before launching Unreal. Use exactly the same value for:
- `Automation RunTest`;
- validator `--prefix`;
- metadata `automation_filter`.

When ExpectedCount is supplied:
- pass it to validator as `--expected-count`;
- record it as `expected_test_count`.
When omitted, record JSON null and keep the root suite dynamic.

### Scripts/verify-unreal.sh

Add:
- `--automation-prefix <prefix>`;
- `--expected-count <positive integer>`.

Mirror the PowerShell validation and single-source behavior. Normalize provenance to preserve `started_utc` from process start and write terminal `finished_utc` on pass/fail rather than replacing the only timestamp on each metadata write.

## Canonical staged verification

| Prefix | Expected count |
| --- | ---: |
| `TorqueAtlas.Suspension.Contact.Compliant.` | 2 |
| `TorqueAtlas.Vehicle.StaticMassBalance.` | 3 |
| `TorqueAtlas.Vehicle.StaticLoadComparison.` | 3 |
| `TorqueAtlas.Vehicle.StaticLoad.` | 9 |
| `TorqueAtlas.Suspension.Contact.` | 9 |
| `TorqueAtlas.` | dynamic |

A count mismatch is a hard failure. It must never silently fall back to dynamic discovery.

## Verification order

1. Run `python Scripts/test_validate_automation_report.py`.
2. Run `python Scripts/source_sanity.py`.
3. Run UE 5.8 UHT/UBT for TorqueAtlasEditor Development.
4. Execute the five exact-count gates in the table.
5. Execute the dynamic TorqueAtlas root suite.
6. Validate stationary baseline artifacts against docs/46-48.
7. Capture the first real 120 Hz, 120-consecutive-sample FL/FR/RL/RR baseline.

## Frozen physics boundary

This harness work must not alter suspension, tire, aero, load-sign, settled-sampler or acceptance-envelope parameters. The provisional physical envelope remains total 2%, front/rear 3%, left/right 3%, max-corner 2% of expected total support load until executable stationary evidence justifies a change.

## Risks

- Source writes may be blocked independently of documentation writes.
- UE 5.8 executable verification still requires an engine-capable environment.
- Existing notRun/inProcess regressions can produce false confidence until their aggregate totals are corrected.
- A test-count mismatch can indicate either accidental test disappearance or unexpected namespace growth; both require diagnosis, not automatic acceptance.

## Exact continuation checkpoint

Land source changes in this order: validator -> regressions -> PowerShell runner -> Bash runner/provenance. Then execute Python regressions and source sanity. Only after those pass, synchronize docs/CHECKPOINT.md and proceed to UE 5.8 UHT/UBT and the 2/3/3/9/9 staged Automation ladder.
