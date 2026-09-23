# Torque Atlas / Racegame — Project Canon

Status: active design & technical R&D  
Last updated: 2026-09-23  
Repository: `latektigroo0904/racegame`

## Vision
A realistic open-world driving and racing simulator built around four pillars:
1. high-fidelity vehicle dynamics;
2. functional structural/mechanical damage;
3. large worlds generated from real geographic data;
4. free driving, racing, ownership, maintenance and tuning in one consistent simulation.

Working product name: **Torque Atlas**. Repository name remains `racegame` until naming/IP checks are complete.

## Non-negotiable design rules
- Driving quality comes before world size.
- Damage must affect function, not only visuals.
- Tuning changes actual physics.
- The same simulation foundation powers free roam, circuit, rally, drift, drag and crashes.
- Scale is generated procedurally; quality is curated manually.
- No pay-to-win.
- Real geographic data must be used legally and with correct attribution/licensing.
- No direct copying of protected BeamNG technology; only the design goal of deep functional deformation is used as inspiration.

## Target experience
Players can cruise, race, rally, drift, drag, road-trip, test crashes, buy/maintain/repair/tune vehicles and join multiplayer activities.

## Technical direction
- Engine: Unreal Engine 5.x.
- Core simulation: custom C++ modules.
- Unreal handles rendering, asset pipeline, world partition, UI, audio and high-level framework.
- Chaos Vehicles may be used for comparison/prototyping/low-fidelity AI, not as final player-vehicle solver.
- Custom systems: vehicle dynamics, tire solver, structural damage, component damage, GeoForge map pipeline.
- World tech: World Partition, HLOD, Data Layers, Large World Coordinates where appropriate.
- Real map sources: OpenStreetMap plus legally usable elevation/land-use datasets.
- Geospatial tooling: PostgreSQL/PostGIS + custom conversion tools.

## Scale strategy
Hybrid map scale:
- hero cities/circuits/mountain passes: near 1:1;
- secondary areas: ~1:2;
- long connective stretches: ~1:3 to 1:6.
Recognizability is preserved while dead travel time and production cost are compressed.

## Long-term content ceiling
- Maximum 100 original fictional vehicle brands.
- Initial target: ~20–30 brands.
- Launch target concept: ~80–120 vehicle models, depending on budget.
- First prototype: 1 car.
- MVP: 5–10 cars and ~50–100 km².
- First vertical slice should be much smaller and focused on proving physics.

## Current technical order
1. Vehicle physics foundation.
2. Tire solver.
3. Surface/wetness/aquaplaning.
4. Powertrain.
5. Structural damage.
6. Mechanical consequence graph.
7. GeoForge.
8. Traffic.
9. Streaming.
10. Multiplayer.
11. Career/economy/content scale.

## Canon prototype vehicle
Working ID: **TA-P01**
- sports coupe;
- 1,420 kg;
- RWD;
- 2.0 L turbo inline-four;
- ~220 kW;
- ~410 Nm;
- six-speed manual;
- 2.62 m wheelbase;
- ~53/47 front/rear mass balance;
- double wishbone front;
- multi-link rear.

## Proof-of-Physics acceptance scenario
1. Start vehicle.
2. Tires warm with use.
3. Rain reduces grip.
4. Player impacts front-left structure.
5. Structure deforms.
6. Alignment changes.
7. Radiator is damaged.
8. Coolant leaks.
9. Steering pulls.
10. Engine temperature rises.
11. ECU reduces output.
12. Engine may eventually fail.

If this chain works convincingly, the project has proven its central technical premise.

## Documentation rule
Every meaningful design or technical decision must be recorded in this repository. The latest approved document is the canonical source; chat text is not treated as the only copy.
