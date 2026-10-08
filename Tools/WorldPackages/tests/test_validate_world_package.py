from __future__ import annotations
import copy,importlib.util,json
from pathlib import Path
import unittest
ROOT=Path(__file__).resolve().parents[1]
S=importlib.util.spec_from_file_location("vw",ROOT/"validate_world_package.py");assert S and S.loader
M=importlib.util.module_from_spec(S);S.loader.exec_module(M)
EX=json.loads((ROOT/"examples"/"prototype_world_v1.json").read_text())
class WorldTests(unittest.TestCase):
 def test_valid(self):self.assertEqual(M.validate_world(EX),[])
 def test_duplicate_chunk(self):
  x=copy.deepcopy(EX);x["chunks"].append(copy.deepcopy(x["chunks"][0]))
  self.assertTrue(any("duplicate" in e for e in M.validate_world(x)))
 def test_bad_bounds(self):
  x=copy.deepcopy(EX);x["chunks"][0]["max_m"][0]=0
  self.assertTrue(any("min_m must" in e for e in M.validate_world(x)))
 def test_bad_hash(self):
  x=copy.deepcopy(EX);x["lane_graph_sha256"]="bad"
  self.assertTrue(any("lane_graph_sha256" in e for e in M.validate_world(x)))
if __name__=="__main__":unittest.main()
