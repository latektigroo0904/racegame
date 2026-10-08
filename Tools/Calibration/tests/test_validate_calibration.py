from __future__ import annotations

import copy
import importlib.util
import json
from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]
VALIDATOR_PATH = ROOT / "validate_calibration.py"
EXAMPLE_PATH = ROOT / "examples" / "ta_p01_calibration_v1.json"

SPEC = importlib.util.spec_from_file_location(
    "validate_calibration",
    VALIDATOR_PATH,
)
assert SPEC is not None and SPEC.loader is not None
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class CalibrationValidatorTests(unittest.TestCase):
    def setUp(self) -> None:
        self.manifest = json.loads(EXAMPLE_PATH.read_text(encoding="utf-8"))

    def test_example_valid(self) -> None:
        self.assertEqual(MODULE.validate_calibration(self.manifest), [])

    def test_duplicate_parameter_rejected(self) -> None:
        parameter = copy.deepcopy(self.manifest["groups"][0]["parameters"][0])
        self.manifest["groups"][0]["parameters"].append(parameter)
        errors = MODULE.validate_calibration(self.manifest)
        self.assertTrue(any("duplicate" in error for error in errors))

    def test_nonfinite_parameter_rejected(self) -> None:
        self.manifest["groups"][0]["parameters"][0]["value"] = float("inf")
        errors = MODULE.validate_calibration(self.manifest)
        self.assertTrue(any("finite number" in error for error in errors))

    def test_p3_requires_source(self) -> None:
        self.manifest["groups"][0]["provenance_class"] = "P3"
        self.manifest["groups"][0]["source_reference"] = ""
        errors = MODULE.validate_calibration(self.manifest)
        self.assertTrue(any("source_reference" in error for error in errors))

    def test_p4_requires_validation_scenario(self) -> None:
        self.manifest["groups"][0]["provenance_class"] = "P4"
        self.manifest["groups"][0]["validation_scenarios"] = []
        errors = MODULE.validate_calibration(self.manifest)
        self.assertTrue(any("P4 requires evidence" in error for error in errors))


if __name__ == "__main__":
    unittest.main()
