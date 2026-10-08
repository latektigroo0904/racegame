# Traffic Simulation Architecture v0.1

Updated: 2026-10-08
Status: implementation-ready design

## 1. Goal
Create believable open-world traffic that obeys road topology, traffic rules and vehicle limits while scaling from nearby physical actors to distant statistical agents.

## 2. Ownership
Traffic system owns:
- route/intent;
- lane choice;
- desired speed;
- gap acceptance;
- intersection behavior;
- spawn/despawn policy.

Vehicle physics owns:
- acceleration/braking capability;
- tire/surface limits;
- collision response;
- damage.

Traffic AI never teleports speed to satisfy behavior except explicit far-agent LOD.

## 3. Driver profile
Per agent:
- patience [0..1];
- awareness [0..1];
- aggression [0..1];
- skill [0..1];
- risk tolerance [0..1];
- lawfulness [0..1].

Profiles affect policy thresholds, not physical vehicle constants.

## 4. Agent state
- route id;
- current lane;
- longitudinal lane coordinate;
- desired destination;
- desired speed;
- current maneuver;
- local hazard state;
- reaction delay;
- profile;
- vehicle capability summary.

## 5. Longitudinal control
Use a bounded car-following controller similar in spirit to IDM:
- desired speed;
- time headway;
- minimum gap;
- relative speed;
- comfortable acceleration/deceleration.

Controller outputs desired longitudinal acceleration or throttle/brake request.

Clamp by vehicle capability.

## 6. Lane change
Use explicit incentive + safety:
- slower leader;
- route requirement;
- merge obligation;
- overtaking;
- returning to legal lane;
- emergency avoidance.

Safety checks:
- target-lane rear gap;
- target-lane front gap;
- predicted braking requirement;
- rule legality.

## 7. Junction model
Junction runtime exposes:
- conflict zones;
- priority;
- stop/yield;
- signal phase;
- turn legality;
- pedestrian crossing reservation later.

Agent requests/holds conflict-zone reservation.

## 8. Signals
Signal controller supports:
- fixed cycle;
- actuated;
- scripted event mode;
- damaged/offline state later.

Traffic agents consume legal movement permissions.

## 9. Merging
Highway merges use:
- acceleration-lane target gap;
- cooperative gap creation probability based on profile;
- forced merge only when risk threshold allows.

## 10. Incidents
Traffic reacts to:
- stopped vehicle;
- crash debris;
- lane closure;
- emergency vehicle;
- weather hazard;
- police/roadwork zone later.

Reaction quality depends on awareness/skill.

## 11. LOD
TLOD0: physical nearby collision-relevant vehicle.
TLOD1: simplified vehicle dynamics.
TLOD2: kinematic lane follower with acceleration constraints.
TLOD3: graph agent with time-position state.
TLOD4: aggregate traffic density/flow.

Promotion/demotion preserves identity, route, approximate speed and critical damage.

## 12. Spawn model
Spawn inputs:
- road class;
- time of day;
- district;
- traffic demand profile;
- weather;
- event closures;
- density budget.

No spawn inside player sight/collision risk unless explicitly allowed by debug tooling.

## 13. Traffic demand
Represent OD demand matrices or weighted district pairs.

Prototype may use procedural demand curves:
- morning;
- midday;
- evening;
- night;
- weekend;
- event surge.

## 14. Vehicle selection
Traffic fleet generator uses:
- region;
- income/district weighting;
- vehicle age distribution;
- weather suitability;
- road type;
- time period/theme.

## 15. Physical upgrade
When agent enters physical radius:
- instantiate vehicle;
- seed exact transform/velocity;
- assign deterministic traffic vehicle definition;
- initialize simplified wear/damage;
- settle contact before collision authority.

## 16. Collision behavior
Nearby traffic uses physical collision.

AI reaction after collision:
- stop;
- hazard lights;
- pull over if drivable;
- reroute;
- emergency response request later.

## 17. Determinism
Traffic policy may use RNG, but random decisions use seeded streams:
- world seed;
- agent stable id;
- decision epoch.

This supports replays/regression.

## 18. Telemetry
- active agents by LOD;
- average speed by road class;
- queue length;
- lane-change attempts/success;
- junction wait;
- collisions per vehicle-km;
- emergency stops;
- CPU time per layer.

## 19. Tests
TRAFFIC-001 same seed same decisions.
TRAFFIC-002 no illegal one-way routing.
TRAFFIC-003 following controller preserves minimum gap under normal bounds.
TRAFFIC-004 lane change rejects unsafe rear gap.
TRAFFIC-005 stop sign requires stop.
TRAFFIC-006 signal red blocks movement.
TRAFFIC-007 lane closure reroutes.
TRAFFIC-008 LOD transition preserves route/speed.
TRAFFIC-009 physical capability bounds AI request.
TRAFFIC-010 spawn never occurs in forbidden player bubble.

## 20. Acceptance
First traffic prototype:
- 100s–1000s logical agents depending LOD;
- dozens of nearby physical vehicles after profiling;
- believable highway/city flow;
- no routine illegal routing;
- collisions emerge physically rather than scripted avoidance teleport.
