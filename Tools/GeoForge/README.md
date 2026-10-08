# GeoForge Offline Tooling

Status: pre-Unreal normalized data contract

## Purpose

GeoForge converts external geographic source data into one Torque Atlas-owned normalized format before any Unreal import or gameplay interpretation.

The normalized layer is intentionally independent from:
- OpenStreetMap tag layout;
- PostGIS internal table layout;
- Unreal splines/meshes;
- traffic runtime classes.

## Directory

- `schema/geoforge_normalized_v1.schema.json` — structural JSON schema.
- `validate_geoforge.py` — semantic validator.
- `examples/minimal_network.json` — valid minimal fixture.
- `tests/test_validate_geoforge.py` — Python standard-library tests.

## Contract

Top-level normalized package:

```text
GeoForgePackage
├── schema_version
├── dataset_id
├── coordinate_system
├── nodes[]
├── segments[]
├── lanes[]
└── surface_zones[]
```

### Nodes

Stable geometric/topological anchors.

Required:
- id;
- x_m;
- y_m;
- z_m.

Coordinates are projected/local metric coordinates in metres, never latitude/longitude degrees in hot runtime data.

### Road segments

Required:
- id;
- start_node_id;
- end_node_id;
- road_class;
- speed_limit_mps;
- lane_ids[];
- centerline[].

A centerline has at least two 3D points.

### Lanes

Required:
- id;
- segment_id;
- direction;
- width_m;
- centerline[].

Direction values:
- `forward`;
- `backward`.

### Surface zones

Required:
- id;
- material_id;
- dry_friction_baseline;
- wet_friction_baseline;
- drainage01.

Optional:
- segment_ids[];
- roughness;
- macrotexture;
- temperature_c.

Surface friction fields are world/surface baselines. Tire physics remains authoritative for final forces.

## Validation layers

The JSON schema checks shape/type/range.

The Python semantic validator additionally checks:
- unique IDs;
- all node references resolve;
- all lane references resolve;
- lane-to-segment ownership is consistent;
- lane widths are positive;
- centerlines contain finite coordinates;
- segments are not self-connected;
- speed limits are finite/non-negative;
- surface-zone segment references resolve.

## Usage

```bash
python Tools/GeoForge/validate_geoforge.py Tools/GeoForge/examples/minimal_network.json
```

Exit code:
- 0 valid;
- 1 validation failure;
- 2 file/JSON error.

## Promotion path

1. freeze normalized v1 package semantics;
2. add OSM/PostGIS importer;
3. add DEM/elevation sampling;
4. add topology cleanup;
5. add lane/junction derivation;
6. produce normalized packages;
7. only after UE 5.8 baseline, add `TA_World` importer/runtime module.

No source-provider-specific tag may become gameplay truth without normalization.
