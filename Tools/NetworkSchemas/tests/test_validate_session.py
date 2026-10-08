from __future__ import annotations

import copy
import importlib.util
import json
from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]
VALIDATOR_PATH = ROOT / "validate_session.py"
EXAMPLE_PATH = ROOT / "examples" / "ranked_session_v1.json"

SPEC = importlib.util.spec_from_file_location(
    "validate_session",
    VALIDATOR_PATH,
)
assert SPEC is not None and SPEC.loader is not None
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class SessionManifestTests(unittest.TestCase):
    def setUp(self) -> None:
        self.manifest = json.loads(EXAMPLE_PATH.read_text(encoding="utf-8"))

    def test_ranked_fixture_valid(self) -> None:
        self.assertEqual(MODULE.validate_session(self.manifest), [])

    def test_ranked_requires_server_authority(self) -> None:
        self.manifest["server_authoritative"] = False
        errors = MODULE.validate_session(self.manifest)
        self.assertTrue(any("server_authoritative" in error for error in errors))

    def test_ranked_rejects_open_mod_policy(self) -> None:
        self.manifest["mod_policy"] = "open"
        errors = MODULE.validate_session(self.manifest)
        self.assertTrue(any("open mod policy" in error for error in errors))

    def test_invalid_physics_hash_rejected(self) -> None:
        self.manifest["allowed_vehicle_physics_hashes"] = ["xyz"]
        errors = MODULE.validate_session(self.manifest)
        self.assertTrue(any("physics_hashes" in error for error in errors))

    def test_duplicate_content_hash_rejected(self) -> None:
        value = self.manifest["allowed_content_manifest_sha256"][0]
        self.manifest["allowed_content_manifest_sha256"].append(value)
        errors = MODULE.validate_session(self.manifest)
        self.assertTrue(any("duplicates" in error for error in errors))

    def test_private_open_mod_session_can_be_non_authoritative(self) -> None:
        manifest = copy.deepcopy(self.manifest)
        manifest["mode"] = "private"
        manifest["mod_policy"] = "open"
        manifest["server_authoritative"] = False
        manifest["allowed_content_manifest_sha256"] = []
        manifest["allowed_vehicle_physics_hashes"] = []

        self.assertEqual(MODULE.validate_session(manifest), [])


if __name__ == "__main__":
    unittest.main()
