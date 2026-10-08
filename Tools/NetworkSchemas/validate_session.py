#!/usr/bin/env python3
"""Semantic validator for Torque Atlas multiplayer session manifests."""

from __future__ import annotations

import json
import re
import sys
from pathlib import Path
from typing import Any


SHA256_RE = re.compile(r"^[0-9a-f]{64}$")
PHYSICS_HASH_RE = re.compile(r"^[0-9a-f]{8}$")
MOD_POLICIES = {"base_only", "whitelist", "open"}
MODES = {"ranked", "unranked", "private"}


def validate_session(manifest: Any) -> list[str]:
    errors: list[str] = []

    if not isinstance(manifest, dict):
        return ["root: expected object"]

    if manifest.get("schema_version") != 1:
        errors.append("schema_version: expected 1")

    mode = manifest.get("mode")
    if mode not in MODES:
        errors.append("mode: invalid")

    session_id = manifest.get("session_id")
    if not isinstance(session_id, str) or not session_id:
        errors.append("session_id: expected non-empty string")

    build_id = manifest.get("game_build_id")
    if not isinstance(build_id, str) or not build_id:
        errors.append("game_build_id: expected non-empty string")

    for field in ("physics_version", "damage_model_version"):
        value = manifest.get(field)
        if not isinstance(value, int) or value < 1:
            errors.append(f"{field}: expected integer >= 1")

    server_authoritative = manifest.get("server_authoritative")
    if not isinstance(server_authoritative, bool):
        errors.append("server_authoritative: expected boolean")

    max_players = manifest.get("max_players")
    if not isinstance(max_players, int) or not 1 <= max_players <= 64:
        errors.append("max_players: expected integer [1,64]")

    mod_policy = manifest.get("mod_policy")
    if mod_policy not in MOD_POLICIES:
        errors.append("mod_policy: invalid")

    content_hashes = manifest.get("allowed_content_manifest_sha256", [])
    physics_hashes = manifest.get("allowed_vehicle_physics_hashes", [])

    if not isinstance(content_hashes, list):
        errors.append("allowed_content_manifest_sha256: expected array")
        content_hashes = []

    if not isinstance(physics_hashes, list):
        errors.append("allowed_vehicle_physics_hashes: expected array")
        physics_hashes = []

    if len(content_hashes) != len(set(content_hashes)):
        errors.append("allowed_content_manifest_sha256: duplicates are not allowed")

    if len(physics_hashes) != len(set(physics_hashes)):
        errors.append("allowed_vehicle_physics_hashes: duplicates are not allowed")

    for index, value in enumerate(content_hashes):
        if not isinstance(value, str) or not SHA256_RE.match(value):
            errors.append(
                f"allowed_content_manifest_sha256[{index}]: invalid sha256"
            )

    for index, value in enumerate(physics_hashes):
        if not isinstance(value, str) or not PHYSICS_HASH_RE.match(value):
            errors.append(
                f"allowed_vehicle_physics_hashes[{index}]: invalid uint32 hex hash"
            )

    if mode == "ranked":
        if server_authoritative is not True:
            errors.append("ranked: server_authoritative must be true")

        if mod_policy == "open":
            errors.append("ranked: open mod policy is not allowed")

        if not content_hashes:
            errors.append("ranked: at least one content manifest hash is required")

        if not physics_hashes:
            errors.append("ranked: at least one vehicle physics hash is required")

    return errors


def main(argv: list[str]) -> int:
    if len(argv) != 2:
        print("usage: validate_session.py <session.json>", file=sys.stderr)
        return 2

    path = Path(argv[1])

    try:
        manifest = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        print(f"{path}: {exc}", file=sys.stderr)
        return 2

    errors = validate_session(manifest)

    if errors:
        for error in errors:
            print(error, file=sys.stderr)
        return 1

    print(f"{path}: valid Torque Atlas session manifest v1")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
