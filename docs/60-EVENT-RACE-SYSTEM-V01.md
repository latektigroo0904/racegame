# Event & Race System v0.1

Updated: 2026-10-08
Status: implementation-ready design

## 1. Goal
One event framework supports circuit, point-to-point, street, rally, drift, drag, offroad, time trial and free-form challenges.

## 2. Event definition
Data-driven:
- event id/version;
- discipline;
- map/route;
- eligible vehicles;
- class/performance rules;
- assists/rules profile;
- weather/time;
- grid/start;
- laps/checkpoints;
- penalties;
- rewards;
- AI field;
- multiplayer settings.

## 3. Route primitives
- start gate;
- finish gate;
- checkpoint;
- sector;
- lap line;
- joker/alternate route later;
- speed trap;
- drift zone.

## 4. Timing
Use authoritative simulation/race clock.
Record:
- total;
- laps;
- sectors;
- checkpoint splits;
- penalties;
- best valid lap.

Rendering frame rate never determines race time.

## 5. Validation
Checkpoint order prevents shortcuts.
Track-boundary rules configurable:
- strict circuit;
- rally corridor;
- open-road checkpoint route.

## 6. Start types
- standing;
- rolling;
- staggered;
- individual time trial;
- drag tree;
- rally countdown.

## 7. Circuit
Supports:
- laps;
- grid;
- qualifying;
- pit lane later;
- flags/safety car later.

## 8. Rally
Supports:
- point-to-point;
- codriver notes later;
- staggered timing;
- service intervals;
- accumulated damage.

## 9. Drift
Scoring inputs:
- angle;
- line/zone;
- speed;
- transitions;
- continuity;
- proximity where authored.

Do not score by arbitrary handbrake usage.

## 10. Drag
- reaction time;
- false start;
- elapsed time;
- trap speed;
- lane boundaries;
- pre-stage/stage later.

## 11. Offroad
Route/checkpoint with looser corridor and terrain-class eligibility.

## 12. Street/open-road
Traffic behavior policy is event-specific:
- closed road;
- partial closure;
- live traffic.

Live traffic competitive modes require clear fairness/safety rules.

## 13. Vehicle eligibility
Rules may inspect:
- model/brand;
- drive type;
- power/weight;
- tire category;
- installed parts;
- performance class;
- damage condition.

No client-only eligibility in authoritative multiplayer.

## 14. Performance class
Prefer transparent calculated class from:
- power/weight;
- tire capability;
- aero;
- drivetrain;
- braking;
- mass.

Final formula requires executable calibration and can remain content-tunable.

## 15. Penalties
- false start;
- missed checkpoint;
- cut;
- collision conduct in ranked modes;
- wrong route;
- pit/speed-zone violations later.

## 16. AI events
AI receives event intent/rules but uses same physical vehicle capability.

Difficulty changes:
- driving policy;
- risk;
- consistency;
- strategic behavior.

Do not give hidden grip/power bonuses by default.

## 17. Multiplayer
Server owns:
- countdown;
- timing;
- checkpoint validity;
- penalties;
- final result.

Client may predict UI only.

## 18. Replays
Store authoritative input/events/keyframes sufficient to reconstruct race-critical state.

## 19. Tests
EVENT-001 checkpoint order.
EVENT-002 missed checkpoint invalidates/penalizes.
EVENT-003 lap increments exactly once.
EVENT-004 timing independent of render FPS.
EVENT-005 false start detection.
EVENT-006 eligibility deterministic.
EVENT-007 reward once.
EVENT-008 multiplayer result server-authoritative.
EVENT-009 replay reconstructs timing.
EVENT-010 event weather/profile version recorded.

## 20. MVP
- circuit;
- point-to-point;
- time trial;
- rally;
- drag;
- basic drift;
- free-roam challenges.
