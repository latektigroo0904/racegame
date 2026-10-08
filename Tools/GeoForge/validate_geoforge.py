#!/usr/bin/env python3
"""Semantic validator for Torque Atlas GeoForge normalized packages."""

from __future__ import annotations

import json
import math
import sys
from pathlib import Path
from typing import Any, Iterable


def _finite(value: Any) -> bool:
    return isinstance(value, (int, float)) and math.isfinite(float(value))


def _validate_point3(point: Any, path: str, errors: list[str]) -> None:
    if not isinstance(point, list) or len(point) != 3:
        errors.append(f"{path}: expected [x, y, z]")
        return

    for index, value in enumerate(point):
        if not _finite(value):
            errors.append(f"{path}[{index}]: coordinate must be finite")


def _unique_ids(items: Iterable[dict[str, Any]], kind: str, errors: list[str]) -> set[str]:
    ids: set[str] = set()

    for index, item in enumerate(items):
        item_id = item.get("id")
        if not isinstance(item_id, str) or not item_id:
            errors.append(f"{kind}[{index}].id: expected non-empty string")
            continue

        if item_id in ids:
            errors.append(f"{kind}: duplicate id '{item_id}'")
        ids.add(item_id)

    return ids


def validate_package(package: Any) -> list[str]:
    errors: list[str] = []

    if not isinstance(package, dict):
        return ["root: expected object"]

    if package.get("schema_version") != 1:
        errors.append("schema_version: expected 1")

    coordinate_system = package.get("coordinate_system")
    if not isinstance(coordinate_system, dict):
        errors.append("coordinate_system: expected object")
    else:
        if coordinate_system.get("units") != "m":
            errors.append("coordinate_system.units: expected 'm'")
        if coordinate_system.get("axis_convention") != "x_forward_y_right_z_up":
            errors.append(
                "coordinate_system.axis_convention: expected "
                "'x_forward_y_right_z_up'"
            )

    nodes = package.get("nodes", [])
    segments = package.get("segments", [])
    lanes = package.get("lanes", [])
    surface_zones = package.get("surface_zones", [])

    for name, value in (
        ("nodes", nodes),
        ("segments", segments),
        ("lanes", lanes),
        ("surface_zones", surface_zones),
    ):
        if not isinstance(value, list):
            errors.append(f"{name}: expected array")

    if errors:
        return errors

    node_ids = _unique_ids(nodes, "nodes", errors)
    segment_ids = _unique_ids(segments, "segments", errors)
    lane_ids = _unique_ids(lanes, "lanes", errors)
    _unique_ids(surface_zones, "surface_zones", errors)

    for index, node in enumerate(nodes):
        for field in ("x_m", "y_m", "z_m"):
            if not _finite(node.get(field)):
                errors.append(f"nodes[{index}].{field}: expected finite number")

    lane_by_id = {
        lane.get("id"): lane
        for lane in lanes
        if isinstance(lane.get("id"), str)
    }

    for index, segment in enumerate(segments):
        start = segment.get("start_node_id")
        end = segment.get("end_node_id")

        if start not in node_ids:
            errors.append(f"segments[{index}].start_node_id: unresolved '{start}'")
        if end not in node_ids:
            errors.append(f"segments[{index}].end_node_id: unresolved '{end}'")
        if start == end and start is not None:
            errors.append(f"segments[{index}]: start and end node must differ")

        speed = segment.get("speed_limit_mps")
        if not _finite(speed) or float(speed) < 0:
            errors.append(f"segments[{index}].speed_limit_mps: invalid")

        centerline = segment.get("centerline")
        if not isinstance(centerline, list) or len(centerline) < 2:
            errors.append(f"segments[{index}].centerline: need at least two points")
        else:
            for point_index, point in enumerate(centerline):
                _validate_point3(
                    point,
                    f"segments[{index}].centerline[{point_index}]",
                    errors,
                )

        referenced_lane_ids = segment.get("lane_ids")
        if not isinstance(referenced_lane_ids, list):
            errors.append(f"segments[{index}].lane_ids: expected array")
            continue

        for lane_id in referenced_lane_ids:
            if lane_id not in lane_ids:
                errors.append(
                    f"segments[{index}].lane_ids: unresolved lane '{lane_id}'"
                )
                continue

            lane = lane_by_id.get(lane_id)
            if lane is not None and lane.get("segment_id") != segment.get("id"):
                errors.append(
                    f"segments[{index}].lane_ids: lane '{lane_id}' belongs to "
                    f"segment '{lane.get('segment_id')}'"
                )

    for index, lane in enumerate(lanes):
        segment_id = lane.get("segment_id")
        if segment_id not in segment_ids:
            errors.append(f"lanes[{index}].segment_id: unresolved '{segment_id}'")

        if lane.get("direction") not in ("forward", "backward"):
            errors.append(f"lanes[{index}].direction: invalid")

        width = lane.get("width_m")
        if not _finite(width) or float(width) <= 0:
            errors.append(f"lanes[{index}].width_m: must be positive")

        centerline = lane.get("centerline")
        if not isinstance(centerline, list) or len(centerline) < 2:
            errors.append(f"lanes[{index}].centerline: need at least two points")
        else:
            for point_index, point in enumerate(centerline):
                _validate_point3(
                    point,
                    f"lanes[{index}].centerline[{point_index}]",
                    errors,
                )

    for index, zone in enumerate(surface_zones):
        for field in ("dry_friction_baseline", "wet_friction_baseline"):
            value = zone.get(field)
            if not _finite(value) or float(value) < 0:
                errors.append(f"surface_zones[{index}].{field}: invalid")

        drainage = zone.get("drainage01")
        if not _finite(drainage) or not 0.0 <= float(drainage) <= 1.0:
            errors.append(f"surface_zones[{index}].drainage01: expected [0,1]")

        referenced_segments = zone.get("segment_ids", [])
        if not isinstance(referenced_segments, list):
            errors.append(f"surface_zones[{index}].segment_ids: expected array")
        else:
            for segment_id in referenced_segments:
                if segment_id not in segment_ids:
                    errors.append(
                        f"surface_zones[{index}].segment_ids: unresolved "
                        f"segment '{segment_id}'"
                    )

    return errors


def main(argv: list[str]) -> int:
    if len(argv) != 2:
        print("usage: validate_geoforge.py <package.json>", file=sys.stderr)
        return 2

    path = Path(argv[1])

    try:
        package = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        print(f"{path}: {exc}", file=sys.stderr)
        return 2

    errors = validate_package(package)

    if errors:
        for error in errors:
            print(error, file=sys.stderr)
        return 1

    print(f"{path}: valid GeoForge normalized package v1")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
