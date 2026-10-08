from __future__ import annotations

import importlib.util
from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]
PATH = ROOT / "economy_reference.py"

SPEC = importlib.util.spec_from_file_location("economy_reference", PATH)
assert SPEC is not None and SPEC.loader is not None
M = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(M)


class EconomyTransactionTests(unittest.TestCase):
    def test_reward_applies_once(self) -> None:
        state = M.EconomyState(credits=1000)
        tx = M.EconomyTransaction(
            transaction_id="reward-1",
            credit_credits=500,
        )

        result = M.apply_transaction(state, tx)

        self.assertEqual(result.credits, 1500)

        with self.assertRaises(M.TransactionError):
            M.apply_transaction(result, tx)

    def test_purchase_is_atomic(self) -> None:
        state = M.EconomyState(credits=1000)
        tx = M.EconomyTransaction(
            transaction_id="buy-vehicle",
            debit_credits=800,
            add_vehicle_ids=("vehicle-1",),
        )

        result = M.apply_transaction(state, tx)

        self.assertEqual(result.credits, 200)
        self.assertIn("vehicle-1", result.vehicle_ids)

    def test_insufficient_funds_does_not_mutate_input(self) -> None:
        state = M.EconomyState(credits=100)
        tx = M.EconomyTransaction(
            transaction_id="too-expensive",
            debit_credits=500,
            add_vehicle_ids=("vehicle-1",),
        )

        with self.assertRaises(M.TransactionError):
            M.apply_transaction(state, tx)

        self.assertEqual(state.credits, 100)
        self.assertNotIn("vehicle-1", state.vehicle_ids)

    def test_sale_removes_owned_vehicle_and_credits_value(self) -> None:
        state = M.EconomyState(
            credits=100,
            vehicle_ids={"vehicle-1"},
        )

        tx = M.EconomyTransaction(
            transaction_id="sell-vehicle",
            credit_credits=750,
            remove_vehicle_ids=("vehicle-1",),
        )

        result = M.apply_transaction(state, tx)

        self.assertEqual(result.credits, 850)
        self.assertNotIn("vehicle-1", result.vehicle_ids)

    def test_unowned_removal_is_rejected(self) -> None:
        state = M.EconomyState(credits=100)

        with self.assertRaises(M.TransactionError):
            M.apply_transaction(
                state,
                M.EconomyTransaction(
                    transaction_id="bad-remove",
                    remove_part_ids=("part-x",),
                ),
            )


if __name__ == "__main__":
    unittest.main()
