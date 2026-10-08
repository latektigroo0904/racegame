# Vehicle Audio & Haptics Architecture v0.1

Updated: 2026-10-08
Status: implementation-ready design

## 1. Principle
Audio/haptics consume physical telemetry/events; they do not drive vehicle physics.

## 2. Engine audio inputs
- RPM;
- load/torque;
- throttle;
- cylinder/engine archetype;
- turbo speed/boost;
- exhaust configuration;
- damage/misfire later.

## 3. Synthesis approach
Hybrid:
- authored recordings/sample layers;
- procedural pitch/load interpolation;
- transient events;
- optional granular/spectral approaches later.

Avoid one sample pitched across whole RPM range.

## 4. Drivetrain
Inputs:
- gear;
- shaft speeds;
- differential speed;
- clutch slip;
- gearbox load;
- driveline lash events.

## 5. Tire audio
Inputs:
- slip velocity;
- longitudinal/lateral force;
- surface material;
- wetness/water;
- loose material;
- load.

Separate:
- scrub/squeal;
- rolling texture;
- gravel impacts;
- water spray.

## 6. Suspension/body
- bump-stop impact;
- suspension velocity;
- curb/road impulse;
- structural creak;
- loose/detached part.

## 7. Crash
Event data:
- impulse;
- material pair;
- contact location;
- deformation/fracture;
- glass;
- detached parts.

Layered event, not only speed-based crash sound.

## 8. Mechanical failure
Distinct cues:
- tire puncture/flap;
- bearing damage;
- brake grinding;
- coolant leak/steam;
- engine knock/seizure;
- starter failure;
- electrical relay/click.

## 9. Environment
- wind from relative air speed;
- rain intensity;
- tunnel/reverb zones;
- traffic;
- surface spray.

## 10. Haptics
Gamepad:
- engine vibration;
- ABS pulse;
- curb/impact;
- tire slip;
- damage.

Wheel:
physical FFB remains separate from cosmetic vibration.
Optional texture overlays must remain bounded.

## 11. Interior/exterior
Acoustic mix depends on:
- camera;
- cabin isolation;
- windows;
- body damage;
- engine placement.

## 12. LOD
Distant vehicles:
- simplified engine band;
- limited tire/crash detail.

Nearby/player:
full mix.

## 13. Telemetry/event bus
Audio subscribes to immutable snapshot/event data.
No direct dependency from physics solver to audio engine.

## 14. Tests
AUDIO-001 zero RPM no running engine loop.
AUDIO-002 RPM/load monotonic layer selection.
AUDIO-003 tire surface material maps deterministically.
AUDIO-004 crash event magnitude bounded.
AUDIO-005 audio LOD switch does not alter physics.
AUDIO-006 FFB physical channel unaffected by cosmetic haptic layer.

## 15. MVP
- engine/exhaust;
- drivetrain;
- tire/surface;
- impacts;
- wind;
- rain;
- basic damage cues.
