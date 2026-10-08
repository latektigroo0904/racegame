from __future__ import annotations

import importlib.util
from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]
ROUTER_PATH = ROOT / "route_lane_graph.py"

SPEC = importlib.util.spec_from_file_location(
    "route_lane_graph",
    ROUTER_PATH,
)
assert SPEC is not None and SPEC.loader is not None
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


def make_graph() -> dict:
    return {
        "schema_version": 1,
        "lanes": [
            {
                "id": "a",
                "length_m": 100.0,
                "speed_limit_mps": 10.0,
                "successor_lane_ids": ["b", "c"],
            },
            {
                "id": "b",
                "length_m": 100.0,
                "speed_limit_mps": 10.0,
                "successor_lane_ids": ["d"],
            },
            {
                "id": "c",
                "length_m": 150.0,
                "speed_limit_mps": 30.0,
                "successor_lane_ids": ["d"],
            },
            {
                "id": "d",
                "length_m": 50.0,
                "speed_limit_mps": 10.0,
                "successor_lane_ids": [],
            },
        ],
    }


class LaneRoutingTests(unittest.TestCase):
    def test_fastest_route_not_fewest_distance(self) -> None:
        route = MODULE.find_route(make_graph(), "a", "d")
        self.assertEqual(route, ["a", "c", "d"])

    def test_same_start_goal(self) -> None:
        self.assertEqual(
            MODULE.find_route(make_graph(), "b", "b"),
            ["b"],
        )

    def test_unreachable_returns_empty(self) -> None:
        graph = make_graph()
        graph["lanes"].append(
            {
                "id": "isolated",
                "length_m": 10.0,
                "speed_limit_mps": 10.0,
                "successor_lane_ids": [],
            }
        )

        self.assertEqual(
            MODULE.find_route(graph, "a", "isolated"),
            [],
        )

    def test_unknown_lane_fails(self) -> None:
        with self.assertRaises(ValueError):
            MODULE.find_route(make_graph(), "missing", "d")

    def test_equal_cost_route_is_deterministic(self) -> None:
        graph = {
            "lanes": [
                {
                    "id": "start",
                    "length_m": 1.0,
                    "speed_limit_mps": 1.0,
                    "successor_lane_ids": ["right", "left"],
                },
                {
                    "id": "left",
                    "length_m": 1.0,
                    "speed_limit_mps": 1.0,
                    "successor_lane_ids": ["goal"],
                },
                {
                    "id": "right",
                    "length_m": 1.0,
                    "speed_limit_mps": 1.0,
                    "successor_lane_ids": ["goal"],
                },
                {
                    "id": "goal",
                    "length_m": 1.0,
                    "speed_limit_mps": 1.0,
                    "successor_lane_ids": [],
                },
            ]
        }

        self.assertEqual(
            MODULE.find_route(graph, "start", "goal"),
            ["start", "left", "goal"],
        )


if __name__ == "__main__":
    unittest.main()
