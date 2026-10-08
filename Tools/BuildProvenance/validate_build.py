#!/usr/bin/env python3
"""Validate Torque Atlas build provenance manifests."""

from __future__ import annotations

import json
import re
import sys
from pathlib import Path
from typing import Any


SHA40 = re.compile(r"^[0-9a-f]{40}$")
SHA256 = re.compile(r"^[0-9a-f]{64}$")
STATUSES = {"not_run", "pass", "fail"}


def validate_build(value: Any) -> list[str]:
    errors: list[str] = []

    if not isinstance(value, dict):
        return ["root: expected object"]

    if value.get("schema_version") != 1:
        errors.append("schema_version: expected 1")

    for field in ("build_id", "unreal_engine_version", "compiler_id", "platform", "configuration"):
        item = value.get(field)
        if not isinstance(item, str) or not item:
            errors.append(f"{field}: expected non-empty string")

    commit = value.get("git_commit_sha")
    if not isinstance(commit, str) or not SHA40.match(commit):
        errors.append("git_commit_sha: expected lowercase 40-char hex")

    if not isinstance(value.get("git_dirty"), bool):
        errors.append("git_dirty: expected boolean")

    for field in ("physics_version", "damage_model_version"):
        item = value.get(field)
        if not isinstance(item, int) or item < 1:
            errors.append(f"{field}: expected integer >= 1")

    content_hash = value.get("content_manifest_sha256")
    if not isinstance(content_hash, str) or not SHA256.match(content_hash):
        errors.append("content_manifest_sha256: expected lowercase sha256")

    verification = value.get("verification")
    if not isinstance(verification, dict):
        errors.append("verification: expected object")
        return errors

    for field in ("source_sanity", "uht", "ubt", "module_load", "automation"):
        status = verification.get(field)
        if status not in STATUSES:
            errors.append(f"verification.{field}: invalid status")

    release_candidate = value.get("release_candidate", False)
    if not isinstance(release_candidate, bool):
        errors.append("release_candidate: expected boolean")
    elif release_candidate:
        if value.get("git_dirty") is not False:
            errors.append("release_candidate: git_dirty must be false")

        for field in ("source_sanity", "uht", "ubt", "module_load", "automation"):
            if verification.get(field) != "pass":
                errors.append(
                    f"release_candidate: verification.{field} must be pass"
                )

    return errors


def main(argv: list[str]) -> int:
    if len(argv) != 2:
        print("usage: validate_build.py <build.json>", file=sys.stderr)
        return 2

    path = Path(argv[1])

    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        print(f"{path}: {exc}", file=sys.stderr)
        return 2

    errors = validate_build(value)

    if errors:
        for error in errors:
            print(error, file=sys.stderr)
        return 1

    print(f"{path}: valid Torque Atlas build provenance v1")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
