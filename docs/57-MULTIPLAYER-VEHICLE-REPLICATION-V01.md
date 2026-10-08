# Multiplayer Vehicle State & Damage Replication v0.1

Updated: 2026-10-08
Status: implementation-ready networking architecture

## 1. Principle

Server authority owns competitive/persistent truth.

Do not replicate every structural node every frame.

## 2. Authoritative state

Server authoritative:
- vehicle transform/velocities;
- driver inputs or accepted control stream;
- engine/powertrain state needed for reconciliation;
- wheel rotational states as required;
- critical tire states;
- functional damage;
- persistent instance changes;
- race timing/economy;
- collision/impact decisions.

## 3. Local prediction

Owning client predicts:
- controls;
- vehicle fixed-step simulation;
- immediate visual response.

Server sends state snapshots/reconciliation.

Correction must avoid teleporting minor differences where resimulation can recover.

## 4. Physics configuration identity

Connection/session validates:
- vehicle definition ID;
- PhysicsConfigHash;
- DamageModelVersion;
- approved part/loadout identity.

Ranked session rejects incompatible physics configs.

## 5. Snapshot

Conceptual:

```
FTANetVehicleState
{
    SimulationTick
    Position
    Orientation
    LinearVelocity
    AngularVelocity

    EngineState
    Gear
    CriticalWheelState[4]

    CriticalFunctionalDamage
    StructureChecksum
}
```

Exact compression is later profiling work.

## 6. Input replication

Client sends compact timestamped input:
- throttle;
- brake;
- steering;
- clutch;
- gear;
- assist-relevant commands.

Server simulates/validates.

## 7. Collision event

Important collision replication:

```
ImpactId
ServerTick
VehicleIds
ContactPoint
ContactNormal
Impulse
AngularImpulse
RelativeVelocity
DamageSeed / deterministic event identity
```

Server owns authoritative functional damage consequence.

## 8. Structural damage

Three layers:

### Authoritative reduced structure
Server simulates enough structure/damage to determine:
- component failure;
- alignment/mount displacement;
- detach-critical state;
- drivability.

### Client visual structure
Client may run denser deformation locally from authoritative impacts.

### Structural keyframe
Occasional correction:
- major node/zone transforms;
- broken connection IDs/bitsets;
- checksum.

## 9. Determinism

Bit-identical cross-platform determinism is not required initially.

Required:
- stable fixed-step semantics;
- bounded prediction divergence;
- reproducible server result for same server build/config;
- state reconciliation.

## 10. Damage authority

Client cannot authoritatively claim:
- repaired vehicle;
- unbroken component;
- restored tire;
- changed fuel/battery;
- altered installed part.

Server validates persistent changes.

## 11. Vehicle LOD networking

Remote vehicles may receive different snapshot frequency/detail based on:
- distance;
- relevance;
- race proximity;
- collision likelihood.

Never drop data required for imminent collision authority.

## 12. Traffic

AI traffic authority may be server-side.

Far traffic can be agent-level.

Near player/collision-interest traffic upgrades to physical representation.

## 13. Bandwidth priorities

High:
- local race opponents;
- imminent contacts;
- critical state changes.

Medium:
- nearby traffic.

Low:
- distant visual vehicles.

Structural visual detail is lower priority than transform/critical damage.

## 14. Reconciliation events

Explicit reliable events:
- component fracture;
- puncture;
- engine stall/seizure;
- vehicle immobilized;
- part detach if gameplay relevant;
- repair;
- refuel/recharge.

## 15. Anti-cheat boundary

Server checks:
- impossible controls/input rate;
- physics hash mismatch;
- impossible acceleration/velocity divergence;
- inventory/part ownership;
- repair/economy transactions.

Do not rely on client hidden values.

## 16. Replay

Authoritative replay can store:
- initial state/config hashes;
- input stream;
- server event stream;
- periodic keyframes.

This is preferable to storing all high-frequency node states.

## 17. Tests

NET-001 same config accepted.
NET-002 physics-hash mismatch rejected.
NET-003 input sequence reproduces bounded server/client path.
NET-004 delayed snapshot reconciles.
NET-005 collision event creates matching critical functional damage.
NET-006 visual structural divergence corrected by keyframe.
NET-007 lost noncritical packet does not corrupt persistent state.
NET-008 reliable critical failure arrives exactly once semantically.
NET-009 LOD transition preserves vehicle identity/state.
NET-010 replay reconstructs race-critical state within tolerance.

## 18. MVP target

Initial multiplayer:
- 8–16 players;
- authoritative server;
- limited dense structure replication;
- persistent functional damage;
- race/free-roam sessions.

Scale only after profiling.

## 19. Non-goals v0.1

Deferred:
- 64-player dense collision grid;
- rollback of every structural node;
- MMO persistent shard;
- peer-to-peer authority;
- deterministic lockstep across all hardware.

## 20. Security rule

Server trusts intent, not client physics outcome.
