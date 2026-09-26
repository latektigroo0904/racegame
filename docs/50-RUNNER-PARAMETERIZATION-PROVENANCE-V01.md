# Runner Parameterization and Provenance Plan V01

Status: implementation-ready design for branch `automation/harden-ue-verification`.

## Purpose
Close the remaining gap between the Automation namespace contract and the two UE verification runners without changing vehicle physics.

## Canonical CLI contract
PowerShell:
`Verify-Unreal.ps1 -AutomationPrefix <prefix> [-ExpectedCount <positive-int>]`

Bash:
`verify-unreal.sh --automation-prefix <prefix> [--expected-count <positive-int>]`

Defaults preserve the full dynamic root suite: `TorqueAtlas.` with no exact count.

## Single-source invariant
The validated AutomationPrefix value MUST be the same value supplied to:
1. Unreal `Automation RunTest`;
2. `validate_automation_report.py --prefix`;
3. metadata `automation_filter`.

When ExpectedCount is present, the same positive integer MUST be supplied to:
1. validator `--expected-count`;
2. metadata `expected_test_count`.

No fallback from a count mismatch to dynamic counting is permitted.

## Staged diagnostic gates
| Prefix | ExpectedCount |
|---|---:|
| TorqueAtlas.Suspension.Contact.Compliant. | 2 |
| TorqueAtlas.Vehicle.StaticMassBalance. | 3 |
| TorqueAtlas.Vehicle.StaticLoadComparison. | 3 |
| TorqueAtlas.Vehicle.StaticLoad. | 9 |
| TorqueAtlas.Suspension.Contact. | 9 |
| TorqueAtlas. | dynamic |

## Provenance normalization
Both runners must persist:
- started_utc once, before build/Automation work;
- finished_utc exactly once on terminal pass/fail;
- git_sha;
- Unreal version;
- target/platform/configuration;
- automation_filter;
- expected_test_count (null for dynamic root);
- build_exit_code;
- automation_exit_code;
- terminal status and failure reason when applicable.

Bash currently rewrites only updated_utc; its implementation should be brought into parity with PowerShell rather than preserving that asymmetry.

## Validation ordering
1. Validate CLI namespace syntax and ExpectedCount before invoking Unreal.
2. Generate/build UE 5.8.
3. Run only the requested Automation namespace.
4. Reject non-zero Automation process exit.
5. Validate exported JSON with identical prefix/count.
6. Mark metadata passed only after JSON validation succeeds.

## Regression obligations
The Python validator suite must independently prove:
- valid dynamic root;
- valid exact count;
- exact-count mismatch rejection;
- malformed namespace rejection;
- tests outside requested prefix rejection;
- aggregate reconciliation;
- failed/notRun/inProcess rejection without aggregate masking;
- malformed test entries/counters rejection.

## Non-goals
This change must not alter suspension, tires, aero, contact-load signs, sampler cadence, settled-window length, or the provisional 2/3/3/2 percent acceptance envelope.

## Implementation order
1. strict validator namespace grammar;
2. validator regression repairs/extensions;
3. PowerShell parameterization;
4. Bash parameterization plus provenance parity;
5. local Python regressions and source_sanity;
6. UE 5.8 UHT/UBT;
7. staged 2/3/3/9/9 Automation gates;
8. dynamic root suite;
9. stationary baseline capture under docs/46-48.
