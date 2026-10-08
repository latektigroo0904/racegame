from __future__ import annotations

import copy
import importlib.util
import json
from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]
EVALUATOR_PATH = ROOT / "evaluate_result.py"
SCENARIO_PATH = ROOT / "examples" / "static_mass_balance_v1.json"
RESULT_PATH = ROOT / "examples" / "static_mass_balance_result_pass_v1.json"

SPEC = importlib.util.spec_from_file_location(
    "evaluate_result",
    EVALUATOR_PATH,
)
assert SPEC is not None and SPEC.loader is not None
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class RegressionResultEvaluatorTests(unittest.TestCase):
    def setUp(self) -> None:
        self.scenario = json.loads(SCENARIO_PATH.read_text(encoding="utf-8"))
        self.result = json.loads(RESULT_PATH.read_text(encoding="utf-8"))

    def test_passing_fixture_passes(self) -> None:
        passed, failures = MODULE.evaluate_result(self.scenario, self.result)
        self.assertTrue(passed)
        self.assertEqual(failures, [])

    def test_missing_metric_fails(self) -> None:
        self.result["metrics"].pop()
        passed, failures = MODULE.evaluate_result(self.scenario, self.result)
        self.assertFalse(passed)
        self.assertTrue(any("missing result metric" in failure for failure in failures))

    def test_out_of_range_metric_fails(self) -> None:
        self.result["metrics"][0]["value"] = 0.5
        passed, failures = MODULE.evaluate_result(self.scenario, self.result)
        self.assertFalse(passed)
        self.assertTrue(any("outside" in failure for failure in failures))

    def test_scenario_version_mismatch_fails(self) -> None:
        self.result["scenario_version"] = 99
        passed, failures = MODULE.evaluate_result(self.scenario, self.result)
        self.assertFalse(passed)
        self.assertTrue(any("scenario_version" in failure for failure in failures))

    def test_duplicate_metric_key_fails(self) -> None:
        self.result["metrics"].append(copy.deepcopy(self.result["metrics"][0]))
        passed, failures = MODULE.evaluate_result(self.scenario, self.result)
        self.assertFalse(passed)
        self.assertTrue(any("duplicate metric key" in failure for failure in failures))


if __name__ == "__main__":
    unittest.main()
