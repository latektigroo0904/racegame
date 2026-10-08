from __future__ import annotations
import copy,importlib.util,json
from pathlib import Path
import unittest
ROOT=Path(__file__).resolve().parents[1]
S=importlib.util.spec_from_file_location("vr",ROOT/"validate_replay.py");assert S and S.loader
M=importlib.util.module_from_spec(S);S.loader.exec_module(M)
EX=json.loads((ROOT/"examples"/"minimal_replay_v1.json").read_text())
class ReplayTests(unittest.TestCase):
 def test_valid(self):self.assertEqual(M.validate_replay(EX),[])
 def test_input_order(self):
  x=copy.deepcopy(EX);x["inputs"][1]["tick"]=-1
  self.assertTrue(M.validate_replay(x))
 def test_duplicate_event(self):
  x=copy.deepcopy(EX);x["events"].append(copy.deepcopy(x["events"][0]))
  self.assertTrue(any("duplicate" in e for e in M.validate_replay(x)))
 def test_control_range(self):
  x=copy.deepcopy(EX);x["inputs"][0]["throttle01"]=2
  self.assertTrue(any("throttle01" in e for e in M.validate_replay(x)))
if __name__=="__main__":unittest.main()
