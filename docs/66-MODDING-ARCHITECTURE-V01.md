# Modding Architecture & Safety v0.1

Updated: 2026-10-08
Status: implementation-ready design

## 1. Goal
Support PC modding without letting arbitrary mods corrupt ranked multiplayer, saves or core executable integrity.

## 2. Mod categories
- vehicle definitions;
- components/parts;
- liveries;
- audio;
- tracks/world regions;
- events;
- UI themes;
- scripting later.

## 3. Data-first
Prefer declarative versioned data formats for:
- vehicle components;
- tuning;
- event definitions;
- surfaces;
- traffic profiles.

Native code mods are a later separate trust tier.

## 4. Namespaces
Every mod has:
- mod id;
- version;
- author namespace;
- content ids prefixed/namespaced.

Prevent collisions with base IDs.

## 5. Dependency manifest
- required game schema/version;
- dependencies;
- conflicts;
- optional DLC/content dependencies;
- load order only where unavoidable.

## 6. Physics identity
Modded physics content changes PhysicsConfigHash.

Multiplayer session rules:
- ranked: approved/whitelisted configs only;
- unranked/private: server defines allowed mod set.

## 7. Save safety
Save references mod IDs/versions.
If missing:
- do not delete vehicle/content silently;
- mark unavailable;
- preserve serialized state for restoration where feasible.

## 8. Sandboxing
Data mods cannot execute arbitrary OS code.
Script mods, if introduced, use sandboxed API with:
- no arbitrary filesystem;
- no arbitrary network;
- bounded CPU/time;
- explicit capabilities.

## 9. World mods
Geo/world packs use normalized GeoForge/runtime format.
Validate topology and resource budgets before load.

## 10. Vehicle mods
Must pass:
- schema;
- units;
- mass/geometry sanity;
- component refs;
- hash;
- optional regression suite.

## 11. Distribution
Potential later:
- official mod portal/workshop integration;
- local sideload.

No platform chosen yet.

## 12. Security
Never trust mod-provided:
- paths;
- executable commands;
- network endpoints;
- serialized object classes.

Strict parsing and size limits.

## 13. Ranked integrity
Ranked server advertises exact:
- game build;
- allowed content manifest;
- physics hashes.

Client mismatch cannot join ranked physics session.

## 14. Tests
MOD-001 namespace collision rejected.
MOD-002 missing dependency reported.
MOD-003 invalid schema rejected.
MOD-004 physics mod changes hash.
MOD-005 cosmetic mod does not.
MOD-006 ranked mismatch rejected.
MOD-007 missing mod save preserved safely.
MOD-008 path traversal rejected.
MOD-009 oversized content rejected/bounded.

## 15. MVP
No public scripting.
Start with:
- liveries;
- event definitions;
- vehicle data in private/dev mode;
- world test packages later.
