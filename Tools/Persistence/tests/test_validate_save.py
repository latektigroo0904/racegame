from __future__ import annotations

import copy
import importlib.util
import json
from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]
VALIDATOR_PATH = ROOT / "validate_save.py"
EXAMPLE_PATH = ROOT / "examples" / "minimal_save_v1.json"

SPEC = importlib.util.spec_from_file_location("validate_save", VALIDATOR_PATH)
assert SPEC is not None and SPEC.loader is not None
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class SaveValidatorTests(unittest.TestCase):
    def setUp(self) -> None:
        self.save = json.loads(EXAMPLE_PATH.read_text(encoding="utf-8"))

    def test_minimal_save_valid(self) -> None:
        self.assertEqual(MODULE.validate_save(self.save), [])

    def test_negative_credits_rejected(self) -> None:
        self.save["credits"] = -1
        errors = MODULE.validate_save(self.save)
        self.assertTrue(any("credits" in error for error in errors))

    def test_duplicate_vehicle_instance_rejected(self) -> None:
        self.save["vehicles"].append(copy.deepcopy(self.save["vehicles"][0]))
        errors = MODULE.validate_save(self.save)
        self.assertTrue(any("duplicate 'vehicle-0001'" in error for error in errors))

    def test_invalid_content_id_rejected(self) -> None:
        self.save["vehicles"][0]["definition_id"] = "not-namespaced"
        errors = MODULE.validate_save(self.save)
        self.assertTrue(any("definition_id" in error for error in errors))

    def test_condition_fraction_out_of_range_rejected(self) -> None:
        self.save["vehicles"][0]["battery_soc01"] = 1.5
        errors = MODULE.validate_save(self.save)
        self.assertTrue(any("battery_soc01" in error for error in errors))

    def test_duplicate_transaction_rejected(self) -> None:
        self.save["transactions"].append(copy.deepcopy(self.save["transactions"][0]))
        errors = MODULE.validate_save(self.save)
        self.assertTrue(any("transactions" in error and "duplicate" in error for error in errors))


if __name__ == "__main__":
    unittest.main()
