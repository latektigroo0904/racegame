# Persistence Tooling

Engine-independent save-state contracts used to validate persistence semantics before Unreal/platform storage integration.

## Save package v1

Contains:
- schema version;
- profile revision;
- credits;
- career state;
- vehicle instances;
- transaction journal tail.

Vehicle instances reference authored content by stable namespaced IDs.

## Rules

- credits may not be negative;
- vehicle instance IDs are unique;
- odometer/engine hours are non-negative;
- installed component IDs are namespaced;
- tire/fluid/battery normalized condition values remain finite/in-range;
- journal transaction IDs are unique;
- schema/version fields are explicit.

## Usage

```bash
python Tools/Persistence/validate_save.py \
  Tools/Persistence/examples/minimal_save_v1.json
```
