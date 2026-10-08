#!/usr/bin/env python3
"""Compute a deterministic SHA-256 identity for Torque Atlas JSON content."""

from __future__ import annotations

import hashlib
import json
import sys
from pathlib import Path
from typing import Any


def canonical_json_bytes(value: Any) -> bytes:
    return json.dumps(
        value,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
        allow_nan=False,
    ).encode("utf-8")


def manifest_sha256(value: Any) -> str:
    return hashlib.sha256(canonical_json_bytes(value)).hexdigest()


def main(argv: list[str]) -> int:
    if len(argv) != 2:
        print("usage: hash_manifest.py <manifest.json>", file=sys.stderr)
        return 2

    path = Path(argv[1])

    try:
        value = json.loads(path.read_text(encoding="utf-8"))
        digest = manifest_sha256(value)
    except (OSError, json.JSONDecodeError, ValueError) as exc:
        print(f"{path}: {exc}", file=sys.stderr)
        return 2

    print(digest)
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
