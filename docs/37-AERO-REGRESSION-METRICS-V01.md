# Aero Regression Metrics v0.1

Updated: 2026-09-23

## Decision
Regression reporting consumes the exact aerodynamic result already copied into `FTATelemetrySample`; it must never re-run or approximate the aero solver.

## Canonical scalar channels
`TAAeroRegressionMetrics::Extract` derives:
- relative air speed (m/s);
- dynamic pressure (Pa);
- total aerodynamic force magnitude (N);
- drag-axis force (N), positive when force opposes the scenario forward axis;
- vertical aerodynamic force (N), negative for downforce when +Z/up is supplied;
- pitch torque (N m), projected on the scenario right axis.

The caller supplies normalized-able forward/up world axes. Degenerate, parallel, non-finite or invalid telemetry input is rejected instead of silently producing a regression value.

## Ownership
`FTAVehicleStepOutput::Aerodynamics` -> `TATelemetry::MakeSample` -> `FTATelemetrySample` -> `TAAeroRegressionMetrics::Extract` -> regression envelope/report.

This preserves one physics source of truth and keeps reporting frame policy separate from the aero solver.

## Runtime integration status
`TAVehicleSimulation::Step` now calls `TAVehicleAerodynamicsBridge::AddToChassis` from the same pre-integration chassis state used by tire/suspension forces. Aero contributes to the shared `FTAChassisForceAccumulator` before the single chassis integration, and `FTAVehicleStepOutput::Aerodynamics` carries the exact solver result into telemetry/regression reporting. The previous zero-aero full-step integration risk is therefore source-closed.

Remaining risk: this path is still UE 5.8 executable-unverified; generated code, compiler/linker behavior, module loading and Automation remain the acceptance gate.

## Acceptance
Automation tests cover scalar projection/sign conventions and invalid-frame rejection. UE 5.8 executable verification remains required before baseline promotion.
