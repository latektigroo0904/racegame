#!/usr/bin/env python3
"""Semantic validator for Torque Atlas regression scenario manifests."""

from __future__ import annotations

import json
import math
import re
import sys
from pathlib import Path
from typing import Any


SCENARIO_ID_RE = re.compile(r"^ta\.scenario:[a-zA-Z0-9._-]+$")
CONTENT_ID_RE = re.compile(r"^[a-z0-9._-]+:[a-zA-Z0-9._-]+$")
CATEGORIES = {
    "static_load",
    "braking",
    "acceleration",
    "skidpad",
    "slalom",
    "aero",
    "wet",
    "powertrain",
    "damage",
    "traffic",
}
STATISTICS = {"minimum", "maximum", "mean", "steady_mean", "final"}


def _finite(value: Any) -> bool:
    return isinstance(value, (int, float)) and math.isfinite(float(value))


def validate_scenario(scenario: Any) -> list[str]:
    errors: list[str] = []

    if not isinstance(scenario, dict):
        return ["root: expected object"]

    if scenario.get("schema_version") != 1:
        errors.append("schema_version: expected 1")

    scenario_id = scenario.get("scenario_id")
    if not isinstance(scenario_id, str) or not SCENARIO_ID_RE.match(scenario_id):
        errors.append("scenario_id: expected ta.scenario:<id>")

    version = scenario.get("scenario_version")
    if not isinstance(version, int) or version < 1:
        errors.append("scenario_version: expected integer >= 1")

    if scenario.get("category") not in CATEGORIES:
        errors.append("category: invalid")

    vehicle_id = scenario.get("vehicle_definition_id")
    if not isinstance(vehicle_id, str) or not CONTENT_ID_RE.match(vehicle_id):
        errors.append("vehicle_definition_id: expected namespaced id")

    fixed_step_hz = scenario.get("fixed_step_hz")
    if not _finite(fixed_step_hz) or float(fixed_step_hz) <= 0:
        errors.append("fixed_step_hz: expected finite > 0")

    duration = scenario.get("duration_seconds")
    if not _finite(duration) or float(duration) <= 0:
        errors.append("duration_seconds: expected finite > 0")

    environment = scenario.get("environment")
    if not isinstance(environment, dict):
        errors.append("environment: expected object")
    else:
        wetness = environment.get("surface_wetness01")
        if not _finite(wetness) or not 0.0 <= float(wetness) <= 1.0:
            errors.append("environment.surface_wetness01: expected [0,1]")

        water = environment.get("water_depth_mm")
        if not _finite(water) or float(water) < 0:
            errors.append("environment.water_depth_mm: expected >= 0")

        density = environment.get("air_density_kg_m3")
        if not _finite(density) or float(density) < 0:
            errors.append("environment.air_density_kg_m3: expected >= 0")

        wind = environment.get("wind_world_mps")
        if not isinstance(wind, list) or len(wind) != 3 or not all(_finite(v) for v in wind):
            errors.append("environment.wind_world_mps: expected three finite numbers")

    control_program = scenario.get("control_program", [])
    if not isinstance(control_program, list):
        errors.append("control_program: expected array")
        control_program = []

    previous_time = -1.0

    for index, step in enumerate(control_program):
        if not isinstance(step, dict):
            errors.append(f"control_program[{index}]: expected object")
            continue

        time_seconds = step.get("time_seconds")
        if not _finite(time_seconds) or float(time_seconds) < 0:
            errors.append(f"control_program[{index}].time_seconds: invalid")
            continue

        time_value = float(time_seconds)
        if time_value < previous_time:
            errors.append("control_program: time_seconds must be nondecreasing")

        if _finite(duration) and time_value > float(duration):
            errors.append(
                f"control_program[{index}].time_seconds: beyond duration_seconds"
            )

        previous_time = time_value

        for field in ("throttle01", "brake01", "clutch01"):
            if field in step:
                value = step[field]
                if not _finite(value) or not 0.0 <= float(value) <= 1.0:
                    errors.append(f"control_program[{index}].{field}: expected [0,1]")

        if "steering01" in step:
            value = step["steering01"]
            if not _finite(value) or not -1.0 <= float(value) <= 1.0:
                errors.append(
                    f"control_program[{index}].steering01: expected [-1,1]"
                )

    acceptance = scenario.get("acceptance", [])
    if not isinstance(acceptance, list) or not acceptance:
        errors.append("acceptance: expected non-empty array")
        acceptance = []

    keys: set[tuple[Any, ...]] = set()

    for index, envelope in enumerate(acceptance):
        if not isinstance(envelope, dict):
            errors.append(f"acceptance[{index}]: expected object")
            continue

        metric = envelope.get("metric")
        statistic = envelope.get("statistic")
        wheel_index = envelope.get("wheel_index")

        if not isinstance(metric, str) or not metric:
            errors.append(f"acceptance[{index}].metric: invalid")

        if statistic not in STATISTICS:
            errors.append(f"acceptance[{index}].statistic: invalid")

        if wheel_index is not None and (
            not isinstance(wheel_index, int) or not 0 <= wheel_index <= 3
        ):
            errors.append(f"acceptance[{index}].wheel_index: expected [0,3]")

        minimum = envelope.get("minimum_allowed")
        maximum = envelope.get("maximum_allowed")

        if not _finite(minimum) or not _finite(maximum):
            errors.append(f"acceptance[{index}]: bounds must be finite")
        elif float(minimum) > float(maximum):
            errors.append(f"acceptance[{index}]: minimum exceeds maximum")

        key = (metric, statistic, wheel_index)
        if key in keys:
            errors.append(f"acceptance[{index}]: duplicate envelope key {key}")
        keys.add(key)

    return errors


def main(argv: list[str]) -> int:
    if len(argv) != 2:
        print("usage: validate_scenario.py <scenario.json>", file=sys.stderr)
        return 2

    path = Path(argv[1])

    try:
        scenario = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        print(f"{path}: {exc}", file=sys.stderr)
        return 2

    errors = validate_scenario(scenario)

    if errors:
        for error in errors:
            print(error, file=sys.stderr)
        return 1

    print(f"{path}: valid Torque Atlas regression scenario v1")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
