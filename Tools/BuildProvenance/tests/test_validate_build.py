from __future__ import annotations
import copy, importlib.util, json
from pathlib import Path
import unittest

ROOT=Path(__file__).resolve().parents[1]
SPEC=importlib.util.spec_from_file_location("validate_build",ROOT/"validate_build.py")
assert SPEC and SPEC.loader
M=importlib.util.module_from_spec(SPEC); SPEC.loader.exec_module(M)
EX=json.loads((ROOT/"examples"/"source_preflight_build_v1.json").read_text())

class BuildTests(unittest.TestCase):
    def test_example(self): self.assertEqual(M.validate_build(EX),[])
    def test_bad_commit(self):
        x=copy.deepcopy(EX); x["git_commit_sha"]="bad"
        self.assertTrue(any("git_commit_sha" in e for e in M.validate_build(x)))
    def test_release_requires_ue_gates(self):
        x=copy.deepcopy(EX); x["release_candidate"]=True
        errors=M.validate_build(x)
        self.assertTrue(any("verification.uht" in e for e in errors))
    def test_release_requires_clean_git(self):
        x=copy.deepcopy(EX); x["release_candidate"]=True; x["git_dirty"]=True
        for k in x["verification"]: x["verification"][k]="pass"
        self.assertTrue(any("git_dirty" in e for e in M.validate_build(x)))

if __name__=="__main__": unittest.main()
