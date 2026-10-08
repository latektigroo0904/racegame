#!/usr/bin/env python3
"""Semantic validator for Torque Atlas content manifests."""

from __future__ import annotations

import json
import math
import re
import sys
from pathlib import Path
from typing import Any


ID_NAMESPACE_RE = re.compile(r"^[a-z0-9._-]+:[a-zA-Z0-9._-]+$")
PROVENANCE = {"P0", "P1", "P2", "P3", "P4"}
CONFIDENCE = {"low", "medium", "high"}
COMPONENT_TYPES = {
    "engine",
    "motor",
    "battery",
    "transmission",
    "differential",
    "brake",
    "tire",
    "suspension",
    "steering",
    "aero",
    "cooling",
    "electrical",
    "structure",
    "cosmetic",
}
DISCIPLINES = {
    "circuit",
    "point_to_point",
    "rally",
    "drift",
    "drag",
    "offroad",
    "time_trial",
    "challenge",
}


def _finite_01(value: Any) -> bool:
    return (
        isinstance(value, (int, float))
        and math.isfinite(float(value))
        and 0.0 <= float(value) <= 1.0
    )


def _validate_namespaced_id(
    value: Any,
    namespace: str,
    path: str,
    errors: list[str],
) -> None:
    if not isinstance(value, str) or not ID_NAMESPACE_RE.match(value):
        errors.append(f"{path}: expected namespaced id")
        return

    if not value.startswith(namespace + ":"):
        errors.append(
            f"{path}: id '{value}' does not belong to namespace '{namespace}'"
        )


def validate_manifest(manifest: Any) -> list[str]:
    errors: list[str] = []

    if not isinstance(manifest, dict):
        return ["root: expected object"]

    if manifest.get("schema_version") != 1:
        errors.append("schema_version: expected 1")

    namespace = manifest.get("namespace")
    if not isinstance(namespace, str) or not namespace:
        errors.append("namespace: expected non-empty string")
        namespace = ""

    components = manifest.get("components", [])
    events = manifest.get("events", [])
    profiles = manifest.get("driver_profiles", [])

    for name, value in (
        ("components", components),
        ("events", events),
        ("driver_profiles", profiles),
    ):
        if not isinstance(value, list):
            errors.append(f"{name}: expected array")

    if errors:
        return errors

    all_ids: set[str] = set()

    def register_id(value: Any, path: str) -> None:
        _validate_namespaced_id(value, namespace, path, errors)

        if isinstance(value, str):
            if value in all_ids:
                errors.append(f"{path}: duplicate id '{value}'")
            all_ids.add(value)

    for index, component in enumerate(components):
        if not isinstance(component, dict):
            errors.append(f"components[{index}]: expected object")
            continue

        register_id(component.get("id"), f"components[{index}].id")

        component_type = component.get("component_type")
        if component_type not in COMPONENT_TYPES:
            errors.append(f"components[{index}].component_type: invalid")

        version = component.get("version")
        if not isinstance(version, int) or version < 1:
            errors.append(f"components[{index}].version: expected integer >= 1")

        physics_affecting = component.get("physics_affecting")
        if not isinstance(physics_affecting, bool):
            errors.append(
                f"components[{index}].physics_affecting: expected boolean"
            )
            continue

        calibration = component.get("calibration")

        if physics_affecting:
            if not isinstance(calibration, dict):
                errors.append(
                    f"components[{index}].calibration: required for physics content"
                )
                continue

            if calibration.get("provenance_class") not in PROVENANCE:
                errors.append(
                    f"components[{index}].calibration.provenance_class: invalid"
                )

            if calibration.get("confidence") not in CONFIDENCE:
                errors.append(
                    f"components[{index}].calibration.confidence: invalid"
                )

        if component_type == "cosmetic" and physics_affecting:
            errors.append(
                f"components[{index}]: cosmetic component cannot be "
                "physics_affecting"
            )

    for index, event in enumerate(events):
        if not isinstance(event, dict):
            errors.append(f"events[{index}]: expected object")
            continue

        register_id(event.get("id"), f"events[{index}].id")

        if event.get("discipline") not in DISCIPLINES:
            errors.append(f"events[{index}].discipline: invalid")

        version = event.get("version")
        if not isinstance(version, int) or version < 1:
            errors.append(f"events[{index}].version: expected integer >= 1")

        reward = event.get("reward_credits")
        if not isinstance(reward, int) or reward < 0:
            errors.append(f"events[{index}].reward_credits: invalid")

        entry_cost = event.get("entry_cost_credits", 0)
        if not isinstance(entry_cost, int) or entry_cost < 0:
            errors.append(f"events[{index}].entry_cost_credits: invalid")

        route_id = event.get("route_id")
        if not isinstance(route_id, str) or not route_id:
            errors.append(f"events[{index}].route_id: expected non-empty string")

    profile_fields = (
        "patience",
        "awareness",
        "aggression",
        "skill",
        "risk_tolerance",
        "lawfulness",
    )

    for index, profile in enumerate(profiles):
        if not isinstance(profile, dict):
            errors.append(f"driver_profiles[{index}]: expected object")
            continue

        register_id(profile.get("id"), f"driver_profiles[{index}].id")

        for field in profile_fields:
            if not _finite_01(profile.get(field)):
                errors.append(
                    f"driver_profiles[{index}].{field}: expected finite [0,1]"
                )

    return errors


def main(argv: list[str]) -> int:
    if len(argv) != 2:
        print("usage: validate_content_manifest.py <manifest.json>", file=sys.stderr)
        return 2

    path = Path(argv[1])

    try:
        manifest = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        print(f"{path}: {exc}", file=sys.stderr)
        return 2

    errors = validate_manifest(manifest)

    if errors:
        for error in errors:
            print(error, file=sys.stderr)
        return 1

    print(f"{path}: valid Torque Atlas content manifest v1")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
