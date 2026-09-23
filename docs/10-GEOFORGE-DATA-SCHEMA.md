# GeoForge Data Schema v0.1

Status: pre-implementation schema.

## 1. Goal
Convert legally usable geographic datasets into a deterministic intermediate representation that Unreal can consume without coupling game code directly to OpenStreetMap or any single source format.

## 2. Pipeline boundary
Source adapters -> normalized GeoForge database -> validation/enrichment -> export packages -> Unreal importer.

This separation is mandatory. Raw OSM tags must not become permanent gameplay APIs.

## 3. Spatial database
Recommended authoring/processing store: PostgreSQL + PostGIS.

All imported records retain source/provenance fields for licensing, debugging and regeneration.

## 4. Core entities

### GF_RoadNode
- id
- position geographic
- projected position
- elevation
- source/provenance
- confidence

### GF_RoadSegment
- id
- from_node / to_node
- centerline geometry
- road_class
- lane_count_forward/backward
- directionality
- width
- shoulder widths
- speed metadata
- surface material
- bridge/tunnel flags
- gradient/banking overrides
- access restrictions
- marking profile
- drainage class
- traffic density class
- source/confidence

### GF_Lane
- id
- parent road segment
- lane index
- direction
- width
- allowed vehicle classes
- turn permissions
- speed rule
- marking left/right

### GF_Intersection
- id
- connected segments
- control type
- priority model
- signal program reference
- lane connectors
- conflict zones
- manual-review flag

### GF_Structure
For bridges/tunnels/retaining structures:
- geometry reference
- type
- clearance
- deck/tunnel dimensions
- material/style hints
- connected roads

### GF_BuildingFootprint
- polygon
- height/floors when available
- use class
- roof hints
- regional style zone
- hero/ordinary flag

### GF_LandParcel
- polygon
- land-use class
- vegetation profile
- surface profile
- density parameters

### GF_WaterFeature
- geometry
- type
- level/flow hints
- shoreline class

### GF_TerrainTile
- tile ID
- bounds
- DEM source
- resolution
- min/max elevation
- quality/confidence

### GF_GameplayZone
Authored layer independent of source map data:
- race/event eligibility
- speed-camera/police rules if used
- spawn density
- service locations
- discovery region
- compression rules
- art-quality tier.

## 5. Coordinate strategy
Store original geographic coordinates plus a projected local coordinate representation. Export Unreal world chunks relative to stable regional origins to preserve precision. Do not permanently bake source coordinates into float-only gameplay assets.

## 6. Road classification normalization
Map heterogeneous source tags into a finite internal taxonomy such as:
- motorway
- trunk/expressway
- arterial
- collector
- local
- residential
- service
- track
- unpaved road
- dedicated race circuit.

Country/region rule packs translate markings, signs, lane widths and intersection conventions.

## 7. Confidence system
Every generated property can carry confidence:
- source-confirmed;
- inferred-high;
- inferred-low;
- authored override.

Low-confidence critical road geometry is queued for validation instead of silently accepted.

## 8. Gameplay compression metadata
Hybrid-scale transformations are explicit and reversible at pipeline level. Store:
- original geometry reference;
- compression region;
- compression ratio/rule;
- protected landmarks/intersections;
- transition zone.

Never apply arbitrary scaling independently per road segment; topology and driving rhythm must remain coherent.

## 9. Unreal export unit
Export by region/cell package rather than one monolithic map. Each package can contain:
- terrain tile references;
- road spline meshes;
- lane/traffic graph;
- buildings;
- foliage masks;
- water;
- signs/markings;
- gameplay metadata;
- provenance manifest.

## 10. Validation gates
GEO-001 disconnected road graph.
GEO-002 impossible lane transition.
GEO-003 bridge without valid approach.
GEO-004 tunnel/terrain collision.
GEO-005 excessive grade/curvature.
GEO-006 road below/above terrain unexpectedly.
GEO-007 duplicate crossing without intersection/grade separation.
GEO-008 missing traffic direction.
GEO-009 unsupported source licence/provenance.
GEO-010 streaming-cell boundary continuity.

## 11. Quality tiers
G0 Raw: normalized import.
G1 Clean: automated topology/geometry cleanup.
G2 Playable: collision, lanes, traffic and terrain validated.
G3 Art Pass: regional assets and visual polish.
G4 Hero: hand-authored landmark/circuit/city-center quality.

## 12. First pipeline proof
Use a deliberately small real-data test region, not the final world. Success criteria:
1. ingest roads + DEM;
2. build valid road graph;
3. generate drivable Unreal road splines;
4. preserve grade and major topology;
5. generate lane graph;
6. stream cells cleanly;
7. regenerate deterministically after source/tool changes.

## 13. Licensing rule
Every source adapter requires a documented licence/attribution entry before production use. GeoForge provenance must make it possible to identify the source of generated geographic content later.
