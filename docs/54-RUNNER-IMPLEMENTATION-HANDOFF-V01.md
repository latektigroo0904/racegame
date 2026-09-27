# 54 — Runner implementation handoff V01

Status: source patch prepared; GitHub source mutation blocked by write-safety gate  
Scope: `Scripts/Verify-Unreal.ps1`, then `Scripts/verify-unreal.sh`  
Baseline blobs:
- PowerShell: `9c48c0a14796862f117d508937fa14e4bee4cc33`
- Bash: `471e925bc1b276a85e66fcd2bfa595c2c0288705`

## PowerShell patch — exact implementation

The prepared source patch performs only these contract changes:

1. Extend `param` with:
   - `[string]$AutomationPrefix = "TorqueAtlas."`
   - `[Nullable[int]]$ExpectedCount = $null`
2. Fail before UE discovery/launch when a supplied `ExpectedCount <= 0`.
3. Metadata:
   - replace hardcoded `automation_filter` with `$AutomationPrefix`;
   - add `expected_test_count = $ExpectedCount`.
4. Build Unreal command from `"-ExecCmds=Automation RunTest $AutomationPrefix"`.
5. Build validator arguments once:
   `$ValidatorArgs = @($ReportValidator, $ReportRoot, "--prefix", $AutomationPrefix, "--summary-output", $ReportSummary)`
6. Append `@("--expected-count", $ExpectedCount.ToString())` only when count is supplied.
7. Invoke `python @ValidatorArgs`.
8. Preserve the existing PowerShell `started_utc` / terminal `finished_utc` lifecycle unchanged.

No physics or acceptance parameters are touched.

## Newly identified consistency requirement

The validator owns the canonical namespace grammar, but allowing a malformed runner prefix to launch UE before the validator rejects the report wastes an expensive engine run. A runner-side grammar preflight is therefore desirable, but duplicating the regex in three places creates drift risk.

Decision for the next implementation pass: do **not** silently introduce a second grammar contract into the runners. First land the parameter plumbing exactly as docs/53 specifies. Namespace syntax remains validator-authoritative. A later preflight should share a common validator/check entry point rather than copy the regex.

## Bash implementation plan

After the PowerShell patch lands:

- defaults: `AUTOMATION_PREFIX="TorqueAtlas."`, `EXPECTED_COUNT=""`;
- parse `--automation-prefix <value>` and `--expected-count <value>`;
- reject missing values and reject count unless it matches `^[1-9][0-9]*$`;
- capture `STARTED_UTC` exactly once before the first metadata write;
- metadata receives `automation_filter`, nullable `expected_test_count`, stable `started_utc`;
- write `finished_utc` only for `passed` / `failed`;
- remove `updated_utc` as lifecycle provenance;
- use `$AUTOMATION_PREFIX` for RunTest and validator;
- append validator `--expected-count` only when non-empty.

## Acceptance / source sanity

Before UE execution:

- validator regression suite passes;
- each runner contains only one declared default literal `TorqueAtlas.`;
- RunTest, validator and metadata all consume the resolved prefix variable;
- expected count is optional, positive when supplied, and propagated to validator + metadata;
- PowerShell/Bash expose equivalent lifecycle provenance;
- no vehicle-physics source changes are included.

## Risk register

- **R1 — source write gate:** direct update of `Scripts/Verify-Unreal.ps1` was rejected by the repository write-safety layer despite a fresh blob SHA. Mitigation: exact patch intent and baseline hashes are frozen here.
- **R2 — grammar duplication:** runner-side copied regex could diverge from validator. Mitigation: validator remains authoritative until a shared preflight mechanism exists.
- **R3 — shell parity:** Bash timestamp semantics currently differ from PowerShell. Mitigation: Bash provenance conversion is mandatory before executable evidence is accepted.

## Exact continuation checkpoint

1. Re-fetch `Scripts/Verify-Unreal.ps1`; if blob SHA is unchanged, apply the exact patch above.
2. Run/inspect validator regressions and source sanity.
3. Patch Bash per this document and docs/53.
4. Synchronize `docs/CHECKPOINT.md`.
5. Execute UE 5.8 UHT/UBT.
6. Execute the verification ladder: 2 -> 3 -> 3 -> 9 -> 9 -> dynamic `TorqueAtlas.` root.
7. Only after the harness is green, proceed to the stationary 120 Hz / 120-sample FL/FR/RL/RR evidence capture.

Physics/tire/aero/suspension/load-sign/sampler/acceptance tuning remains frozen until executable evidence exists.
