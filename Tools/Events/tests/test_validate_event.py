from __future__ import annotations

import copy
import importlib.util
import json
from pathlib import Path
import unittest

ROOT=Path(__file__).resolve().parents[1]
S=importlib.util.spec_from_file_location("ve",ROOT/"validate_event.py")
assert S and S.loader
M=importlib.util.module_from_spec(S); S.loader.exec_module(M)
EX=json.loads((ROOT/"examples"/"proving_ground_circuit_v1.json").read_text())

class EventTests(unittest.TestCase):
    def test_valid(self): self.assertEqual(M.validate_event(EX), [])
    def test_circuit_needs_lap(self):
        x=copy.deepcopy(EX); x["laps"]=0
        self.assertTrue(any("laps" in e for e in M.validate_event(x)))
    def test_checkpoint_order_unique(self):
        x=copy.deepcopy(EX); x["checkpoints"][1]["order"]=0
        self.assertTrue(any("duplicate order" in e for e in M.validate_event(x)))
    def test_class_range(self):
        x=copy.deepcopy(EX); x["eligibility"]["minimum_performance_class"]=500
        x["eligibility"]["maximum_performance_class"]=100
        self.assertTrue(any("minimum class exceeds" in e for e in M.validate_event(x)))
    def test_drag_requires_tree(self):
        x=copy.deepcopy(EX); x["discipline"]="drag"; x["start_type"]="standing"
        self.assertTrue(any("drag_tree" in e for e in M.validate_event(x)))

if __name__=="__main__": unittest.main()
