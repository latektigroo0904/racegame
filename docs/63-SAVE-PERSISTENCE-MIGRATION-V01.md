# Save, Persistence & Migration Architecture v0.1

Updated: 2026-10-08
Status: implementation-ready design

## 1. Principle
Persistent player/vehicle/economy state must survive game updates without storing transient solver internals that can be recomputed safely.

## 2. Save domains
- player profile;
- career;
- economy;
- garage;
- vehicle instances;
- setups;
- world discoveries;
- settings;
- multiplayer-authoritative account state where applicable.

## 3. Vehicle instance save
Store:
- definition id/version;
- installed part ids/versions;
- odometer;
- engine hours;
- consumables;
- fuel/charge;
- battery health;
- tire sets/state;
- fluid quantities/condition;
- wear;
- functional damage;
- structural repair state/history;
- service history;
- accident history.

Avoid storing every hot transient:
- tire force;
- suspension force;
- current solver iteration;
- temporary contact state.

## 4. Schema versioning
Every save root has:
- SaveSchemaVersion;
- game build/version;
- timestamp;
- migration history.

Each major domain may have its own version.

## 5. Migration
Migrations are:
- ordered;
- deterministic;
- idempotence-tested where possible;
- backup-first.

OldVersion → ... → CurrentVersion.

Never rewrite user save without preserving recovery copy.

## 6. Definition changes
If vehicle definition updates:
- instance retains installed components/condition;
- effective physics recompiles against new compatible definition;
- incompatible parts migrate or are flagged;
- legacy definition snapshot/reference may be retained for historical integrity.

## 7. Physics version change
Major physics-model changes may invalidate old setup equivalence.
Save stores:
- prior PhysicsVersion;
- previous physics hash for history;
- current recompiled hash after migration.

## 8. Transactions
Economy/purchase/repair operations use atomic transaction semantics.

Journal:
- transaction id;
- type;
- amount/items;
- precondition;
- resulting state hash/version.

## 9. Crash safety
Local save:
- write temp;
- fsync/platform equivalent where feasible;
- atomic replace;
- keep previous generation.

## 10. Cloud conflict
Compare:
- revision;
- timestamp;
- transaction journal;
- vehicle instance revisions.

Do not merge currency by naive addition.

## 11. Multiplayer
Server-authoritative persistent state should store canonical database record.
Client cache is not source of truth.

## 12. Replay vs save
Replay records simulation reconstruction.
Save records persistent gameplay state.
Do not mix them.

## 13. Privacy
Do not store unnecessary personal identifiers in game saves/telemetry.
Stable gameplay IDs should be pseudonymous/internal.

## 14. Tests
SAVE-001 roundtrip equality.
SAVE-002 old schema migrates.
SAVE-003 migration backup created.
SAVE-004 interrupted write preserves previous save.
SAVE-005 transaction applies once.
SAVE-006 duplicate transaction rejected/idempotent.
SAVE-007 cosmetic change migration preserves physics state.
SAVE-008 definition part incompatibility reports cleanly.
SAVE-009 cloud conflict does not duplicate money/cars.
SAVE-010 corrupted section fails safely.

## 15. MVP
- local profile;
- career/economy;
- garage/vehicle instances;
- settings;
- three rolling backups;
- explicit schema migration framework.
