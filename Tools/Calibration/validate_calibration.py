#!/usr/bin/env python3
"""Semantic validator for Torque Atlas calibration manifests."""

from __future__ import annotations

import json
import math
import re
import sys
from pathlib import Path
from typing import Any


CONTENT_ID_RE = re.compile(r"^[a-z0-9._-]+:[a-zA-Z0-9._-]+$")
PROVENANCE = {"P0", "P1", "P2", "P3", "P4"}
CONFIDENCE = {"low", "medium", "high"}


def validate_calibration(manifest: Any) -> list[str]:
    errors: list[str] = []

    if not isinstance(manifest, dict):
        return ["root: expected object"]

    if manifest.get("schema_version") != 1:
        errors.append("schema_version: expected 1")

    version = manifest.get("version")
    if not isinstance(version, int) or version < 1:
        errors.append("version: expected integer >= 1")

    vehicle_id = manifest.get("vehicle_definition_id")
    if not isinstance(vehicle_id, str) or not CONTENT_ID_RE.match(vehicle_id):
        errors.append("vehicle_definition_id: expected namespaced content id")

    physics_version = manifest.get("physics_version")
    if not isinstance(physics_version, int) or physics_version < 1:
        errors.append("physics_version: expected integer >= 1")

    groups = manifest.get("groups", [])
    if not isinstance(groups, list):
        return errors + ["groups: expected array"]

    group_ids: set[str] = set()

    for group_index, group in enumerate(groups):
        if not isinstance(group, dict):
            errors.append(f"groups[{group_index}]: expected object")
            continue

        group_id = group.get("group_id")
        if not isinstance(group_id, str) or not group_id:
            errors.append(f"groups[{group_index}].group_id: invalid")
        elif group_id in group_ids:
            errors.append(f"groups[{group_index}].group_id: duplicate '{group_id}'")
        else:
            group_ids.add(group_id)

        provenance = group.get("provenance_class")
        if provenance not in PROVENANCE:
            errors.append(f"groups[{group_index}].provenance_class: invalid")

        if group.get("confidence") not in CONFIDENCE:
            errors.append(f"groups[{group_index}].confidence: invalid")

        source_reference = group.get("source_reference", "")
        if provenance in {"P3", "P4"} and (
            not isinstance(source_reference, str) or not source_reference.strip()
        ):
            errors.append(
                f"groups[{group_index}].source_reference: required for {provenance}"
            )

        scenarios = group.get("validation_scenarios", [])
        if not isinstance(scenarios, list):
            errors.append(f"groups[{group_index}].validation_scenarios: expected array")
            scenarios = []

        if len(scenarios) != len(set(scenarios)):
            errors.append(
                f"groups[{group_index}].validation_scenarios: duplicates not allowed"
            )

        if provenance == "P4" and not scenarios:
            errors.append(
                f"groups[{group_index}].validation_scenarios: P4 requires evidence"
            )

        parameters = group.get("parameters", [])
        if not isinstance(parameters, list) or not parameters:
            errors.append(f"groups[{group_index}].parameters: expected non-empty array")
            continue

        parameter_paths: set[str] = set()

        for parameter_index, parameter in enumerate(parameters):
            if not isinstance(parameter, dict):
                errors.append(
                    f"groups[{group_index}].parameters[{parameter_index}]: expected object"
                )
                continue

            path = parameter.get("path")
            if not isinstance(path, str) or not path:
                errors.append(
                    f"groups[{group_index}].parameters[{parameter_index}].path: invalid"
                )
            elif path in parameter_paths:
                errors.append(
                    f"groups[{group_index}].parameters[{parameter_index}].path: "
                    f"duplicate '{path}'"
                )
            else:
                parameter_paths.add(path)

            value = parameter.get("value")
            if not isinstance(value, (int, float)) or not math.isfinite(float(value)):
                errors.append(
                    f"groups[{group_index}].parameters[{parameter_index}].value: "
                    "expected finite number"
                )

            units = parameter.get("units")
            if not isinstance(units, str) or not units:
                errors.append(
                    f"groups[{group_index}].parameters[{parameter_index}].units: invalid"
                )

    return errors


def main(argv: list[str]) -> int:
    if len(argv) != 2:
        print("usage: validate_calibration.py <manifest.json>", file=sys.stderr)
        return 2

    path = Path(argv[1])

    try:
        manifest = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        print(f"{path}: {exc}", file=sys.stderr)
        return 2

    errors = validate_calibration(manifest)

    if errors:
        for error in errors:
            print(error, file=sys.stderr)
        return 1

    print(f"{path}: valid Torque Atlas calibration manifest v1")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
