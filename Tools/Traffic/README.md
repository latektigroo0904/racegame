# Traffic Offline Tooling

## Purpose

Build deterministic traffic lane connectivity from the GeoForge normalized road package before Unreal runtime integration.

This layer intentionally performs only topology derivation. It does not yet decide:
- traffic signals;
- turn restrictions from source-provider tags;
- priority;
- dynamic closures;
- agent behavior.

Those become explicit later inputs.

## Lane graph v1

Each lane node contains:
- lane id;
- start road-node id;
- end road-node id;
- successor lane ids;
- predecessor lane ids.

Direction is derived from the parent GeoForge segment:

- `forward`: segment start → segment end;
- `backward`: segment end → segment start.

Immediate same-segment U-turns are excluded by default.

## Usage

```bash
python Tools/Traffic/build_lane_graph.py \
  Tools/GeoForge/examples/minimal_network.json \
  /tmp/lane_graph.json
```

## Promotion path

1. topology-only adjacency;
2. turn restriction input;
3. junction movement/conflict zones;
4. signal/controller references;
5. route costs;
6. runtime traffic graph import.
