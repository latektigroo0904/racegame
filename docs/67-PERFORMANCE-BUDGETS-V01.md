# Performance Budgets & Simulation LOD v0.1

Updated: 2026-10-08
Status: provisional engineering budget

## 1. Principle
Performance targets are budgets to measure against, not claims.

## 2. Frame targets
Prototype:
- 60 FPS baseline;
- 120 FPS optional/high-end.

Player physics fixed step may run faster than render.

## 3. Player vehicle CPU target
Early target average:
- complete player vehicle physics substantially below 2.5 ms;
- severe crash may spike higher temporarily;
- tire/contact work target below ~0.5 ms where achievable.

All must be profiled on reference hardware.

## 4. Physics cadence
Candidate:
- vehicle/chassis: 240 Hz;
- tire: 240–480 Hz adaptive;
- structure: 120 Hz normal, higher during impact;
- thermal: 20–60 Hz;
- maintenance: 10–20 Hz.

Do not increase rate to hide unstable equations before diagnosing them.

## 5. Allocation rule
Hot fixed-step solvers:
- zero routine heap allocation;
- preallocated arrays;
- bounded iteration counts;
- instrumentation available.

## 6. Vehicle LOD
VLOD0 full player.
VLOD1 important nearby.
VLOD2 simplified physical.
VLOD3 kinematic.
VLOD4 statistical.

## 7. Structure LOD
Player/severe nearby:
full/reduced structural solver.
Other vehicles:
functional damage state + coarse deformation.

## 8. Tire LOD
TLOD0 full thermal/transient/wet.
TLOD1 reduced steady/transient.
TLOD2 lookup/simplified force.
Far agents: no tire solver.

## 9. Traffic budgets
Separate:
- logical agent update;
- planning;
- nearby control;
- physical vehicle.

Update frequencies scale with relevance.

## 10. World
Measure:
- streaming bandwidth;
- asset memory;
- HLOD;
- collision/nav/lane graph memory;
- surface cells.

## 11. Network
Budget:
- local opponent snapshots;
- traffic;
- reliable critical events;
- structure keyframes.

Dense node replication is prohibited.

## 12. Telemetry
Full developer telemetry may be expensive.
Modes:
- off;
- minimal;
- regression;
- full debug.

No disk write per physics substep synchronously.

## 13. Profiling identifiers
Every subsystem emits:
- mean;
- p95;
- max;
- call count;
- allocations.

## 14. Reference tiers
Define later:
- Minimum;
- Recommended;
- High;
- Ultra/Cinematic.

Physics correctness does not change by graphics tier.
Only simulation LOD for non-player actors may scale.

## 15. Failure policy
If over budget:
1. profile;
2. remove allocations/cache misses;
3. reduce redundant work;
4. introduce LOD/batching;
5. only then reconsider solver detail.

Do not reduce player physics correctness blindly.

## 16. Tests
PERF-001 fixed-step no unexpected allocations.
PERF-002 deterministic under same cadence.
PERF-003 LOD transition bounded.
PERF-004 traffic scaling curve measured.
PERF-005 world streaming hitch budget.
PERF-006 telemetry modes measured.

## 17. Acceptance
Every vertical-slice milestone includes a recorded performance baseline with hardware/build/scene provenance.
