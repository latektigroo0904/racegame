# Proof-of-Physics → World Prototype Transition v0.1

Updated: 2026-10-08
Status: implementation-ready transition architecture

## 1. Purpose

Define the controlled transition from one high-fidelity vehicle/proving-ground runtime to the first open-world Torque Atlas prototype.

The world must consume the vehicle simulation through explicit interfaces rather than injecting shortcuts into vehicle physics.

## 2. Entry gate

World prototype work may begin in parallel on data/tooling, but integrated vehicle/world promotion should follow:
- UE 5.8 build green;
- full vehicle Automation green;
- static/load baseline archived;
- minimum braking/steering/aero/driveline proving-ground traces archived;
- canonical fixed timestep frozen for prototype.

## 3. World prototype target

First integrated region:
- 20–25 km²;
- small town;
- industrial zone;
- highway;
- country roads;
- mountain/grade segment;
- proving-ground/test loop.

Purpose:
- road variety;
- streaming;
- AI traffic;
- weather/surface coupling;
- vehicle persistence;
- damage/recovery.

Not visual-content scale.

## 4. Data layers

### Geo source layer
- road centerlines;
- junctions;
- elevation;
- building footprints;
- water;
- land use;
- administrative/reference metadata.

### Normalized GeoForge layer
- RoadNode;
- RoadSegment;
- Junction;
- Lane;
- SurfaceZone;
- TerrainTile;
- BuildingFootprint;
- WaterBody;
- POI;
- TrafficRuleZone.

### Runtime world layer
- splines/meshes;
- collision;
- lane graph;
- surface-query cells;
- traffic spawn graph;
- streaming cells;
- HLOD.

Raw OSM/source tags never become gameplay truth directly.

## 5. Road segment contract

A runtime road segment should expose:
- stable ID;
- geometry/spline;
- lane count/direction;
- lane widths;
- speed rule;
- road class;
- surface material;
- banking/superelevation;
- longitudinal grade;
- junction connectivity;
- traffic permissions;
- surface-state reference.

## 6. Surface query boundary

Vehicle physics asks the world for a compact surface sample.

World returns:
- material;
- local normal;
- temperature;
- wetness;
- water depth;
- snow;
- ice;
- loose material;
- roughness;
- drainage;
- rubber/dirt contamination.

Vehicle/tire solver decides forces.

World does not calculate tire grip.

## 7. Weather coupling

Weather system owns:
- precipitation;
- ambient temperature;
- wind field;
- solar/time input;
- cloud/visibility state.

Surface system owns:
- accumulated water;
- evaporation;
- runoff/drainage;
- snow/ice state;
- road temperature.

Aerodynamics receives world wind/air density through environment input.

## 8. Streaming hierarchy

Initial target:
- macro partition: several-km world regions;
- runtime streaming cells: ~1–2 km;
- local detail cells: hundreds of metres;
- HLOD for distant environment.

Exact sizes remain profiling decisions.

Vehicle physics state must not depend on render LOD.

## 9. Coordinate precision

Use Unreal large-world coordinate support.

Physics adapters convert world coordinates to local simulation-safe representations where required.

Avoid:
- storing geographic degrees directly in hot vehicle physics;
- mixing centimetres and metres implicitly;
- rebasing structural node-local coordinates with world origin.

## 10. Traffic lane graph

Lane graph contains:
- lane center path;
- direction;
- legal connectivity;
- priority;
- speed rule;
- stop/yield control;
- merge/split;
- signal reference.

Traffic pathfinding operates on lanes/road graph.

Nearby vehicles may upgrade from kinematic/agent representation to richer rigid vehicle LOD.

## 11. Vehicle simulation LOD

### VLOD0
Player/high-interest:
- full tire;
- suspension;
- powertrain;
- functional damage;
- structure where needed.

### VLOD1
Nearby important traffic:
- rigid chassis;
- simplified wheel/suspension;
- core tire forces;
- reduced damage.

### VLOD2
Nearby ordinary traffic:
- simplified vehicle dynamics.

### VLOD3
Distant:
- path-following rigid/kinematic.

### VLOD4
Far:
- statistical agent.

Transitions must preserve:
- transform;
- velocity;
- route;
- critical damage state;
- identity.

## 12. Traffic behavior state

Driver profile:
- patience;
- awareness;
- aggression;
- skill;
- risk tolerance;
- lawfulness.

Behavior:
- follow lane;
- car-following;
- lane change;
- merge;
- yield;
- junction;
- incident reaction.

Do not use profile to violate physical acceleration/braking limits silently.

## 13. Incidents

World incident system can represent:
- collision;
- stopped vehicle;
- road closure;
- debris;
- weather hazard;
- emergency response.

Traffic reroutes based on graph availability.

## 14. Player persistence

World/garage layer stores vehicle instance:
- VIN/instance ID;
- mileage;
- engine hours;
- installed parts;
- fuel;
- battery state;
- tire state;
- wear;
- damage;
- repair history.

Vehicle definition remains immutable authored specification.

## 15. Recovery

Prototype recovery modes:
- roadside reset for testing;
- tow/recovery service for gameplay;
- garage repair.

Reset must be explicit gameplay/tool action, not automatic physics correction.

## 16. GeoForge build pipeline

```
Source datasets
→ ingest
→ normalize
→ topology validation
→ road/lane classification
→ terrain reconstruction
→ road spline generation
→ junction generation
→ surface zones
→ building/land-use generation
→ traffic graph
→ runtime export
→ Unreal import/build
```

Every generated element retains provenance/source IDs where licensing permits.

## 17. Validation gates

GEO-001 no orphan critical road nodes.
GEO-002 lane connectivity legal.
GEO-003 road elevation continuity.
GEO-004 bridge/tunnel vertical separation preserved.
GEO-005 no impossible road grade above configured class limit unless source/hero override.
GEO-006 surface query returns deterministic value at fixed point/time state.
GEO-007 world-to-physics unit conversion exact.
GEO-008 traffic route has no illegal direction reversal.
GEO-009 streaming does not delete active vehicle physics state.
GEO-010 HLOD/render state does not change physics surface result.

## 18. First world acceptance

A successful world prototype allows:
- spawn TA-P01;
- drive town→highway→rural→mountain;
- stream continuously;
- encounter traffic;
- experience rain/wet road;
- crash;
- continue with damage;
- stop/repair/recover;
- retain vehicle instance state.

## 19. Non-goals first region

Deferred:
- continent scale;
- every building interior;
- perfect cadastral fidelity;
- dense pedestrian city simulation;
- full seasons;
- large multiplayer population;
- procedural planet.

## 20. Promotion rule

World scale increases only after:
- build time;
- runtime streaming;
- traffic density;
- surface queries;
- vehicle physics CPU;
- storage size

are measured for the representative 20–25 km² region.
