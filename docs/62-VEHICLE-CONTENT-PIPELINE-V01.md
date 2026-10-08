# Vehicle Content Production Pipeline v0.1

Updated: 2026-10-08
Status: implementation-ready design

## 1. Objective
Scale from prototype TA-P01 to dozens/hundreds of fictional vehicles without hand-authoring every physics field from scratch or losing provenance.

## 2. Layered content model

VehicleFamily
→ Platform/Architecture
→ Model
→ Trim
→ Variant
→ VehicleDefinition
→ VehicleInstance

Shared component families reduce duplication.

## 3. Vehicle family data
- brand;
- region;
- era;
- market segment;
- design language;
- drivetrain architecture options;
- platform family.

## 4. Platform
Defines reusable:
- wheelbase range;
- track range;
- hardpoint templates;
- suspension family;
- drivetrain packaging;
- crash structure archetype;
- electrical architecture;
- fuel/energy architecture.

## 5. Model/trim
Overrides:
- body mass/aero;
- engine;
- gearbox;
- differential;
- brakes;
- tires;
- tuning;
- equipment.

## 6. Component library
Reusable authored components:
- engines;
- motors;
- batteries;
- transmissions;
- differentials;
- brake packages;
- wheel/tire families;
- dampers/springs;
- aero parts;
- cooling packages.

Each component has:
- stable id;
- version;
- units;
- provenance;
- compatible interfaces.

## 7. Compile pipeline

Raw authoring
→ schema validation
→ component resolution
→ inheritance/overrides
→ unit validation
→ effective physics compilation
→ PhysicsConfigHash
→ asset export/import
→ regression selection
→ content manifest.

## 8. No silent inheritance
Every inherited value should be inspectable in tooling.
Debug output can show:
- source component;
- override chain;
- final value.

## 9. Mass/inertia
Do not derive all mass from one curb-mass scalar.
Author/derive:
- sprung mass;
- unsprung masses;
- major component locations;
- COM;
- inertia tensor.

Early content may use bounded estimation, with provenance marked accordingly.

## 10. Structure archetypes
Reusable templates:
- compact unibody;
- sports unibody;
- ladder-frame pickup;
- SUV;
- EV skateboard.

Templates provide topology/layout, not final identical crash strength.

## 11. Fictional brands
Initial production recommendation:
- 20–30 launch brands;
- ~80–120 launch models;
- expand toward 100 brands only when tools/QA support it.

Brand identity data:
- market positioning;
- naming convention;
- era/style;
- platform/component sharing;
- geography.

## 12. Art handoff
Physics definition and render asset connect through stable IDs and authored transforms.

Art may not silently change:
- wheelbase;
- wheel radius;
- hardpoints;
- collision dimensions

without physics recompile/review.

## 13. LOD
Vehicle art LOD independent of physics LOD.
Damage visual LOD independent from functional damage truth.

## 14. Tuning parts
Part definition contains:
- compatibility;
- installation slot;
- mass delta/location;
- parameter deltas or replacement component;
- cost;
- visual asset;
- legality/class effects.

No generic “+10 handling” production stat.

## 15. Quality gates
CONTENT-001 schema valid.
CONTENT-002 no unresolved component refs.
CONTENT-003 units valid.
CONTENT-004 COM inside plausible bounds.
CONTENT-005 mass closes.
CONTENT-006 wheel geometry coherent.
CONTENT-007 tire/wheel compatibility.
CONTENT-008 powertrain ratios valid.
CONTENT-009 physics hash stable for same effective config.
CONTENT-010 cosmetic-only change leaves physics hash unchanged.
CONTENT-011 required regression suite selected.
CONTENT-012 calibration provenance complete for production.

## 16. Automation
Batch content compiler should output:
- errors;
- warnings;
- physics hash;
- resolved dependency graph;
- calibration manifest;
- affected tests.

## 17. MVP content
Prototype:
- TA-P01 sports coupe;
- FWD compact;
- AWD rally hatch;
- sedan;
- SUV/pickup;
- EV.

This deliberately spans different physical architectures.

## 18. Non-goals
- fully procedural final car art;
- fake licensed brand similarity;
- manually copied real vehicle specifications without rights/provenance review.
