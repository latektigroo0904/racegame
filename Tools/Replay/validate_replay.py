#!/usr/bin/env python3
"""Validate deterministic Torque Atlas replay stream manifests."""

from __future__ import annotations
import json,re,sys
from pathlib import Path
from typing import Any

HASH8=re.compile(r"^[0-9a-f]{8}$")

def validate_replay(v:Any)->list[str]:
    e=[]
    if not isinstance(v,dict): return ["root: expected object"]
    if v.get("schema_version")!=1:e.append("schema_version: expected 1")
    if not isinstance(v.get("replay_id"),str) or not v["replay_id"]:e.append("replay_id: invalid")
    if not isinstance(v.get("build_id"),str) or not v["build_id"]:e.append("build_id: invalid")
    h=v.get("physics_hash")
    if not isinstance(h,str) or not HASH8.match(h):e.append("physics_hash: invalid")
    hz=v.get("fixed_step_hz")
    if not isinstance(hz,(int,float)) or hz<=0:e.append("fixed_step_hz: invalid")
    inputs=v.get("inputs",[]); events=v.get("events",[])
    if not isinstance(inputs,list):e.append("inputs: expected array");inputs=[]
    if not isinstance(events,list):e.append("events: expected array");events=[]
    prev=-1
    for i,x in enumerate(inputs):
        if not isinstance(x,dict):e.append(f"inputs[{i}]: expected object");continue
        tick=x.get("tick")
        if not isinstance(tick,int) or tick<0:e.append(f"inputs[{i}].tick: invalid");continue
        if tick<prev:e.append("inputs: ticks must be nondecreasing")
        prev=tick
        for f,lo,hi in (("throttle01",0,1),("brake01",0,1),("clutch01",0,1),("steering01",-1,1)):
            if f in x and (not isinstance(x[f],(int,float)) or not lo<=x[f]<=hi):
                e.append(f"inputs[{i}].{f}: out of range")
    prev=-1; ids=set()
    for i,x in enumerate(events):
        if not isinstance(x,dict):e.append(f"events[{i}]: expected object");continue
        tick=x.get("tick");eid=x.get("event_id")
        if not isinstance(tick,int) or tick<0:e.append(f"events[{i}].tick: invalid");continue
        if tick<prev:e.append("events: ticks must be nondecreasing")
        prev=tick
        if not isinstance(eid,str) or not eid:e.append(f"events[{i}].event_id: invalid")
        elif eid in ids:e.append(f"events[{i}].event_id: duplicate '{eid}'")
        else:ids.add(eid)
    return e

def main(argv):
    if len(argv)!=2:return 2
    p=Path(argv[1])
    try:v=json.loads(p.read_text())
    except (OSError,json.JSONDecodeError) as ex:print(ex,file=sys.stderr);return 2
    e=validate_replay(v)
    if e:
        print("\n".join(e),file=sys.stderr);return 1
    print(f"{p}: valid replay v1");return 0
if __name__=="__main__":raise SystemExit(main(sys.argv))
