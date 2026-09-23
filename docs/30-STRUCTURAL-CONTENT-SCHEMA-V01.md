# Structural Content Schema v0.1

Status: implemented prototype authoring baseline
Updated: 2026-09-23

## Purpose
Move structural crash data, suspension bindings and functional damage routing out of C++ test fixtures and into versioned vehicle content.

## Authoring container
`FTAVehicleStructureAuthoringDefinition` is embedded in `UTAVehicleDefinition`.

Structure is optional for backward compatibility.

If no nodes are authored:
- empty structure configuration is valid;
- constraints/routes/bindings without nodes are invalid.

## Structural nodes
Authored node fields:
- vehicle-reference-local position;
- physical node mass;
- pinned flag.

Compilation:
```
Position_COMLocal =
Position_VehicleLocal - COM_VehicleLocal
```

Runtime initialization stores:
- reference position;
- current position;
- previous position;
- inverse mass;
- pinned state.

## Constraints
Authored:
- node A/B indices;
- compliance;
- yield strain;
- fracture strain;
- plastic flow;
- optional target component index.

Rest length is **not duplicated in authored data**.

Compilation derives:
```
RestLength =
|ReferenceNodeB - ReferenceNodeA|
```

This prevents stale rest-length values after moving authoring nodes.

Invalid/identical nodes and zero-length constraints fail compilation.

## Structural solver parameters
Authored:
- constraint iterations;
- max positional correction;
- max plastic rest-state change per step.

## Impact distribution
Authored:
- distribution radius;
- distance falloff exponent;
- deformation impulse fraction;
- max per-node delta velocity.

These values compile into the same configuration consumed by the collision/structure coupler.

## Suspension bindings
Authorable independently for:
- front-left upper/lower/tie-rod/damper pickups;
- front-right upper/lower/tie-rod/damper pickups;
- rear-left five link pickups + damper;
- rear-right five link pickups + damper.

Each binding contains:
- structural node indices;
- weights.

### Spatial validation
The compiler calculates the weighted undamaged structural binding centroid and compares it with the physical suspension pickup.

Prototype thresholds:
- > 0.10 m: warning;
- > 0.30 m: error.

This catches accidental binding of a suspension mount to a distant body region.

## Damage bridge authoring
Mount-damage entries contain:
- target component index;
- node indices;
- weights;
- ascending displacement thresholds.

Constraint target component indices compile from the constraints themselves.

An optional impact target component receives general structural impact-energy signals.

## Damage routing authoring
Routes contain:
- target component index;
- authoring consumer type;
- accepted signal types;
- impact-energy scale;
- full-crush displacement.

Current authoring consumer:
- Radiator.

Future consumer types can be added without changing generic `FTADamageSignal`.

## Compiled structure runtime
`FTAVehicleStructureCompiledConfig` contains:
- structural solver config;
- impact distribution config;
- immutable initial nodes;
- initial constraints;
- damage bridge config;
- damage routing config;
- four suspension binding groups.

Persistent runtime state is created separately so owned/damaged vehicles do not mutate the definition.

## Physics configuration hash
Structural node reference positions/masses, constraints and impact-distribution parameters participate in the current prototype physics hash.

Damage-routing parameters also participate.

Further binding/threshold hash coverage should be expanded before networking/replay lock.

## End-to-end content regression
Source regression now covers:

```
UTAVehicleDefinition
→ authored structural nodes/binding/route
→ BuildCompiledConfig
→ persistent damage runtime
→ collision
→ structure deformation
→ suspension offset
→ next-step altered alignment
```

## Known limitations
- no editor visualization yet;
- no material/joint profile asset library yet;
- only distance constraints are compiled;
- no authored deformation cage;
- no automatic structure generation from CAD/body topology;
- current target component IDs are integer prototype IDs;
- left/right structural authoring is manual rather than generated from symmetry;
- no savegame serialization/migration yet.

## Acceptance
v0.1 is accepted when authored structure data compiles into deterministic runtime arrays, invalid node/constraint/binding topology is rejected, and compiled bindings can drive the same crash-to-handling chain used by native test fixtures.
