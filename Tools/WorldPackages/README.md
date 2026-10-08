# World Package Tooling

A world package manifest binds normalized GeoForge data, lane graph data, surface data and streaming chunks by cryptographic identity.

It deliberately does not contain Unreal asset paths as gameplay truth.

## v1 identity

- world package id
- dataset id
- GeoForge normalized SHA-256
- lane graph SHA-256
- surface package SHA-256
- chunk list with metric bounds and content hash

This lets:
- builds record exact world input;
- multiplayer compare world packages;
- QA issue bundles identify streamed content;
- chunk regeneration remain deterministic/auditable.
