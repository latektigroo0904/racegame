#!/usr/bin/env python3
"""Validate Torque Atlas event definition manifests."""

from __future__ import annotations

import json
import re
import sys
from pathlib import Path
from typing import Any


ID_RE = re.compile(r"^[a-z0-9._-]+:[a-zA-Z0-9._-]+$")
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
START_TYPES = {"standing", "rolling", "staggered", "time_trial", "drag_tree"}
TRAFFIC_POLICIES = {"closed", "controlled", "live"}


def validate_event(v: Any) -> list[str]:
    e: list[str] = []

    if not isinstance(v, dict):
        return ["root: expected object"]

    if v.get("schema_version") != 1:
        e.append("schema_version: expected 1")

    for field in ("event_id", "route_id"):
        value = v.get(field)
        if not isinstance(value, str) or not ID_RE.match(value):
            e.append(f"{field}: expected namespaced id")

    version = v.get("version")
    if not isinstance(version, int) or version < 1:
        e.append("version: expected integer >= 1")

    discipline = v.get("discipline")
    if discipline not in DISCIPLINES:
        e.append("discipline: invalid")

    start_type = v.get("start_type")
    if start_type not in START_TYPES:
        e.append("start_type: invalid")

    traffic_policy = v.get("traffic_policy")
    if traffic_policy not in TRAFFIC_POLICIES:
        e.append("traffic_policy: invalid")

    laps = v.get("laps", 0)
    if not isinstance(laps, int) or laps < 0:
        e.append("laps: expected integer >= 0")

    checkpoints = v.get("checkpoints", [])
    if not isinstance(checkpoints, list):
        e.append("checkpoints: expected array")
        checkpoints = []

    checkpoint_ids: set[str] = set()

    for index, checkpoint in enumerate(checkpoints):
        if not isinstance(checkpoint, dict):
            e.append(f"checkpoints[{index}]: expected object")
            continue

        checkpoint_id = checkpoint.get("id")
        if not isinstance(checkpoint_id, str) or not checkpoint_id:
            e.append(f"checkpoints[{index}].id: invalid")
        elif checkpoint_id in checkpoint_ids:
            e.append(f"checkpoints[{index}].id: duplicate '{checkpoint_id}'")
        else:
            checkpoint_ids.add(checkpoint_id)

        order = checkpoint.get("order")
        if not isinstance(order, int) or order < 0:
            e.append(f"checkpoints[{index}].order: invalid")

    orders = [
        checkpoint.get("order")
        for checkpoint in checkpoints
        if isinstance(checkpoint, dict)
        and isinstance(checkpoint.get("order"), int)
        and checkpoint.get("order") >= 0
    ]

    if len(orders) != len(set(orders)):
        e.append("checkpoints: duplicate order values")

    if orders and sorted(orders) != list(range(len(orders))):
        e.append("checkpoints: order must be contiguous from zero")

    rewards = v.get("rewards")
    if not isinstance(rewards, dict):
        e.append("rewards: expected object")
    else:
        base = rewards.get("base_credits", 0)
        if not isinstance(base, int) or base < 0:
            e.append("rewards.base_credits: invalid")

        placement = rewards.get("placement_credits", [])
        if not isinstance(placement, list):
            e.append("rewards.placement_credits: expected array")
        elif any(not isinstance(x, int) or x < 0 for x in placement):
            e.append("rewards.placement_credits: values must be >= 0")

    eligibility = v.get("eligibility", {})
    if not isinstance(eligibility, dict):
        e.append("eligibility: expected object")
    else:
        minimum = eligibility.get("minimum_performance_class")
        maximum = eligibility.get("maximum_performance_class")

        if minimum is not None and (
            not isinstance(minimum, (int, float)) or minimum < 0
        ):
            e.append("eligibility.minimum_performance_class: invalid")

        if maximum is not None and (
            not isinstance(maximum, (int, float)) or maximum < 0
        ):
            e.append("eligibility.maximum_performance_class: invalid")

        if (
            isinstance(minimum, (int, float))
            and isinstance(maximum, (int, float))
            and minimum > maximum
        ):
            e.append("eligibility: minimum class exceeds maximum")

    if discipline == "circuit" and laps < 1:
        e.append("circuit: laps must be >= 1")

    if discipline in {"point_to_point", "rally", "time_trial"} and len(checkpoints) < 2:
        e.append(f"{discipline}: at least two checkpoints are required")

    if discipline == "drag" and start_type != "drag_tree":
        e.append("drag: start_type must be drag_tree")

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

    errors = validate_event(value)

    if errors:
        print("\n".join(errors), file=sys.stderr)
        return 1

    print(f"{path}: valid event definition v1")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
