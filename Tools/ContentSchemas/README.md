# Torque Atlas Content Schemas

Engine-independent authoring contracts used before Unreal asset compilation.

## v1 manifest

A package contains:
- package identity;
- namespace;
- reusable vehicle components;
- event definitions;
- traffic driver profiles.

All IDs are namespaced:

```text
<namespace>:<local-id>
```

Base-game example:

```text
ta.base:engine-i4t-20
```

## Physics-affecting content

Any component marked `physics_affecting: true` must provide:
- provenance_class: P0..P4;
- confidence: low/medium/high;
- units_note/source note where appropriate.

This metadata is authoring/tooling information. Hot runtime structs remain compact.

## Usage

```bash
python Tools/ContentSchemas/validate_content_manifest.py \
  Tools/ContentSchemas/examples/base_minimal_manifest.json
```

Exit:
- 0 valid;
- 1 semantic invalid;
- 2 file/JSON error.
