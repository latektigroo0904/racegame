#!/usr/bin/env python3
"""Deterministic shortest-time routing over Torque Atlas lane graphs."""

from __future__ import annotations

import heapq
import json
import math
import sys
from pathlib import Path
from typing import Any


def lane_travel_time_seconds(lane: dict[str, Any]) -> float:
    length_m = float(lane.get("length_m", 1.0))
    speed_mps = float(lane.get("speed_limit_mps", 13.8888888889))

    if not math.isfinite(length_m) or length_m < 0.0:
        raise ValueError(f"lane '{lane.get('id')}' has invalid length")

    if not math.isfinite(speed_mps) or speed_mps <= 0.0:
        raise ValueError(f"lane '{lane.get('id')}' has invalid speed limit")

    return length_m / speed_mps


def find_route(
    graph: dict[str, Any],
    start_lane_id: str,
    goal_lane_id: str,
) -> list[str]:
    lanes = graph.get("lanes", [])

    by_id = {
        lane["id"]: lane
        for lane in lanes
    }

    if start_lane_id not in by_id:
        raise ValueError(f"unknown start lane '{start_lane_id}'")

    if goal_lane_id not in by_id:
        raise ValueError(f"unknown goal lane '{goal_lane_id}'")

    if start_lane_id == goal_lane_id:
        return [start_lane_id]

    distances: dict[str, float] = {start_lane_id: 0.0}
    predecessors: dict[str, str] = {}

    queue: list[tuple[float, str]] = [(0.0, start_lane_id)]

    while queue:
        current_cost, current_id = heapq.heappop(queue)

        if current_cost > distances.get(current_id, math.inf):
            continue

        if current_id == goal_lane_id:
            break

        current = by_id[current_id]

        for successor_id in sorted(current.get("successor_lane_ids", [])):
            if successor_id not in by_id:
                raise ValueError(
                    f"lane '{current_id}' references missing successor "
                    f"'{successor_id}'"
                )

            successor_cost = lane_travel_time_seconds(
                by_id[successor_id]
            )

            candidate_cost = current_cost + successor_cost
            previous_cost = distances.get(successor_id, math.inf)

            if candidate_cost < previous_cost - 1.0e-12:
                distances[successor_id] = candidate_cost
                predecessors[successor_id] = current_id
                heapq.heappush(
                    queue,
                    (candidate_cost, successor_id),
                )
            elif abs(candidate_cost - previous_cost) <= 1.0e-12:
                previous_predecessor = predecessors.get(successor_id)

                if (
                    previous_predecessor is None
                    or current_id < previous_predecessor
                ):
                    predecessors[successor_id] = current_id
                    heapq.heappush(
                        queue,
                        (candidate_cost, successor_id),
                    )

    if goal_lane_id not in distances:
        return []

    route = [goal_lane_id]
    current = goal_lane_id

    while current != start_lane_id:
        current = predecessors[current]
        route.append(current)

    route.reverse()
    return route


def main(argv: list[str]) -> int:
    if len(argv) != 4:
        print(
            "usage: route_lane_graph.py <lane_graph.json> "
            "<start_lane_id> <goal_lane_id>",
            file=sys.stderr,
        )
        return 2

    path = Path(argv[1])

    try:
        graph = json.loads(path.read_text(encoding="utf-8"))
        route = find_route(graph, argv[2], argv[3])
    except (OSError, ValueError, KeyError, json.JSONDecodeError) as exc:
        print(f"{path}: {exc}", file=sys.stderr)
        return 1

    if not route:
        print("no route", file=sys.stderr)
        return 1

    print(json.dumps(route))
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
