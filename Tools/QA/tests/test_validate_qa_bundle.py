from __future__ import annotations
import copy,importlib.util,json
from pathlib import Path
import unittest
ROOT=Path(__file__).resolve().parents[1]
S=importlib.util.spec_from_file_location("vq",ROOT/"validate_qa_bundle.py");assert S and S.loader
M=importlib.util.module_from_spec(S);S.loader.exec_module(M)
EX=json.loads((ROOT/"examples"/"minimal_qa_bundle_v1.json").read_text())
class QaTests(unittest.TestCase):
 def test_valid(self):self.assertEqual(M.validate_bundle(EX),[])
 def test_traversal_rejected(self):
  x=copy.deepcopy(EX);x["artifacts"][0]["path"]="../secret"
  self.assertTrue(any("unsafe" in e for e in M.validate_bundle(x)))
 def test_absolute_rejected(self):
  x=copy.deepcopy(EX);x["artifacts"][0]["path"]="/tmp/a"
  self.assertTrue(any("unsafe" in e for e in M.validate_bundle(x)))
 def test_duplicate_artifact(self):
  x=copy.deepcopy(EX);x["artifacts"].append(copy.deepcopy(x["artifacts"][0]))
  self.assertTrue(any("duplicate" in e for e in M.validate_bundle(x)))
if __name__=="__main__":unittest.main()
