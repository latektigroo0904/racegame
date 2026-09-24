# P1.1 Static Load Evidence Pipeline V0.1

Status: source-complete, UE 5.8 build-unverified.

## Purpose
Close the gap between live four-wheel runtime/contact evidence and the existing analytical static-mass-balance Proof-of-Physics oracle without feeding regression data back into physics.

## Canonical chain
`FTAFourWheelRuntimeState + FTAFourWheelStepOutput` -> `TAStaticLoadEvidenceAdapter` -> `TAStaticLoadSettledSampler` -> `TAStaticLoadComparison` -> `TAStaticLoadAcceptance`.

`TAStaticLoadEvidencePipeline` owns orchestration only. It does not solve suspension, contact, tires or chassis dynamics.

## Sign and corner ownership
The canonical `FTAWheelContactInput::VerticalLoadN` contract is consumed as an already non-negative support-load magnitude. The evidence adapter therefore performs **no sign inversion**. Negative or non-finite values are invalid evidence and fail closed.

Corner mapping is explicit and one-to-one:
- front axle left contact -> FL;
- front axle right contact -> FR;
- rear axle left contact -> RL;
- rear axle right contact -> RR.

This is the only normalization boundary for P1.1 live evidence. Downstream sampling/comparison must never reinterpret force direction.

## Cadence contract
Default evidence cadence is 120 Hz (`1/120 s`) with `1e-6 s` absolute tolerance. Cadence drift is rejected rather than silently changing the physical duration represented by the 120-sample settled window. At the default settings, 120 consecutive samples represent approximately one second of settled evidence.

This resolves the earlier ambiguity around a fixed sample count: **120 samples remains canonical only while the evidence adapter is run at the documented 120 Hz fixed physics cadence**. If the production solver later adopts another fixed cadence, the gate must be migrated deliberately with equivalent duration and regression evidence.

## Fail-closed rules
The adapter rejects invalid cadence/config, unsolved contacts, non-finite chassis velocities, and negative/non-finite support loads. Output is zeroed before validation.

The pipeline resets qualification history when the oracle is invalid, adapter evidence is invalid, or downstream comparison/acceptance cannot be evaluated. Ordinary excess motion or sample-to-sample load instability is processed by the settled sampler and resets its consecutive window.

## Acceptance semantics
A processed tick can be valid without yet being qualified. `bQualified` becomes true only after the complete consecutive settled window exists and comparison plus acceptance evaluation succeed. A qualified result may still have `Acceptance.bPass == false`; this is deliberate and distinguishes valid evidence of bad physics from invalid evidence.

## Regression coverage
Source tests cover canonical mapping, no-double-sign-normalization, vector-speed magnitude, cadence rejection, unsolved-contact rejection, negative-load rejection, warm-up, qualification, motion reset/recovery, deliberate axle-bias failure and cadence fail-closed reset.

## Risks / executable work
- C++ source has not yet been compiled by UE 5.8 UBT in this environment.
- Runtime `VerticalLoadN` provenance must be confirmed under executable contact/suspension tests; this document freezes the P1.1 consumer convention but does not replace solver validation.
- Default PoP tolerances remain provisional until real UE baseline captures exist.
- Aero asset ownership patch 41 remains an independent open closure item.

## Next checkpoint
1. UHT/UBT compile all new P1.1 sources.
2. Run `TorqueAtlas.Vehicle.StaticLoad.*` Automation tests.
3. Capture a real stationary four-wheel baseline at the fixed evidence cadence and compare measured settle duration/error distribution with the analytical oracle.
4. Only then tune acceptance thresholds or sampling duration.
