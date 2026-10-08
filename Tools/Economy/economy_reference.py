#!/usr/bin/env python3
"""Reference atomic economy/inventory transaction semantics for Torque Atlas."""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Iterable


@dataclass
class EconomyState:
    credits: int = 0
    vehicle_ids: set[str] = field(default_factory=set)
    part_ids: set[str] = field(default_factory=set)
    applied_transaction_ids: set[str] = field(default_factory=set)


@dataclass(frozen=True)
class EconomyTransaction:
    transaction_id: str
    debit_credits: int = 0
    credit_credits: int = 0
    add_vehicle_ids: tuple[str, ...] = ()
    remove_vehicle_ids: tuple[str, ...] = ()
    add_part_ids: tuple[str, ...] = ()
    remove_part_ids: tuple[str, ...] = ()


class TransactionError(ValueError):
    pass


def _ensure_unique(values: Iterable[str], field_name: str) -> tuple[str, ...]:
    result = tuple(values)

    if any(not isinstance(value, str) or not value for value in result):
        raise TransactionError(f"{field_name}: ids must be non-empty strings")

    if len(result) != len(set(result)):
        raise TransactionError(f"{field_name}: duplicate ids")

    return result


def apply_transaction(
    state: EconomyState,
    transaction: EconomyTransaction,
) -> EconomyState:
    if not transaction.transaction_id:
        raise TransactionError("transaction_id: required")

    if transaction.transaction_id in state.applied_transaction_ids:
        raise TransactionError(
            f"transaction '{transaction.transaction_id}' already applied"
        )

    if transaction.debit_credits < 0 or transaction.credit_credits < 0:
        raise TransactionError("credit amounts must be non-negative")

    add_vehicles = _ensure_unique(
        transaction.add_vehicle_ids,
        "add_vehicle_ids",
    )
    remove_vehicles = _ensure_unique(
        transaction.remove_vehicle_ids,
        "remove_vehicle_ids",
    )
    add_parts = _ensure_unique(
        transaction.add_part_ids,
        "add_part_ids",
    )
    remove_parts = _ensure_unique(
        transaction.remove_part_ids,
        "remove_part_ids",
    )

    if set(add_vehicles) & set(remove_vehicles):
        raise TransactionError("same vehicle cannot be added and removed")

    if set(add_parts) & set(remove_parts):
        raise TransactionError("same part cannot be added and removed")

    final_credits = (
        state.credits
        - transaction.debit_credits
        + transaction.credit_credits
    )

    if final_credits < 0:
        raise TransactionError("insufficient credits")

    missing_vehicles = set(remove_vehicles) - state.vehicle_ids
    if missing_vehicles:
        raise TransactionError(
            f"cannot remove unowned vehicle(s): {sorted(missing_vehicles)}"
        )

    missing_parts = set(remove_parts) - state.part_ids
    if missing_parts:
        raise TransactionError(
            f"cannot remove unowned part(s): {sorted(missing_parts)}"
        )

    duplicate_vehicles = set(add_vehicles) & state.vehicle_ids
    if duplicate_vehicles:
        raise TransactionError(
            f"cannot add already-owned vehicle(s): {sorted(duplicate_vehicles)}"
        )

    duplicate_parts = set(add_parts) & state.part_ids
    if duplicate_parts:
        raise TransactionError(
            f"cannot add already-owned part(s): {sorted(duplicate_parts)}"
        )

    # Commit only after every precondition has passed.
    new_state = EconomyState(
        credits=final_credits,
        vehicle_ids=(state.vehicle_ids - set(remove_vehicles))
        | set(add_vehicles),
        part_ids=(state.part_ids - set(remove_parts))
        | set(add_parts),
        applied_transaction_ids=set(state.applied_transaction_ids)
        | {transaction.transaction_id},
    )

    return new_state
