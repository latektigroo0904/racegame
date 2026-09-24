#!/usr/bin/env python3
from __future__ import annotations

import importlib.util
from pathlib import Path
import unittest

SCRIPT = Path(__file__).resolve().parents[2] / "scripts/apply_compliant_midpoint_reaction_fix.py"
spec = importlib.util.spec_from_file_location("midpoint_fix", SCRIPT)
module = importlib.util.module_from_spec(spec)
assert spec.loader is not None
spec.loader.exec_module(module)


class MidpointFixTests(unittest.TestCase):
    def test_applies_exactly_once(self) -> None:
        source = "prefix\n" + module.OLD + "\nsuffix\n"
        result, changed = module.apply(source)
        self.assertTrue(changed)
        self.assertNotIn(module.OLD, result)
        self.assertEqual(result.count(module.NEW), 1)

    def test_is_idempotent(self) -> None:
        source = "prefix\n" + module.NEW + "\nsuffix\n"
        result, changed = module.apply(source)
        self.assertFalse(changed)
        self.assertEqual(result, source)

    def test_refuses_missing_anchor(self) -> None:
        with self.assertRaises(RuntimeError):
            module.apply("source drift")

    def test_refuses_ambiguous_unfixed_anchor(self) -> None:
        with self.assertRaises(RuntimeError):
            module.apply(module.OLD + "\n" + module.OLD)

    def test_refuses_ambiguous_fixed_anchor(self) -> None:
        with self.assertRaises(RuntimeError):
            module.apply(module.NEW + "\n" + module.NEW)

    def test_refuses_mixed_fixed_and_unfixed_state(self) -> None:
        with self.assertRaises(RuntimeError):
            module.apply(module.NEW + "\n" + module.OLD)


if __name__ == "__main__":
    unittest.main()
