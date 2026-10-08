#!/usr/bin/env python3
"""Validate Torque Atlas vehicle build / installed-part manifests."""

from __future__ import annotations

import hashlib
import json
import math
import re
import sys
from pathlib import Path
from typing import Any


ID_RE = re.compile(r"^[a-z0-9._-]+:[a-zA-Z0-9._-]+$")


def canonical_hash(value: Any) -> str:
    payload = json.dumps(
        value,
        sort_keys=True,
        separators=(",", ":"),
        ensure_ascii=False,
        allow_nan=False,
    ).encode("utf-8")
    return hashlib.sha256(payload).hexdigest()


def validate_vehicle_build(v: Any) -> list[str]:
    e: list[str] = []

    if not isinstance(v, dict):
        return ["root: expected object"]

    if v.get("schema_version") != 1:
        e.append("schema_version: expected 1")

    for field in ("build_id", "vehicle_definition_id"):
        value = v.get(field)
        if not isinstance(value, str) or not ID_RE.match(value):
            e.append(f"{field}: expected namespaced id")

    base_mass = v.get("base_mass_kg")
    if not isinstance(base_mass, (int, float)) or not math.isfinite(float(base_mass)) or base_mass <= 0:
        e.append("base_mass_kg: expected finite > 0")

    slots = v.get("slots", [])
    parts = v.get("installed_parts", [])

    if not isinstance(slots, list):
        e.append("slots: expected array")
        slots = []

    if not isinstance(parts, list):
        e.append("installed_parts: expected array")
        parts = []

    slot_by_id: dict[str, dict[str, Any]] = {}

    for index, slot in enumerate(slots):
        if not isinstance(slot, dict):
            e.append(f"slots[{index}]: expected object")
            continue

        slot_id = slot.get("slot_id")
        if not isinstance(slot_id, str) or not slot_id:
            e.append(f"slots[{index}].slot_id: invalid")
            continue

        if slot_id in slot_by_id:
            e.append(f"slots[{index}].slot_id: duplicate '{slot_id}'")
            continue

        slot_by_id[slot_id] = slot

        allowed = slot.get("allowed_component_types")
        if not isinstance(allowed, list) or not allowed or any(not isinstance(x, str) or not x for x in allowed):
            e.append(f"slots[{index}].allowed_component_types: invalid")

        if not isinstance(slot.get("required"), bool):
            e.append(f"slots[{index}].required: expected boolean")

    occupied_slots: set[str] = set()
    part_ids: set[str] = set()
    total_mass_delta = 0.0

    for index, part in enumerate(parts):
        if not isinstance(part, dict):
            e.append(f"installed_parts[{index}]: expected object")
            continue

        part_id = part.get("part_id")
        if not isinstance(part_id, str) or not ID_RE.match(part_id):
            e.append(f"installed_parts[{index}].part_id: expected namespaced id")
        elif part_id in part_ids:
            e.append(f"installed_parts[{index}].part_id: duplicate '{part_id}'")
        else:
            part_ids.add(part_id)

        slot_id = part.get("slot_id")
        if slot_id not in slot_by_id:
            e.append(f"installed_parts[{index}].slot_id: unknown '{slot_id}'")
            continue

        if slot_id in occupied_slots:
            e.append(f"installed_parts[{index}].slot_id: slot '{slot_id}' already occupied")
        occupied_slots.add(slot_id)

        component_type = part.get("component_type")
        allowed = slot_by_id[slot_id].get("allowed_component_types", [])
        if component_type not in allowed:
            e.append(
                f"installed_parts[{index}].component_type: '{component_type}' "
                f"not allowed in slot '{slot_id}'"
            )

        mass_delta = part.get("mass_delta_kg", 0.0)
        if not isinstance(mass_delta, (int, float)) or not math.isfinite(float(mass_delta)):
            e.append(f"installed_parts[{index}].mass_delta_kg: invalid")
        else:
            total_mass_delta += float(mass_delta)

        if not isinstance(part.get("physics_affecting"), bool):
            e.append(f"installed_parts[{index}].physics_affecting: expected boolean")

    for slot_id, slot in slot_by_id.items():
        if slot.get("required") is True and slot_id not in occupied_slots:
            e.append(f"required slot '{slot_id}' is empty")

    if isinstance(base_mass, (int, float)) and math.isfinite(float(base_mass)):
        if float(base_mass) + total_mass_delta <= 0.0:
            e.append("effective mass must remain positive")

    return e


def main(argv: list[str]) -> int:
    if len(argv) != 2:
        return 2

    path = Path(argv[1])

    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        print(exc, file=sys.stderr)
        return 2

    errors = validate_vehicle_build(value)

    if errors:
        print("\n".join(errors), file=sys.stderr)
        return 1

    print(canonical_hash(value))
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
