#!/usr/bin/env python3
"""Validate Torque Atlas QA issue bundles."""

from __future__ import annotations
import json,re,sys
from pathlib import Path,PurePosixPath
from typing import Any
H8=re.compile(r"^[0-9a-f]{8}$"); H64=re.compile(r"^[0-9a-f]{64}$")

def safe_rel(p):
    if not isinstance(p,str) or not p:return False
    q=PurePosixPath(p)
    return not q.is_absolute() and ".." not in q.parts

def validate_bundle(v:Any)->list[str]:
    e=[]
    if not isinstance(v,dict):return["root: expected object"]
    if v.get("schema_version")!=1:e.append("schema_version: expected 1")
    for f in ("issue_id","build_id","summary"):
        if not isinstance(v.get(f),str) or not v[f]:e.append(f"{f}: invalid")
    h=v.get("physics_hash")
    if not isinstance(h,str) or not H8.match(h):e.append("physics_hash: invalid")
    a=v.get("artifacts",[])
    if not isinstance(a,list):return e+["artifacts: expected array"]
    ids=set()
    for i,x in enumerate(a):
        if not isinstance(x,dict):e.append(f"artifacts[{i}]: expected object");continue
        aid=x.get("id");path=x.get("path");sha=x.get("sha256")
        if not isinstance(aid,str) or not aid:e.append(f"artifacts[{i}].id: invalid")
        elif aid in ids:e.append(f"artifacts[{i}].id: duplicate '{aid}'")
        else:ids.add(aid)
        if not safe_rel(path):e.append(f"artifacts[{i}].path: unsafe")
        if not isinstance(sha,str) or not H64.match(sha):e.append(f"artifacts[{i}].sha256: invalid")
    return e

def main(argv):
    if len(argv)!=2:return 2
    p=Path(argv[1])
    try:v=json.loads(p.read_text())
    except (OSError,json.JSONDecodeError) as ex:print(ex,file=sys.stderr);return 2
    e=validate_bundle(v)
    if e:print("\n".join(e),file=sys.stderr);return 1
    print(f"{p}: valid QA bundle v1");return 0
if __name__=="__main__":raise SystemExit(main(sys.argv))
