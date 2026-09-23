# Development Roadmap & MVP

## Phase sequence
1. Concept
2. Technical research
3. Prototype
4. Vehicle physics prototype
5. Damage physics prototype
6. Open-world prototype
7. Map-data pipeline
8. Vertical slice
9. Pre-alpha
10. Alpha
11. Beta
12. Release
13. Post-launch

## Current phase
Technical research / pre-prototype specification.

## Milestones

### P0 Foundation
Unreal project, modular C++ architecture, automated tests, logging, profiling, telemetry and coding standards.

### P1 Rigid Vehicle
Chassis, wheels, suspension, steering and input.

### P2 Tire Dynamics
Slip, combined grip, load sensitivity, temperature, pressure and wear.

### P3 Powertrain
Engine, turbo, clutch, six-speed gearbox, differential and driveline inertia.

### P4 Structural Damage
Nodes/constraints, yield, plastic deformation, breakage and telemetry.

### P5 Mechanical Consequences
Alignment damage, tires/wheels, radiator/coolant, oil, engine failure and steering consequences.

### P6 Proof of Physics
Drive → grip/weather → crash → damaged driving → thermal/mechanical failure.

### P7 World Prototype
Real-data pipeline plus approximately 20–25 km² playable test area.

### P8 Vertical Slice
Five polished vehicles, representative region, traffic, weather, events, damage and initial multiplayer.

## MVP target
- ~50–100 km²;
- 5–10 cars;
- one city;
- motorways;
- rural/mountain roads;
- AI traffic;
- weather;
- high-fidelity vehicle physics;
- first production damage system;
- basic tuning/maintenance;
- 8–16 player multiplayer.

## Team planning
Serious vertical slice: approximately 25–45 people depending on outsourcing.
Large full production: roughly 120–220 internal at peak plus outsourcing, subject to scope.

## Early budget bands
Planning estimates only:
- prototype: €0.5M–€2M;
- strong vertical slice: €3M–€8M;
- large AA: €25M–€60M;
- ambitious AAA: €70M–€150M+ development.

## Scope rule
Never scale world or vehicle count faster than tooling, physics and QA infrastructure can support.
