from __future__ import annotations

import importlib.util
from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]
BUILDER_PATH = ROOT / "build_lane_graph.py"

SPEC = importlib.util.spec_from_file_location(
    "build_lane_graph",
    BUILDER_PATH,
)
assert SPEC is not None and SPEC.loader is not None
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


def make_package() -> dict:
    return {
        "dataset_id": "traffic-test",
        "segments": [
            {
                "id": "s1",
                "start_node_id": "a",
                "end_node_id": "b",
            },
            {
                "id": "s2",
                "start_node_id": "b",
                "end_node_id": "c",
            },
        ],
        "lanes": [
            {
                "id": "s1-f",
                "segment_id": "s1",
                "direction": "forward",
            },
            {
                "id": "s1-b",
                "segment_id": "s1",
                "direction": "backward",
            },
            {
                "id": "s2-f",
                "segment_id": "s2",
                "direction": "forward",
            },
            {
                "id": "s2-b",
                "segment_id": "s2",
                "direction": "backward",
            },
        ],
    }


class LaneGraphTests(unittest.TestCase):
    def test_forward_chain_connects(self) -> None:
        graph = MODULE.build_lane_graph(make_package())
        by_id = {lane["id"]: lane for lane in graph["lanes"]}

        self.assertEqual(
            by_id["s1-f"]["successor_lane_ids"],
            ["s2-f"],
        )

    def test_backward_chain_connects(self) -> None:
        graph = MODULE.build_lane_graph(make_package())
        by_id = {lane["id"]: lane for lane in graph["lanes"]}

        self.assertEqual(
            by_id["s2-b"]["successor_lane_ids"],
            ["s1-b"],
        )

    def test_immediate_same_segment_uturn_is_not_generated(self) -> None:
        graph = MODULE.build_lane_graph(make_package())
        by_id = {lane["id"]: lane for lane in graph["lanes"]}

        self.assertNotIn(
            "s1-b",
            by_id["s1-f"]["successor_lane_ids"],
        )

    def test_predecessors_are_inverse_of_successors(self) -> None:
        graph = MODULE.build_lane_graph(make_package())
        by_id = {lane["id"]: lane for lane in graph["lanes"]}

        self.assertEqual(
            by_id["s2-f"]["predecessor_lane_ids"],
            ["s1-f"],
        )

    def test_output_is_deterministically_sorted(self) -> None:
        package = make_package()
        package["lanes"].reverse()

        graph = MODULE.build_lane_graph(package)

        ids = [lane["id"] for lane in graph["lanes"]]
        self.assertEqual(ids, sorted(ids))


if __name__ == "__main__":
    unittest.main()
