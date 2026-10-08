#!/usr/bin/env python3
"""Evaluate Torque Atlas regression result metrics against a scenario manifest."""

from __future__ import annotations

import json
import math
import sys
from pathlib import Path
from typing import Any


def _metric_key(item: dict[str, Any]) -> tuple[Any, ...]:
    return (
        item.get("metric"),
        item.get("statistic"),
        item.get("wheel_index"),
    )


def evaluate_result(
    scenario: Any,
    result: Any,
) -> tuple[bool, list[str]]:
    failures: list[str] = []

    if not isinstance(scenario, dict):
        return False, ["scenario: expected object"]

    if not isinstance(result, dict):
        return False, ["result: expected object"]

    if result.get("schema_version") != 1:
        failures.append("result.schema_version: expected 1")

    if result.get("scenario_id") != scenario.get("scenario_id"):
        failures.append("result.scenario_id: does not match scenario")

    if result.get("scenario_version") != scenario.get("scenario_version"):
        failures.append("result.scenario_version: does not match scenario")

    physics_hash = result.get("physics_hash")
    if not isinstance(physics_hash, str) or len(physics_hash) != 8:
        failures.append("result.physics_hash: expected 8-character hex string")
    else:
        try:
            int(physics_hash, 16)
        except ValueError:
            failures.append("result.physics_hash: invalid hex")

    metrics = result.get("metrics")
    if not isinstance(metrics, list):
        return False, failures + ["result.metrics: expected array"]

    metrics_by_key: dict[tuple[Any, ...], dict[str, Any]] = {}

    for index, item in enumerate(metrics):
        if not isinstance(item, dict):
            failures.append(f"result.metrics[{index}]: expected object")
            continue

        key = _metric_key(item)

        if key in metrics_by_key:
            failures.append(f"result.metrics[{index}]: duplicate metric key {key}")
            continue

        value = item.get("value")
        if not isinstance(value, (int, float)) or not math.isfinite(float(value)):
            failures.append(f"result.metrics[{index}].value: expected finite number")
            continue

        metrics_by_key[key] = item

    acceptance = scenario.get("acceptance")
    if not isinstance(acceptance, list):
        return False, failures + ["scenario.acceptance: expected array"]

    for index, envelope in enumerate(acceptance):
        if not isinstance(envelope, dict):
            failures.append(f"scenario.acceptance[{index}]: expected object")
            continue

        key = _metric_key(envelope)
        item = metrics_by_key.get(key)

        if item is None:
            failures.append(f"missing result metric for acceptance key {key}")
            continue

        value = float(item["value"])
        minimum = envelope.get("minimum_allowed")
        maximum = envelope.get("maximum_allowed")

        if not isinstance(minimum, (int, float)) or not math.isfinite(float(minimum)):
            failures.append(f"scenario.acceptance[{index}].minimum_allowed: invalid")
            continue

        if not isinstance(maximum, (int, float)) or not math.isfinite(float(maximum)):
            failures.append(f"scenario.acceptance[{index}].maximum_allowed: invalid")
            continue

        if value < float(minimum) or value > float(maximum):
            failures.append(
                f"{key}: value {value:.17g} outside "
                f"[{float(minimum):.17g}, {float(maximum):.17g}]"
            )

    return len(failures) == 0, failures


def main(argv: list[str]) -> int:
    if len(argv) != 3:
        print(
            "usage: evaluate_result.py <scenario.json> <result.json>",
            file=sys.stderr,
        )
        return 2

    scenario_path = Path(argv[1])
    result_path = Path(argv[2])

    try:
        scenario = json.loads(scenario_path.read_text(encoding="utf-8"))
        result = json.loads(result_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        print(exc, file=sys.stderr)
        return 2

    passed, failures = evaluate_result(scenario, result)

    if not passed:
        for failure in failures:
            print(failure, file=sys.stderr)
        return 1

    print(
        f"{result_path}: PASS against "
        f"{scenario.get('scenario_id')} v{scenario.get('scenario_version')}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
