# P1.1 Stationary Baseline Artifact Schema V0.1

Status: design contract; executable producer/validator pending UE 5.8 verification.

## Purpose
Define the canonical machine-readable `stationary-baseline.json` contract for the first stationary four-wheel Proof-of-Physics baseline. The artifact is evidence only and MUST NOT feed measured or analytical wheel loads back into vehicle physics, authored suspension, tire forces, or acceptance thresholds.

This schema refines docs/46. When this document and docs/46 differ, the stricter fail-closed interpretation applies until an explicit versioned decision changes the contract.

## Top-level structure
A baseline artifact MUST be a JSON object containing exactly these logical sections:
- `provenance`
- `oracle`
- `qualification`
- `samples`
- `measurement`
- `comparison`
- `acceptance`

Additional producer diagnostics MAY be added under a dedicated `diagnostics` object, but MUST NOT alter validation or acceptance semantics.

## Numeric rules
All numeric evidence MUST be finite JSON numbers.
- NaN and +/-Infinity are invalid.
- JSON booleans MUST NOT be accepted as numeric values.
- Support loads, mass, gravity magnitude, wheelbase, track widths, sample rate, interval, cadence tolerance and counts that are physically/count constrained MUST be non-negative or strictly positive as specified below.
- Derived values MUST be independently recomputable from primary evidence.

## provenance
Required fields:
- `schema_version`: exactly `"0.1"`
- `git_sha`: tested commit SHA; MUST NOT be empty
- `unreal_version`: tested UE version; MUST NOT be empty
- `physics_version`: version identifier or explicit null when genuinely unavailable
- `vehicle_definition`: stable identifier; MUST NOT be empty
- `vehicle_config_hash`: deterministic hash when available, otherwise explicit null
- `automation_filter`: validated TorqueAtlas namespace used for the qualifying run
- `automation_expected_count`: positive integer for diagnostic gates; null for the full `TorqueAtlas.` root suite
- `captured_at_utc`: UTC timestamp

Unknown values MUST be null where permitted and MUST NOT be fabricated.

## oracle
Required primary inputs:
- `mass_kg > 0`
- `gravity_mps2 > 0`
- `com_x_from_rear_axle_m`
- `com_y_m`
- `wheelbase_m > 0`
- `front_track_m > 0`
- `rear_track_m > 0`

Required expected output:
`expected_load_n` with finite, non-negative `FL`, `FR`, `RL`, `RR`.

The validator SHOULD independently recompute equilibrium expectations from the primary oracle inputs when the canonical analytical formula is available. Stored expected values MUST NOT silently override recomputed values.

## qualification
Required:
- `sample_rate_hz = 120`
- `sample_interval_s = 1/120`
- `cadence_tolerance_s = 1e-6`
- `required_consecutive_samples = 120`
- `qualified_sample_count`
- `bQualified`
- `reset_count >= 0`
- `reset_reasons`: array, empty when no resets were observed

For an acceptance-eligible artifact:
- `bQualified` MUST be true.
- `qualified_sample_count` MUST equal 120.
- `samples` MUST contain exactly those 120 accepted consecutive samples.

An unqualified diagnostic capture MAY contain fewer samples, but MUST NOT report `acceptance.bPass=true`.

## samples
Each accepted sample MUST contain:
- `sample_index`
- `elapsed_time_s`
- `vertical_load_n`: `FL`, `FR`, `RL`, `RR`

Validation invariants:
1. `sample_index` starts at 0 and increments by exactly 1 without gaps or duplicates.
2. `elapsed_time_s` is finite and strictly increasing.
3. Adjacent sample deltas MUST satisfy the canonical 120-Hz interval within `cadence_tolerance_s`.
4. Every corner `VerticalLoadN` is finite and >= 0.
5. Acceptance-eligible artifacts contain exactly 120 samples.

The sample array is primary measurement evidence. Stored aggregates are secondary and MUST be checked against it.

## measurement
Required derived fields:
- `mean_load_n`: FL, FR, RL, RR
- `measured_total_load_n`
- `measured_front_axle_load_n`
- `measured_rear_axle_load_n`
- `measured_left_side_load_n`
- `measured_right_side_load_n`

The independent validator MUST recompute corner means directly from `samples`, then derive all aggregate loads. A stale or contradictory stored aggregate invalidates the artifact.

Canonical identities:
- total = FL + FR + RL + RR
- front = FL + FR
- rear = RL + RR
- left = FL + RL
- right = FR + RR

## comparison
Primary errors remain signed:
- `corner_error_n.FL = measured.FL - expected.FL`
- `corner_error_n.FR = measured.FR - expected.FR`
- `corner_error_n.RL = measured.RL - expected.RL`
- `corner_error_n.RR = measured.RR - expected.RR`
- `total_error_n = measured_total - expected_total`
- `front_axle_error_n = measured_front - expected_front`
- `rear_axle_error_n = measured_rear - expected_rear`
- `left_side_error_n = measured_left - expected_left`
- `right_side_error_n = measured_right - expected_right`

The validator MUST recompute these fields. Absolute errors MAY be emitted as convenience values but are never the canonical signed comparison evidence.

## acceptance
Required configured envelope:
- `max_total_error_fraction = 0.02`
- `max_axle_error_fraction = 0.03`
- `max_side_error_fraction = 0.03`
- `max_corner_error_fraction_of_expected_total = 0.02`

Required result:
- `bPass`
- `failing_metrics`: array of stable metric identifiers

The validator MUST independently recompute acceptance from oracle + recomputed measurement/comparison values. A producer-written `bPass` is evidence to cross-check, not authority.

`bQualified` and `bPass` are independent:
- qualified + pass: valid passing baseline
- qualified + fail: valid diagnostic evidence of a reproducible discrepancy
- unqualified + fail: diagnostic capture only
- unqualified + pass: invalid artifact

## Fail-closed conditions
Reject acceptance eligibility when any of the following occurs:
- malformed/non-object JSON root;
- missing required section/field;
- non-finite or wrongly typed numeric evidence;
- boolean accepted as a number;
- negative support load;
- malformed or non-contiguous sample indices;
- non-monotonic or off-cadence sample timing;
- sample count inconsistent with qualification;
- stored means/aggregates inconsistent with recomputation;
- stored signed errors inconsistent with recomputation;
- stored acceptance inconsistent with recomputation;
- invalid TorqueAtlas automation namespace;
- diagnostic expected-count missing/non-positive where required;
- full-root expected-count incorrectly fixed;
- provenance/config contradiction.

## Validator responsibility
The future independent validator MUST treat only primary evidence as authoritative:
1. validate provenance and qualification;
2. validate every raw sample;
3. recompute means;
4. recompute aggregate loads;
5. recompute signed comparisons;
6. recompute threshold results;
7. compare recomputed values against stored derived fields;
8. emit a compact validation summary and non-zero exit code on any contradiction.

This deliberately prevents the capture producer from proving its own derived result.

## Storage
Canonical path per run:
`Saved/Verification/<timestamp>/stationary-baseline.json`

Optional convenience export:
`Saved/Verification/<timestamp>/stationary-baseline-samples.csv`

JSON remains canonical; CSV MUST NOT be used as the acceptance authority.

## Implementation checkpoint
Do not implement producer-driven tolerance changes. Next executable work after harness hardening is:
1. land strict TorqueAtlas namespace grammar and regression coverage;
2. parameterize both verification runners with AutomationPrefix and optional ExpectedCount;
3. synchronize provenance metadata and docs/CHECKPOINT.md;
4. pass source-sanity;
5. run UE 5.8 UHT/UBT and the 2/3/3/9/9 staged gates plus dynamic root suite;
6. implement baseline producer + independent validator against this contract;
7. capture the first real 120-Hz FL/FR/RL/RR baseline.
