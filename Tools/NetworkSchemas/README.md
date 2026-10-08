# Multiplayer Session Schema Tooling

Engine-independent session compatibility contract.

## Session manifest v1

A server advertises:
- game build id;
- physics version;
- damage model version;
- authority mode;
- player limit;
- mod policy;
- allowed content-manifest SHA-256 hashes;
- allowed vehicle PhysicsConfigHash values.

Clients compare this before joining a physics session.

## Ranked rules

Ranked requires:
- server_authoritative = true;
- mod_policy != open;
- at least one allowed content manifest hash;
- at least one allowed vehicle physics hash.

## Hash encoding

PhysicsConfigHash is represented as exactly eight lowercase hexadecimal characters because the current canonical hash is uint32.

Content manifest hashes are SHA-256 lowercase hex strings.
