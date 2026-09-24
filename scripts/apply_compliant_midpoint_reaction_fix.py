#!/usr/bin/env python3
"""Safely apply the compliant-contact midpoint reaction fix.

The compliant bisection midpoint must receive the same
AdditionalSuspensionReactionN used by the endpoint evaluations. This narrow,
idempotent applicator refuses source drift instead of reconstructing the large
resolver file.
"""
from __future__ import annotations

from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
CPP = ROOT / "Plugins/TorqueVehicleSimulation/Source/TA_Vehicle/Private/TAWheelContactResolver.cpp"

OLD = """            if (!EvaluateCompliantTravel(\n                    Chassis,\n                    GeometryConfig,\n                    SuspensionConfig,\n                    TireConfig,\n                    InOutSuspensionState,\n                    InOutTireState,\n                    RackDisplacementM,\n                    DamageOffsets,\n                    Road.PointWorldM,\n                    RoadNormal,\n                    MidTravelM,\n                    DeltaTimeSeconds,\n                    Mid))"""

NEW = """            if (!EvaluateCompliantTravel(\n                    Chassis,\n                    GeometryConfig,\n                    SuspensionConfig,\n                    TireConfig,\n                    InOutSuspensionState,\n                    InOutTireState,\n                    RackDisplacementM,\n                    AdditionalSuspensionReactionN,\n                    DamageOffsets,\n                    Road.PointWorldM,\n                    RoadNormal,\n                    MidTravelM,\n                    DeltaTimeSeconds,\n                    Mid))"""


def apply(text: str) -> tuple[str, bool]:
    if NEW in text:
        if text.count(NEW) != 1:
            raise RuntimeError(f"fixed midpoint anchor is ambiguous: {text.count(NEW)} matches")
        if OLD in text:
            raise RuntimeError("both fixed and unfixed midpoint anchors are present")
        return text, False

    count = text.count(OLD)
    if count != 1:
        raise RuntimeError(f"expected exactly one unfixed midpoint anchor, found {count}")
    return text.replace(OLD, NEW, 1), True


def main() -> int:
    if not CPP.is_file():
        print(f"ERROR: missing canonical source file: {CPP.relative_to(ROOT)}", file=sys.stderr)
        return 2

    try:
        old = CPP.read_text(encoding="utf-8")
        new, changed = apply(old)
    except RuntimeError as exc:
        print(f"REFUSED: {exc}", file=sys.stderr)
        return 3

    if changed:
        CPP.write_text(new, encoding="utf-8", newline="\n")
        print("PASS: compliant midpoint reaction fix applied safely.")
    else:
        print("PASS: compliant midpoint reaction fix already present; no changes needed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
