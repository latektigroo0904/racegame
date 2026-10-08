from __future__ import annotations

import copy
import importlib.util
import json
from pathlib import Path
import unittest

ROOT=Path(__file__).resolve().parents[1]
S=importlib.util.spec_from_file_location("vv",ROOT/"validate_vehicle_build.py")
assert S and S.loader
M=importlib.util.module_from_spec(S); S.loader.exec_module(M)
EX=json.loads((ROOT/"examples"/"ta_p01_stock_build_v1.json").read_text())

class VehicleBuildTests(unittest.TestCase):
    def test_valid(self): self.assertEqual(M.validate_vehicle_build(EX), [])
    def test_required_slot(self):
        x=copy.deepcopy(EX); x["installed_parts"]=x["installed_parts"][1:]
        self.assertTrue(any("required slot 'engine'" in e for e in M.validate_vehicle_build(x)))
    def test_wrong_type(self):
        x=copy.deepcopy(EX); x["installed_parts"][0]["component_type"]="brake"
        self.assertTrue(any("not allowed" in e for e in M.validate_vehicle_build(x)))
    def test_duplicate_slot(self):
        x=copy.deepcopy(EX)
        y=copy.deepcopy(x["installed_parts"][0]); y["part_id"]="ta.base:engine-second"
        x["installed_parts"].append(y)
        self.assertTrue(any("already occupied" in e for e in M.validate_vehicle_build(x)))
    def test_hash_deterministic(self):
        a={"b":2,"a":1};b={"a":1,"b":2}
        self.assertEqual(M.canonical_hash(a),M.canonical_hash(b))
    def test_part_change_changes_hash(self):
        x=copy.deepcopy(EX)
        self.assertNotEqual(M.canonical_hash(EX), M.canonical_hash({**x,"base_mass_kg":1421.0}))

if __name__=="__main__": unittest.main()
