# Open World & GeoForge Specification — v0.1

## Objective
Build a scalable drivable world from legal geographic datasets, then enhance it for gameplay and visual quality.

## Primary inputs
Potential categories:
- OpenStreetMap road/building/infrastructure data;
- open elevation/DEM data;
- open land-use/water/vegetation datasets;
- internally authored metadata.

All datasets require licence review and attribution compliance before production use.

## GeoForge pipeline
```
Raw geographic data
→ import
→ normalization/cleaning
→ road graph
→ terrain reconstruction
→ road splines
→ intersection generation
→ bridge/tunnel handling
→ building footprints
→ vegetation/land-use
→ traffic metadata
→ gameplay validation
→ artist pass
→ World Partition build
```

Recommended spatial database: PostgreSQL + PostGIS.

## Road segment data
```
centerline
width
lane count
direction
speed limit
surface
gradient
banking
markings
traffic rules
collision profile
AI metadata
```

## Intersection generator
Must infer and/or author:
- lane connections;
- turn permissions;
- priority;
- traffic lights;
- pedestrian crossings where supported;
- medians/islands;
- merge and yield behaviour.

Low-confidence intersections are flagged for manual review.

## Buildings
Use footprints plus metadata to drive procedural regional kits.
Hero landmarks are hand-authored.

## Hybrid scale
- G/Hero regions: near 1:1.
- Secondary urban/regional: ~1:2.
- Long transit links: ~1:3 to 1:6.

Compression removes uneventful distance while preserving orientation, major topology and landmarks.

## Quality levels
```
G0 Raw
G1 Clean
G2 Playable
G3 Art Pass
G4 Hero
```

## Streaming
Use Unreal World Partition + HLOD.
Conceptual hierarchy:
- macro region;
- streaming cell;
- detail cell.

Streaming distance adapts to vehicle speed; a car at 300 km/h requires much farther predictive loading than one at 30 km/h.

## Traffic LOD
- near: full vehicle simulation;
- mid: simplified vehicle;
- far: graph/agent simulation only.

## Surface state
Important road cells can store:
```
Temperature
Wetness
WaterDepth
Snow
Ice
Dirt
RubberLevel
Drainage
```

These feed directly into the tire solver.

## First world prototype
After Proof of Physics:
- ~20–25 km²;
- one small urban area;
- industrial area;
- motorway;
- country roads;
- mountain sector.

## MVP world
~50–100 km²:
- one city;
- suburbs;
- motorway network;
- rural roads;
- test/race facility;
- AI traffic;
- weather;
- basic multiplayer.

Do not expand geographic scope before physics, damage and streaming budgets are proven.
