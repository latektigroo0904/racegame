# P1.1 Baseline Numeric Validation Policy V0.1

Status: design contract; independent validator implementation pending UE 5.8 verification.

## Purpose
Define the numerical consistency rules used when an independent validator reconstructs derived baseline evidence from primary `stationary-baseline.json` samples. This policy supplements docs/46 and docs/47. It does not change the physical acceptance envelope.

## Separation of concerns
Two tolerances exist for different reasons and MUST NOT be combined:

1. **Evidence-consistency tolerance** handles harmless floating-point/serialization differences between the UE/C++ producer and an independent validator.
2. **Physics acceptance tolerance** decides whether a qualified stationary baseline agrees with the analytical oracle.

Evidence-consistency tolerance MUST NOT widen, scale, soften or otherwise modify the canonical physical 2% total / 3% axle / 3% side / 2% maximum-corner envelope.

## Canonical consistency predicate
For stored value `s` and independently recomputed value `r`, consistency is:

`abs(s - r) <= abs_tol + rel_tol * max(abs(s), abs(r))`

Both operands MUST first pass finite-number and type validation. JSON booleans are not numbers for this contract.

## V0.1 tolerances
- Loads and forces: `abs_tol = 1e-6 N`, `rel_tol = 1e-9`.
- Derived time values: `abs_tol = 1e-9 s`, `rel_tol = 1e-9`.
- Dimensionless derived fractions: `abs_tol = 1e-12`, `rel_tol = 1e-9`.

The fixed-cadence qualification rule remains separate: adjacent accepted samples target `1/120 s` with the existing `1e-6 s` cadence tolerance.

These consistency values are deliberately much smaller than the physical acceptance envelope. They are serialization/recomputation guards, not model-fit parameters.

## Stable recomputation
The independent Python reference validator SHOULD use `math.fsum` when summing the 120 per-corner samples before division by sample count. Aggregate axle, side and total loads are then derived from the independently reconstructed corner means.

The validator MUST reconstruct:
1. four corner means;
2. total, front/rear and left/right aggregates;
3. all signed errors using `Measured - Expected`;
4. all acceptance metrics;
5. final `bPass` and stable failing metric identifiers.

Producer-derived fields are cross-check evidence only.

## Threshold semantics
Physical threshold equality passes. A reconstructed error exactly equal to its physical limit satisfies that gate.

The numerical consistency predicate is used only to determine whether a stored derived value faithfully represents the independently reconstructed value. It MUST NOT be added to a physical threshold before evaluating acceptance.

## Failure taxonomy
The validator SHOULD emit stable machine-readable failure identifiers:
- `ARTIFACT_MALFORMED`
- `PROVENANCE_INVALID`
- `QUALIFICATION_INVALID`
- `SAMPLE_INVALID`
- `CADENCE_INVALID`
- `ORACLE_MISMATCH`
- `MEASUREMENT_MISMATCH`
- `COMPARISON_MISMATCH`
- `ACCEPTANCE_MISMATCH`

Human-readable detail MAY accompany these identifiers but CI consumers SHOULD key on the identifier.

## Authority and diagnostic value
If producer-written and independently reconstructed acceptance disagree, the independent validator is authoritative for artifact validation.

A qualified artifact whose reconstructed physical acceptance fails remains valid diagnostic evidence. The validator distinguishes an internally contradictory artifact from a coherent qualified baseline that demonstrates a physics discrepancy.

## Change control
Do not loosen these numerical tolerances in response to a physical acceptance failure. A change requires reproducible evidence that cross-language or serialization behavior alone exceeds the current consistency bounds while the underlying primary evidence remains equivalent.

## Implementation checkpoint
After verification-harness hardening:
1. implement these rules in the independent baseline validator;
2. add positive and fail-closed fixtures around every numeric class;
3. prove threshold-equality behavior separately from serialization consistency;
4. run the validator against the first UE 5.8-produced 120 Hz stationary artifact;
5. keep any physics/tolerance decision downstream of that evidence.
