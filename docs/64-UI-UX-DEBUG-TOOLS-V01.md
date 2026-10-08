# UI, UX, Telemetry & Debug Tools v0.1

Updated: 2026-10-08
Status: implementation-ready design

## 1. UI principle
Player UI exposes meaningful vehicle state without requiring engineering knowledge; debug UI exposes the full simulation.

## 2. Player HUD
Configurable:
- speed;
- RPM;
- gear;
- fuel/charge;
- critical warnings;
- navigation;
- event timing;
- assists state;
- damage warnings.

Simulation/Hardcore profiles may reduce UI aids.

## 3. Warning hierarchy
Critical:
- oil pressure;
- coolant overheat;
- brake failure;
- puncture;
- electrical critical;
- engine seizure.

Advisory:
- tire temperature;
- brake temperature;
- low fuel;
- service due.

Warnings derive from real subsystem states.

## 4. Garage UI
Tabs:
- overview;
- structure/body;
- suspension/alignment;
- tires/wheels;
- brakes;
- powertrain;
- cooling/fluids;
- electrical;
- aero;
- maintenance;
- history;
- setup.

## 5. Diagnostic overlays
Developer:
- tire forces/slips;
- contact normals/loads;
- suspension travel/hardpoints;
- aero force vectors/application points;
- drivetrain torque;
- brake pressure/temp;
- fluid flows/leaks;
- electrical voltage/current;
- structure nodes/constraints;
- damage routes;
- AI lane/route;
- streaming cells.

## 6. Telemetry session browser
Filter by:
- scenario;
- vehicle;
- physics hash;
- commit;
- engine build;
- time.

Compare two runs:
- overlaid channels;
- delta metrics;
- acceptance failures.

## 7. Setup UI
Show physical parameter + units.
Explain impact qualitatively but do not hide actual value.

## 8. Damage visualization
Modes:
- visual;
- mechanical;
- structural;
- thermal;
- fluid;
- electrical.

No single HP bar.

## 9. Accessibility
- scalable text;
- colorblind-safe signals;
- remappable controls;
- subtitles;
- reduced camera shake;
- assist presets;
- simplified garage explanations.

Accessibility should not require alternate physics.

## 10. Input configuration
- keyboard;
- gamepad;
- wheel;
- pedals;
- shifter;
- handbrake.

Calibration:
- deadzone;
- saturation;
- linearity;
- inversion;
- device mapping.

## 11. Wheel/FFB UI
Expose:
- physical torque meter;
- device max torque;
- user gain;
- clipping indicator;
- smoothing.

Warn when clipping destroys force detail.

## 12. Developer console
Commands:
- spawn vehicle;
- load scenario;
- set surface/weather;
- damage component;
- puncture tire;
- set fluid state;
- set battery SOC;
- export telemetry;
- run regression.

## 13. QA capture
One-click issue bundle:
- screenshot;
- short replay/input trace;
- physics hash;
- vehicle id;
- map cell;
- telemetry slice;
- logs;
- build id.

## 14. Tests
UI-001 warnings match subsystem state.
UI-002 units consistent.
UI-003 no stale telemetry after vehicle swap.
UI-004 debug overlay cannot mutate physics unless explicit command.
UI-005 FFB clipping indicator correct.
UI-006 save/load UI settings.
UI-007 inaccessible color-only critical warning prohibited.

## 15. MVP
- HUD;
- garage overview;
- tuning/setup;
- diagnostics screen;
- telemetry export;
- core debug overlays.
