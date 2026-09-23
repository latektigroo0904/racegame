# Telemetry v0.1

Status: implementation baseline
Updated: 2026-09-23

## Purpose
Provide deterministic, low-overhead capture of vehicle physics state for regression tests, debugging and future profiling.

## Architecture
`TA_Telemetry` depends on the public `TA_Vehicle` runtime API but the vehicle solver does not depend on telemetry.

This preserves one-way observation:
```
vehicle solver
→ telemetry capture
```

Telemetry is not part of vehicle physics truth.

## Ring buffer
`FTATelemetryRingBuffer`:
- fixed capacity;
- storage allocated during initialization;
- push overwrites oldest sample after capacity;
- chronological read API;
- reset preserves allocated storage.

No steady-state allocation is required by the ring buffer.

## Prototype sample
Each sample currently includes:
- simulation tick;
- engine RPM;
- selected gear;
- clutch temperature;
- coolant temperature;
- cooling efficiency;
- chassis linear velocity;
- chassis angular velocity;
- total longitudinal/lateral force;
- four prototype wheel slip ratios;
- four slip angles;
- four longitudinal tire forces;
- four lateral tire forces;
- reserved subsystem timing fields.

The fixed four-wheel channels are appropriate for TA-P01 only. A scalable telemetry layout for future 6x6/8x8 vehicles is deferred.

## Timing fields
Reserved:
- VehicleSolverMs;
- TireSolverMs;
- PowertrainSolverMs;
- StructureSolverMs;
- DamageSolverMs.

These must be populated by instrumentation outside the mathematical solver code.

## Tests
Current Automation test validates:
- initialization;
- fixed capacity;
- wraparound;
- chronological ordering;
- reset while retaining storage.

## Future exports
Planned:
- CSV;
- JSON/binary trace;
- crash-event markers;
- comparison against regression envelopes;
- plotting tools;
- test report metadata with build commit/config hash.

## Rule
Telemetry may observe, compare and report physics. It must never silently alter simulation values.
