# ADR 41 — Telemetry ownership: compact applied-physics stream vs broad diagnostic stream

Status: **Accepted for Proof-of-Physics v2**  
Date: 2026-09-24

## Context
Torque Atlas currently has two telemetry sample families:

- `FTATelemetrySample` / `FTACompactTelemetryRingBuffer`: a small, direct vehicle-step stream. It already carries the exact `FTAVehicleStepOutput::Aerodynamics` result and is the source for aero regression extraction/reporting.
- `FTAVehicleTelemetrySample` / `FTATelemetryRingBuffer`: a broader diagnostic/regression stream with configuration identity, chassis state, wheel/tire/suspension channels, damage state, thermal state and profiling placeholders.

The overlap is real (tick, engine/cooling and aggregate tire-force channels), but the two types currently serve different data-shape requirements. Blindly copying aero into the broad type would create two independently maintained representations of the same applied force result and would make it unclear which one is canonical for aerodynamic regression evidence.

## Decision
For Proof-of-Physics v2, **retain both telemetry families, but make their ownership explicit rather than treating them as competing canonical samples.**

### Compact stream responsibility
The compact stream is the canonical **applied-step physics evidence** stream for channels whose correctness depends on preserving the exact result produced by `TAVehicleSimulation::Step`.

For aerodynamics specifically:

`FTAVehicleStepOutput::Aerodynamics -> FTATelemetrySample -> TAAeroRegressionMetrics -> TAAeroRegressionReport`

No downstream layer may recompute aerodynamic force, dynamic pressure, relative air speed or aero moment from vehicle speed/configuration when exact applied values are available.

### Broad stream responsibility
The broad stream is the canonical **vehicle diagnostic/state correlation** stream. It owns configuration hash, chassis state, per-wheel/per-tire/per-suspension channels, functional damage state, brake/tire thermal state and profiling data.

It may later expose aero-derived convenience channels only through an explicit adapter from the compact/applied result, never through a second aerodynamic solve.

## Why not merge now
A structural merge is deferred because it would touch mature regression/export surfaces while the repository is still UE 5.8 build-unverified. That creates migration risk without improving the current aerodynamic proof chain.

The safe sequence is:
1. close authored aero ownership in `UTAVehicleDefinition`;
2. execute UHT/UBT and Automation under UE 5.8;
3. inventory actual consumers/export schemas of both telemetry types;
4. decide whether to introduce a shared immutable core sample or a one-way compact-to-broad adapter;
5. migrate only with golden CSV/JSON regression coverage.

## Invariants
1. Physics is computed once.
2. Telemetry observes; telemetry does not solve vehicle physics.
3. Applied aero regression evidence comes from `FTAVehicleStepOutput::Aerodynamics`.
4. Broad telemetry must not become a second source of aerodynamic truth.
5. Existing export schemas remain stable until executable verification exists.
6. Any future consolidation requires explicit schema/version migration and golden-output tests.

## Risks
- Continued overlap can create maintenance drift if fields are added casually to both types.
- A future consumer may incorrectly choose the broad stream for aero evidence because it appears more comprehensive.
- Schema consolidation after external tooling exists will be more expensive.

## Mitigation
- Treat this ADR as the ownership contract.
- New exact-applied physics channels default to the compact stream until an explicit cross-stream adapter is designed.
- New broad diagnostic channels remain in `FTAVehicleTelemetrySample`.
- Do not duplicate aero fields into the broad stream during Proof-of-Physics v2.

## Follow-up acceptance gate
Revisit consolidation only after the canonical vehicle-definition aero property/call site is source-level closed and UE 5.8 build/test evidence is available.
