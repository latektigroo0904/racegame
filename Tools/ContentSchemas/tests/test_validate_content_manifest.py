from __future__ import annotations

import copy
import importlib.util
import json
from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]
VALIDATOR_PATH = ROOT / "validate_content_manifest.py"
EXAMPLE_PATH = ROOT / "examples" / "base_minimal_manifest.json"

SPEC = importlib.util.spec_from_file_location(
    "validate_content_manifest",
    VALIDATOR_PATH,
)
assert SPEC is not None and SPEC.loader is not None
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class ContentManifestValidatorTests(unittest.TestCase):
    def setUp(self) -> None:
        self.manifest = json.loads(EXAMPLE_PATH.read_text(encoding="utf-8"))

    def test_minimal_manifest_is_valid(self) -> None:
        self.assertEqual(MODULE.validate_manifest(self.manifest), [])

    def test_physics_component_requires_calibration(self) -> None:
        del self.manifest["components"][0]["calibration"]

        errors = MODULE.validate_manifest(self.manifest)

        self.assertTrue(any("calibration: required" in error for error in errors))

    def test_cosmetic_component_cannot_affect_physics(self) -> None:
        self.manifest["components"][1]["physics_affecting"] = True
        self.manifest["components"][1]["calibration"] = {
            "provenance_class": "P0",
            "confidence": "low",
        }

        errors = MODULE.validate_manifest(self.manifest)

        self.assertTrue(any("cosmetic component cannot" in error for error in errors))

    def test_wrong_namespace_is_rejected(self) -> None:
        self.manifest["events"][0]["id"] = "other:event"

        errors = MODULE.validate_manifest(self.manifest)

        self.assertTrue(any("does not belong to namespace" in error for error in errors))

    def test_duplicate_id_is_rejected_across_categories(self) -> None:
        duplicate_id = self.manifest["components"][0]["id"]
        self.manifest["driver_profiles"][0]["id"] = duplicate_id

        errors = MODULE.validate_manifest(self.manifest)

        self.assertTrue(any("duplicate id" in error for error in errors))

    def test_driver_profile_out_of_range_is_rejected(self) -> None:
        self.manifest["driver_profiles"][0]["aggression"] = 1.5

        errors = MODULE.validate_manifest(self.manifest)

        self.assertTrue(any("aggression" in error for error in errors))


if __name__ == "__main__":
    unittest.main()
