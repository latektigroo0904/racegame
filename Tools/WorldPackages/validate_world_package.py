#!/usr/bin/env python3
"""Validate Torque Atlas world package manifests."""

from __future__ import annotations
import json,math,re,sys
from pathlib import Path
from typing import Any

H64=re.compile(r"^[0-9a-f]{64}$")

def finite3(v):
    return isinstance(v,list) and len(v)==3 and all(isinstance(x,(int,float)) and math.isfinite(float(x)) for x in v)

def validate_world(v:Any)->list[str]:
    e=[]
    if not isinstance(v,dict):return["root: expected object"]
    if v.get("schema_version")!=1:e.append("schema_version: expected 1")
    for f in ("world_package_id","dataset_id"):
        if not isinstance(v.get(f),str) or not v[f]:e.append(f"{f}: invalid")
    for f in ("geoforge_sha256","lane_graph_sha256","surface_package_sha256"):
        x=v.get(f)
        if not isinstance(x,str) or not H64.match(x):e.append(f"{f}: invalid sha256")
    chunks=v.get("chunks",[])
    if not isinstance(chunks,list):return e+["chunks: expected array"]
    ids=set()
    for i,c in enumerate(chunks):
        if not isinstance(c,dict):e.append(f"chunks[{i}]: expected object");continue
        cid=c.get("id")
        if not isinstance(cid,str) or not cid:e.append(f"chunks[{i}].id: invalid")
        elif cid in ids:e.append(f"chunks[{i}].id: duplicate '{cid}'")
        else:ids.add(cid)
        mn=c.get("min_m");mx=c.get("max_m")
        if not finite3(mn):e.append(f"chunks[{i}].min_m: invalid")
        if not finite3(mx):e.append(f"chunks[{i}].max_m: invalid")
        if finite3(mn) and finite3(mx):
            if any(float(a)>=float(b) for a,b in zip(mn,mx)):
                e.append(f"chunks[{i}]: min_m must be less than max_m on all axes")
        h=c.get("content_sha256")
        if not isinstance(h,str) or not H64.match(h):e.append(f"chunks[{i}].content_sha256: invalid")
        priority=c.get("streaming_priority",0)
        if not isinstance(priority,int) or priority<0:e.append(f"chunks[{i}].streaming_priority: invalid")
    return e

def main(argv):
    if len(argv)!=2:return 2
    p=Path(argv[1])
    try:v=json.loads(p.read_text())
    except (OSError,json.JSONDecodeError) as ex:print(ex,file=sys.stderr);return 2
    e=validate_world(v)
    if e:print("\n".join(e),file=sys.stderr);return 1
    print(f"{p}: valid world package v1");return 0
if __name__=="__main__":raise SystemExit(main(sys.argv))
