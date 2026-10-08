from __future__ import annotations

import copy
import importlib.util
import json
from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]
VALIDATOR_PATH = ROOT / "validate_scenario.py"
EXAMPLE_PATH = ROOT / "examples" / "static_mass_balance_v1.json"

SPEC = importlib.util.spec_from_file_location(
    "validate_scenario",
    VALIDATOR_PATH,
)
assert SPEC is not None and SPEC.loader is not None
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class RegressionScenarioTests(unittest.TestCase):
    def setUp(self) -> None:
        self.scenario = json.loads(EXAMPLE_PATH.read_text(encoding="utf-8"))

    def test_static_example_valid(self) -> None:
        self.assertEqual(MODULE.validate_scenario(self.scenario), [])

    def test_control_program_time_must_be_ordered(self) -> None:
        self.scenario["control_program"] = [
            {"time_seconds": 1.0},
            {"time_seconds": 0.5},
        ]
        errors = MODULE.validate_scenario(self.scenario)
        self.assertTrue(any("nondecreasing" in error for error in errors))

    def test_acceptance_min_must_not_exceed_max(self) -> None:
        self.scenario["acceptance"][0]["minimum_allowed"] = 2.0
        self.scenario["acceptance"][0]["maximum_allowed"] = 1.0
        errors = MODULE.validate_scenario(self.scenario)
        self.assertTrue(any("minimum exceeds maximum" in error for error in errors))

    def test_duplicate_envelope_rejected(self) -> None:
        self.scenario["acceptance"].append(
            copy.deepcopy(self.scenario["acceptance"][0])
        )
        errors = MODULE.validate_scenario(self.scenario)
        self.assertTrue(any("duplicate envelope" in error for error in errors))

    def test_control_time_beyond_duration_rejected(self) -> None:
        self.scenario["control_program"][0]["time_seconds"] = 99.0
        errors = MODULE.validate_scenario(self.scenario)
        self.assertTrue(any("beyond duration_seconds" in error for error in errors))


if __name__ == "__main__":
    unittest.main()
