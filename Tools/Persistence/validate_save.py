#!/usr/bin/env python3
"""Semantic validator for Torque Atlas save package v1."""

from __future__ import annotations

import json
import math
import re
import sys
from pathlib import Path
from typing import Any


CONTENT_ID_RE = re.compile(r"^[a-z0-9._-]+:[a-zA-Z0-9._-]+$")


def _finite_nonnegative(value: Any) -> bool:
    return (
        isinstance(value, (int, float))
        and math.isfinite(float(value))
        and float(value) >= 0.0
    )


def _finite_01(value: Any) -> bool:
    return (
        isinstance(value, (int, float))
        and math.isfinite(float(value))
        and 0.0 <= float(value) <= 1.0
    )


def validate_save(save: Any) -> list[str]:
    errors: list[str] = []

    if not isinstance(save, dict):
        return ["root: expected object"]

    if save.get("save_schema_version") != 1:
        errors.append("save_schema_version: expected 1")

    revision = save.get("profile_revision")
    if not isinstance(revision, int) or revision < 0:
        errors.append("profile_revision: expected integer >= 0")

    credits = save.get("credits")
    if not isinstance(credits, int) or credits < 0:
        errors.append("credits: expected integer >= 0")

    vehicles = save.get("vehicles", [])
    transactions = save.get("transactions", [])

    if not isinstance(vehicles, list):
        errors.append("vehicles: expected array")
        vehicles = []

    if not isinstance(transactions, list):
        errors.append("transactions: expected array")
        transactions = []

    instance_ids: set[str] = set()

    for index, vehicle in enumerate(vehicles):
        if not isinstance(vehicle, dict):
            errors.append(f"vehicles[{index}]: expected object")
            continue

        instance_id = vehicle.get("instance_id")
        if not isinstance(instance_id, str) or not instance_id:
            errors.append(f"vehicles[{index}].instance_id: invalid")
        elif instance_id in instance_ids:
            errors.append(f"vehicles[{index}].instance_id: duplicate '{instance_id}'")
        else:
            instance_ids.add(instance_id)

        definition_id = vehicle.get("definition_id")
        if not isinstance(definition_id, str) or not CONTENT_ID_RE.match(definition_id):
            errors.append(f"vehicles[{index}].definition_id: expected namespaced id")

        version = vehicle.get("definition_version")
        if not isinstance(version, int) or version < 1:
            errors.append(f"vehicles[{index}].definition_version: invalid")

        if not _finite_nonnegative(vehicle.get("odometer_m")):
            errors.append(f"vehicles[{index}].odometer_m: invalid")

        if not _finite_nonnegative(vehicle.get("engine_hours")):
            errors.append(f"vehicles[{index}].engine_hours: invalid")

        if not _finite_01(vehicle.get("fuel_fraction01")):
            errors.append(f"vehicles[{index}].fuel_fraction01: invalid")

        if not _finite_01(vehicle.get("battery_soc01")):
            errors.append(f"vehicles[{index}].battery_soc01: invalid")

        components = vehicle.get("installed_component_ids", [])
        if not isinstance(components, list):
            errors.append(f"vehicles[{index}].installed_component_ids: expected array")
        else:
            seen_components: set[str] = set()
            for component_id in components:
                if not isinstance(component_id, str) or not CONTENT_ID_RE.match(component_id):
                    errors.append(
                        f"vehicles[{index}].installed_component_ids: "
                        f"invalid id '{component_id}'"
                    )
                elif component_id in seen_components:
                    errors.append(
                        f"vehicles[{index}].installed_component_ids: "
                        f"duplicate '{component_id}'"
                    )
                else:
                    seen_components.add(component_id)

    transaction_ids: set[str] = set()

    for index, transaction in enumerate(transactions):
        if not isinstance(transaction, dict):
            errors.append(f"transactions[{index}]: expected object")
            continue

        transaction_id = transaction.get("id")
        if not isinstance(transaction_id, str) or not transaction_id:
            errors.append(f"transactions[{index}].id: invalid")
        elif transaction_id in transaction_ids:
            errors.append(
                f"transactions[{index}].id: duplicate '{transaction_id}'"
            )
        else:
            transaction_ids.add(transaction_id)

        credit_delta = transaction.get("credit_delta")
        if not isinstance(credit_delta, int):
            errors.append(f"transactions[{index}].credit_delta: expected integer")

    return errors


def main(argv: list[str]) -> int:
    if len(argv) != 2:
        print("usage: validate_save.py <save.json>", file=sys.stderr)
        return 2

    path = Path(argv[1])

    try:
        save = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        print(f"{path}: {exc}", file=sys.stderr)
        return 2

    errors = validate_save(save)

    if errors:
        for error in errors:
            print(error, file=sys.stderr)
        return 1

    print(f"{path}: valid Torque Atlas save package v1")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
