# Career, Economy, Garage & Maintenance v0.1

Updated: 2026-10-08
Status: implementation-ready design

## 1. Principle
Career progression should reward driving, ownership and mechanical decision-making without turning realistic vehicle systems into grind-only meters.

## 2. Persistent player profile
- player id;
- licenses;
- reputation by discipline;
- championships/events unlocked;
- sponsors/teams;
- garage slots/properties;
- account credits;
- records/achievements.

## 3. Persistent vehicle instance
Immutable definition reference plus:
- instance id / VIN-like id;
- odometer;
- engine hours;
- installed parts;
- fuel;
- battery SOC/health;
- tire sets and state;
- fluids;
- wear;
- damage;
- repairs;
- accident history;
- ownership history.

## 4. Economy units
Use configurable credits as gameplay currency.
No physics feature depends on currency.

Costs:
- vehicle purchase;
- used vehicle purchase;
- parts;
- labor;
- consumables;
- towing;
- entry fees;
- garage expansion.

Rewards:
- event payouts;
- sponsor objectives;
- championships;
- discovery/challenges;
- vehicle sales.

## 5. Used vehicle market
Listing contains:
- model/year/trim;
- mileage;
- visible condition;
- known service history;
- accident history visibility;
- asking price.

Hidden defects may exist only if career difficulty/content policy enables them; inspection mechanics reveal state probabilistically or through service records.

## 6. Maintenance
Service operations:
- oil;
- brake fluid;
- coolant;
- tire replacement;
- brake pads/rotors later;
- battery;
- alignment;
- suspension parts;
- drivetrain parts.

Maintenance changes real subsystem state.

## 7. Repair levels
- roadside recovery;
- mechanical-only repair;
- cosmetic repair;
- structural repair;
- full restoration;
- DIY where supported.

Structural repair may restore geometry but can retain history.

## 8. Wear
Mileage/time/load dependent:
- tires;
- brakes;
- clutch;
- fluids;
- battery;
- engine;
- bushings/bearings later.

No universal wear bar.

## 9. Licensing
Licenses gate competitive event classes:
- road;
- circuit;
- rally;
- drift;
- drag;
- offroad;
- advanced/pro.

Free roam remains open.

## 10. Reputation
Separate reputation per discipline prevents one activity from trivially unlocking all high-tier content.

## 11. Sponsors
Sponsor contract:
- duration;
- eligible disciplines;
- objectives;
- required vehicle/brand restrictions;
- fixed payout;
- performance bonuses;
- reputation consequences.

No pay-to-win dependency.

## 12. Team layer
Later:
- mechanics;
- crew;
- race engineer;
- transport/logistics.

Prototype can represent staff as service modifiers without RPG stat bloat.

## 13. Insurance
Optional realism layer:
- premium;
- deductible;
- coverage rules;
- claim history.

Must be configurable/off for players who do not want economic punishment.

## 14. Difficulty/economy profiles
Career difficulty may alter:
- payout multipliers;
- repair labor cost;
- inspection transparency;
- recovery cost.

It must not change vehicle physics unless separately selecting realism/assist profile.

## 15. Garage
Garage owns:
- vehicle storage;
- setup slots;
- spare parts;
- tire sets;
- maintenance tools;
- telemetry review;
- visual inspection.

## 16. Tuning
Part categories:
- ECU;
- intake/exhaust;
- forced induction;
- cooling;
- gearbox/final drive;
- differential;
- brakes;
- suspension;
- steering;
- wheels/tires;
- aero;
- weight/cage;
- EV/hybrid components later.

Every performance part compiles to physics calibration changes and therefore changes vehicle physics identity/hash.

## 17. Setup
Setup changes reversible parameters:
- tire pressure;
- alignment;
- springs/dampers where adjustable;
- anti-roll;
- brake bias;
- differential;
- aero;
- gear ratios.

Save named setups separately from installed-part inventory.

## 18. Event economy
Each event definition provides:
- entry cost;
- base payout;
- placement payout;
- clean-driving bonus;
- optional damage/consumable cost exposure.

Avoid rewards tied to exploit-prone raw collision/destruction counts unless intended.

## 19. Save transaction model
All economy mutations are atomic:
- purchase;
- sale;
- repair;
- reward;
- part install.

On multiplayer authoritative servers, server owns transaction truth.

## 20. Tests
CAREER-001 event reward applied once.
CAREER-002 purchase cannot create negative balance unless loans explicitly exist.
CAREER-003 part install consumes owned part/payment atomically.
CAREER-004 vehicle sale removes ownership.
CAREER-005 repair changes only selected subsystem states.
CAREER-006 physics-affecting part changes physics hash.
CAREER-007 cosmetic-only part does not.
CAREER-008 save/load preserves vehicle instance history.

## 21. MVP
- player profile;
- credits;
- licenses;
- reputation;
- vehicle purchase/sale;
- garage;
- maintenance;
- basic tuning;
- event rewards;
- used market v1.
