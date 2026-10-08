#!/usr/bin/env python3
"""Derive a deterministic lane adjacency graph from GeoForge normalized data."""

from __future__ import annotations

import json
import sys
from pathlib import Path
from typing import Any


def _lane_endpoints(
    lane: dict[str, Any],
    segments_by_id: dict[str, dict[str, Any]],
) -> tuple[str, str]:
    segment_id = lane["segment_id"]
    segment = segments_by_id[segment_id]

    if lane["direction"] == "forward":
        return segment["start_node_id"], segment["end_node_id"]

    if lane["direction"] == "backward":
        return segment["end_node_id"], segment["start_node_id"]

    raise ValueError(f"lane '{lane.get('id')}' has invalid direction")


def build_lane_graph(package: dict[str, Any]) -> dict[str, Any]:
    segments = package.get("segments", [])
    lanes = package.get("lanes", [])

    segments_by_id = {
        segment["id"]: segment
        for segment in segments
    }

    lane_records: list[dict[str, Any]] = []

    for lane in lanes:
        lane_id = lane["id"]
        start_node_id, end_node_id = _lane_endpoints(
            lane,
            segments_by_id,
        )

        lane_records.append(
            {
                "id": lane_id,
                "segment_id": lane["segment_id"],
                "start_node_id": start_node_id,
                "end_node_id": end_node_id,
                "successor_lane_ids": [],
                "predecessor_lane_ids": [],
            }
        )

    record_by_id = {
        record["id"]: record
        for record in lane_records
    }

    lanes_starting_at: dict[str, list[str]] = {}

    for record in lane_records:
        lanes_starting_at.setdefault(
            record["start_node_id"],
            [],
        ).append(record["id"])

    for record in lane_records:
        candidates = lanes_starting_at.get(
            record["end_node_id"],
            [],
        )

        successors: list[str] = []

        for candidate_id in candidates:
            if candidate_id == record["id"]:
                continue

            candidate = record_by_id[candidate_id]

            # Do not synthesize an immediate U-turn onto the opposite lane
            # of the same segment. Junction/U-turn legality is a later layer.
            if candidate["segment_id"] == record["segment_id"]:
                continue

            successors.append(candidate_id)

        record["successor_lane_ids"] = sorted(successors)

    for record in lane_records:
        for successor_id in record["successor_lane_ids"]:
            record_by_id[successor_id]["predecessor_lane_ids"].append(
                record["id"]
            )

    for record in lane_records:
        record["predecessor_lane_ids"] = sorted(
            record["predecessor_lane_ids"]
        )

    lane_records.sort(key=lambda item: item["id"])

    return {
        "schema_version": 1,
        "source_dataset_id": package.get("dataset_id", ""),
        "lanes": lane_records,
    }


def main(argv: list[str]) -> int:
    if len(argv) != 3:
        print(
            "usage: build_lane_graph.py <geoforge.json> <output.json>",
            file=sys.stderr,
        )
        return 2

    source = Path(argv[1])
    target = Path(argv[2])

    try:
        package = json.loads(source.read_text(encoding="utf-8"))
        graph = build_lane_graph(package)
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(
            json.dumps(graph, indent=2, sort_keys=True) + "\n",
            encoding="utf-8",
        )
    except (OSError, KeyError, ValueError, json.JSONDecodeError) as exc:
        print(f"{source}: {exc}", file=sys.stderr)
        return 1

    print(f"{target}: wrote {len(graph['lanes'])} lanes")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
