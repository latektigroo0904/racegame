# Regression Envelope V0.1

Date: 2026-09-23
Status: source implemented, UE execution pending

## Purpose
Torque Atlas needs numerical acceptance criteria before handling tuning can be trusted. A scenario must therefore produce a compact machine-comparable result rather than relying on visual inspection or subjective feel.

## Runtime primitive
`TARegressionEnvelope` summarizes one scalar trace without allocating during evaluation. It records sample count, minimum, maximum, arithmetic mean and a trailing steady-state mean. The caller supplies independent allowed ranges for observed minimum, observed maximum and steady-state mean.

## Scenario result identity
A future scenario report must contain at minimum:
- scenario ID;
- physics config hash;
- metric name;
- sample count;
- observed min/max/mean/steady-state;
- expected min/max/steady-state ranges;
- pass/fail;
- engine/toolchain identifier when produced by a trusted UE run.

## First proving-ground scenarios
1. `static_settle_flat` — chassis Z velocity, roll/pitch rate, four wheel loads.
2. `acceleration_rwd_flat` — longitudinal speed, engine RPM, driven-wheel slip.
3. `braking_flat` — stopping deceleration, wheel slip, yaw drift.
4. `constant_steer_flat` — yaw rate, lateral acceleration, front/rear slip balance.
5. `asymmetric_road_step` — FL/FR/RL/RR load transfer and roll response.
6. `front_corner_crash` — pickup displacement, alignment delta and tire-force delta.

## Baseline policy
No numeric range becomes canonical merely because a source test contains a threshold. Trusted envelopes require a successful UE 5.8 build/test run with commit SHA and physics config hash recorded. Initial ranges should be broad enough to catch sign errors, explosions and regressions; they can only tighten after repeated deterministic runs.

## Failure semantics
A metric fails when any required observed statistic is outside its configured inclusive range, the range itself is invalid, the trace is empty, or trace values are non-finite. Scenario aggregation should fail closed when a required metric cannot be evaluated.

## Determinism
Envelope extraction is deterministic for an identical ordered sample trace. It does not itself prove simulation determinism. Cross-run comparison must key on scenario ID and physics config hash and later compare repeated-run summaries and/or trace hashes.

## Risks
- unexecuted source thresholds may be unrealistic;
- transient peaks can be timestep-sensitive;
- steady-state windows that are too short can misclassify settling systems;
- broad ranges can hide slow drift;
- config hash coverage must remain synchronized with all handling-critical authored coefficients.

## Next implementation checkpoint
Add scenario/report structs and deterministic text/CSV serialization around `TARegressionEnvelope`, then feed them from the existing telemetry ring buffer outside the physics step.
