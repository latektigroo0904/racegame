#!/usr/bin/env python3
"""Reference traffic-policy primitives for Torque Atlas offline regression."""

from __future__ import annotations

import math
from dataclasses import dataclass


@dataclass(frozen=True)
class CarFollowingConfig:
    max_accel_mps2: float = 2.0
    comfortable_decel_mps2: float = 3.0
    emergency_decel_mps2: float = 8.0
    desired_time_headway_s: float = 1.5
    minimum_gap_m: float = 2.0
    acceleration_exponent: float = 4.0


@dataclass(frozen=True)
class LaneChangeSafetyConfig:
    minimum_front_gap_m: float = 8.0
    minimum_rear_gap_m: float = 8.0
    rear_time_headway_s: float = 1.0
    max_required_rear_decel_mps2: float = 4.0


def car_following_acceleration_mps2(
    speed_mps: float,
    desired_speed_mps: float,
    gap_m: float,
    leader_speed_mps: float,
    config: CarFollowingConfig = CarFollowingConfig(),
) -> float:
    values = (
        speed_mps,
        desired_speed_mps,
        gap_m,
        leader_speed_mps,
        config.max_accel_mps2,
        config.comfortable_decel_mps2,
        config.emergency_decel_mps2,
        config.desired_time_headway_s,
        config.minimum_gap_m,
        config.acceleration_exponent,
    )

    if not all(math.isfinite(value) for value in values):
        raise ValueError("car-following input must be finite")

    if desired_speed_mps <= 0.0:
        return -config.emergency_decel_mps2

    speed = max(0.0, speed_mps)
    leader_speed = max(0.0, leader_speed_mps)
    gap = max(1.0e-3, gap_m)

    accel = max(1.0e-6, config.max_accel_mps2)
    decel = max(1.0e-6, config.comfortable_decel_mps2)

    closing_speed = speed - leader_speed

    desired_dynamic_gap = (
        config.minimum_gap_m
        + speed * config.desired_time_headway_s
        + max(
            0.0,
            speed * closing_speed
            / (2.0 * math.sqrt(accel * decel)),
        )
    )

    free_road_term = (
        speed / max(desired_speed_mps, 1.0e-6)
    ) ** config.acceleration_exponent

    interaction_term = (
        desired_dynamic_gap / gap
    ) ** 2

    requested = accel * (
        1.0
        - free_road_term
        - interaction_term
    )

    return max(
        -config.emergency_decel_mps2,
        min(config.max_accel_mps2, requested),
    )


def lane_change_is_safe(
    ego_speed_mps: float,
    target_front_gap_m: float,
    target_front_speed_mps: float,
    target_rear_gap_m: float,
    target_rear_speed_mps: float,
    config: LaneChangeSafetyConfig = LaneChangeSafetyConfig(),
) -> bool:
    values = (
        ego_speed_mps,
        target_front_gap_m,
        target_front_speed_mps,
        target_rear_gap_m,
        target_rear_speed_mps,
    )

    if not all(math.isfinite(value) for value in values):
        raise ValueError("lane-change input must be finite")

    ego_speed = max(0.0, ego_speed_mps)
    front_gap = max(0.0, target_front_gap_m)
    rear_gap = max(0.0, target_rear_gap_m)
    rear_speed = max(0.0, target_rear_speed_mps)

    if front_gap < config.minimum_front_gap_m:
        return False

    required_rear_gap = max(
        config.minimum_rear_gap_m,
        rear_speed * config.rear_time_headway_s,
    )

    if rear_gap < required_rear_gap:
        return False

    if rear_speed <= ego_speed:
        return True

    closing_speed = rear_speed - ego_speed

    required_rear_decel = (
        closing_speed * closing_speed
        / max(2.0 * rear_gap, 1.0e-6)
    )

    return (
        required_rear_decel
        <= config.max_required_rear_decel_mps2
    )
