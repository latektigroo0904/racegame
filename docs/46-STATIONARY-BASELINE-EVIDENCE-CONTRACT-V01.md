# P1.1 Stationary Baseline Evidence Contract V0.1

Status: design contract; executable evidence pending UE 5.8 verification.

## Purpose
Define the reproducible artifact produced by the first stationary four-wheel Proof-of-Physics baseline. This artifact is evidence only: it MUST NOT feed analytical or measured wheel loads back into vehicle physics, authored suspension, tire forces, or acceptance thresholds.

## Preconditions
A baseline is eligible for evaluation only after:
1. TorqueAtlasEditor builds under Unreal Engine 5.8.
2. The staged Automation gates pass for the exact tested commit.
3. The evidence pipeline qualifies 120 consecutive samples at 120 Hz (1/120 s) within the canonical 1e-6 s cadence tolerance.
4. No invalid evidence, motion, or instability reset occurs inside the accepted window.

## Artifact model

### provenance
Required:
- schema_version
- git_sha
- unreal_version
- physics_version
- vehicle_definition identifier and deterministic config/hash when available
- automation_filter and, for diagnostic gates, automation_expected_count
- capture timestamp in UTC

Unknown provenance MUST be represented explicitly as unknown/null and MUST NOT be fabricated.

### oracle
Required analytical inputs and outputs:
- mass_kg
- gravity_mps2
- com_x_from_rear_axle_m
- com_y_m
- wheelbase_m
- front_track_m
- rear_track_m
- expected_load_n: FL, FR, RL, RR

The oracle remains the canonical equilibrium expectation. Baseline measurements do not rewrite it.

### qualification
Required:
- sample_rate_hz = 120
- sample_interval_s = 1/120
- cadence_tolerance_s = 1e-6
- required_consecutive_samples = 120
- qualified_sample_count
- bQualified
- reset_count and reset reason(s), when available

A capture with bQualified=false is diagnostic evidence but is not an acceptance baseline.

### samples
For each qualified sample retain at minimum:
- sample_index
- elapsed_time_s or monotonic capture timestamp
- vertical_load_n: FL, FR, RL, RR

VerticalLoadN is a non-negative support-load magnitude. Direction remains owned by SuspensionForceWorldN.

### measurement
Derived only from the qualified sample window:
- mean_load_n: FL, FR, RL, RR
- measured_total_load_n
- measured_front_axle_load_n
- measured_rear_axle_load_n
- measured_left_side_load_n
- measured_right_side_load_n

### comparison
Primary corner errors are signed:
- corner_error_n = Measured - Expected for FL, FR, RL, RR
- total_error
- front_axle_error
- rear_axle_error
- left_side_error
- right_side_error

Absolute values may be derived for threshold checks but MUST NOT replace the signed primary errors.

### acceptance
Record both the configured envelope and result:
- total <= 2%
- front/rear axle <= 3%
- left/right side <= 3%
- max corner error <= 2% of expected total support load
- bPass
- failing metric(s)

bQualified and bPass are independent. A qualified failing baseline MUST be retained; it is valid evidence of a reproducible discrepancy.

## Failure semantics
Classify a failed qualified baseline before changing tolerances:
1. provenance/config mismatch;
2. authored setup/preload/static-compression mismatch;
3. sampling/qualification defect;
4. suspension/contact/load-sign defect;
5. analytical-oracle defect.

Do not widen thresholds merely to make a run pass.

## Recommended storage
Store each run under Saved/Verification/<timestamp>/ with:
- metadata.json
- automation-report-summary.json
- stationary-baseline.json
- optional stationary-baseline-samples.csv
- build/Automation logs already produced by the verification runner.

The JSON artifact is canonical. CSV is a convenience view only.

## Acceptance sequence
UE 5.8 UHT/UBT -> Compliant (2) -> StaticMassBalance (3) -> StaticLoadComparison (3) -> StaticLoad (9) -> Suspension.Contact (9) -> full TorqueAtlas. suite -> stationary baseline capture.

The full root suite intentionally has no fixed expected-count; diagnostic gates do.
