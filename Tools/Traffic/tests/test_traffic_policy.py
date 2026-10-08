from __future__ import annotations

import importlib.util
from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]
PATH = ROOT / "traffic_policy.py"

SPEC = importlib.util.spec_from_file_location("traffic_policy", PATH)
assert SPEC is not None and SPEC.loader is not None
M = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(M)


class TrafficPolicyTests(unittest.TestCase):
    def test_free_road_accelerates_below_desired_speed(self) -> None:
        value = M.car_following_acceleration_mps2(
            speed_mps=10.0,
            desired_speed_mps=20.0,
            gap_m=1000.0,
            leader_speed_mps=20.0,
        )
        self.assertGreater(value, 0.0)

    def test_close_gap_requests_braking(self) -> None:
        value = M.car_following_acceleration_mps2(
            speed_mps=20.0,
            desired_speed_mps=25.0,
            gap_m=5.0,
            leader_speed_mps=10.0,
        )
        self.assertLess(value, 0.0)

    def test_acceleration_is_bounded(self) -> None:
        config = M.CarFollowingConfig(
            max_accel_mps2=2.0,
            emergency_decel_mps2=7.0,
        )
        value = M.car_following_acceleration_mps2(
            40.0,
            20.0,
            1.0,
            0.0,
            config,
        )
        self.assertGreaterEqual(value, -7.0)
        self.assertLessEqual(value, 2.0)

    def test_lane_change_rejects_short_rear_gap(self) -> None:
        self.assertFalse(
            M.lane_change_is_safe(
                ego_speed_mps=20.0,
                target_front_gap_m=30.0,
                target_front_speed_mps=20.0,
                target_rear_gap_m=5.0,
                target_rear_speed_mps=25.0,
            )
        )

    def test_lane_change_accepts_large_stable_gaps(self) -> None:
        self.assertTrue(
            M.lane_change_is_safe(
                ego_speed_mps=20.0,
                target_front_gap_m=50.0,
                target_front_speed_mps=20.0,
                target_rear_gap_m=50.0,
                target_rear_speed_mps=18.0,
            )
        )

    def test_policy_is_deterministic(self) -> None:
        args = dict(
            speed_mps=17.0,
            desired_speed_mps=23.0,
            gap_m=28.0,
            leader_speed_mps=15.0,
        )
        self.assertEqual(
            M.car_following_acceleration_mps2(**args),
            M.car_following_acceleration_mps2(**args),
        )


if __name__ == "__main__":
    unittest.main()
