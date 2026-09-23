# Changelog

## 2026-09-23 — Numerical simulation design session
Added:
- `08-TIRE-SOLVER-NUMERICAL.md` with initial numerical force model, combined slip, load sensitivity, thermal/pressure/wear state, wet-surface model, continuous per-wheel aquaplaning, FFB coupling and regression tests;
- `09-STRUCTURAL-SOLVER-V02.md` with iterative compliance-based solver architecture, plasticity, fracture, joint types, adaptive impact solving, structural LOD and regression tests;
- `10-GEOFORGE-DATA-SCHEMA.md` with normalized geospatial entities, provenance/confidence, hybrid-scale metadata, export units and validation gates.

Decisions recorded:
- brush-inspired semi-empirical tire solver for prototype behind backend-neutral API;
- continuous per-wheel aquaplaning;
- XPBD-style structural compliance approach;
- normalized GeoForge intermediate representation rather than raw source tags in gameplay.

## 2026-09-23 — Documentation baseline
Added canonical repository documentation for:
- project vision;
- master GDD;
- technical architecture;
- tire/surface/steering specification;
- structural/mechanical damage specification;
- open-world and GeoForge direction;
- 60-brand concept pool;
- roadmap/MVP;
- architecture decision log;
- active checkpoint.

## Earlier design work consolidated
The repository baseline incorporates prior project decisions including:
- realistic open-world driving/racing simulator vision;
- maximum 100 fictional brands;
- real-map-data world generation;
- functional soft/structural damage target;
- crash-test sandbox;
- four realism profiles;
- ownership/maintenance/tuning;
- multiplayer and traffic;
- premium/non-pay-to-win business direction;
- Unreal + custom C++ simulation architecture;
- TA-P01 prototype vehicle;
- Proof-of-Physics development strategy.
