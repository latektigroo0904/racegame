from __future__ import annotations

import copy
import importlib.util
import json
from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]
VALIDATOR_PATH = ROOT / "validate_geoforge.py"
EXAMPLE_PATH = ROOT / "examples" / "minimal_network.json"

SPEC = importlib.util.spec_from_file_location(
    "validate_geoforge",
    VALIDATOR_PATH,
)
assert SPEC is not None and SPEC.loader is not None
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class GeoForgeValidatorTests(unittest.TestCase):
    def setUp(self) -> None:
        self.package = json.loads(EXAMPLE_PATH.read_text(encoding="utf-8"))

    def test_minimal_fixture_is_valid(self) -> None:
        self.assertEqual(MODULE.validate_package(self.package), [])

    def test_duplicate_node_id_is_rejected(self) -> None:
        duplicate = copy.deepcopy(self.package["nodes"][0])
        self.package["nodes"].append(duplicate)

        errors = MODULE.validate_package(self.package)

        self.assertTrue(any("duplicate id 'node-a'" in error for error in errors))

    def test_unresolved_segment_node_is_rejected(self) -> None:
        self.package["segments"][0]["end_node_id"] = "missing-node"

        errors = MODULE.validate_package(self.package)

        self.assertTrue(any("unresolved 'missing-node'" in error for error in errors))

    def test_lane_segment_mismatch_is_rejected(self) -> None:
        self.package["lanes"][0]["segment_id"] = "other-segment"

        errors = MODULE.validate_package(self.package)

        self.assertTrue(any("belongs to segment" in error for error in errors))

    def test_nonpositive_lane_width_is_rejected(self) -> None:
        self.package["lanes"][0]["width_m"] = 0.0

        errors = MODULE.validate_package(self.package)

        self.assertTrue(any("width_m: must be positive" in error for error in errors))

    def test_unresolved_surface_segment_is_rejected(self) -> None:
        self.package["surface_zones"][0]["segment_ids"] = ["missing-segment"]

        errors = MODULE.validate_package(self.package)

        self.assertTrue(any("missing-segment" in error for error in errors))


if __name__ == "__main__":
    unittest.main()
