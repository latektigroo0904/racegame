from __future__ import annotations

import importlib.util
from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]
HASHER_PATH = ROOT / "hash_manifest.py"

SPEC = importlib.util.spec_from_file_location("hash_manifest", HASHER_PATH)
assert SPEC is not None and SPEC.loader is not None
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class ManifestHashTests(unittest.TestCase):
    def test_key_order_does_not_change_hash(self) -> None:
        a = {"b": 2, "a": 1}
        b = {"a": 1, "b": 2}

        self.assertEqual(
            MODULE.manifest_sha256(a),
            MODULE.manifest_sha256(b),
        )

    def test_value_change_changes_hash(self) -> None:
        a = {"a": 1}
        b = {"a": 2}

        self.assertNotEqual(
            MODULE.manifest_sha256(a),
            MODULE.manifest_sha256(b),
        )

    def test_hash_is_lowercase_sha256_hex(self) -> None:
        digest = MODULE.manifest_sha256({"a": 1})

        self.assertEqual(len(digest), 64)
        self.assertEqual(digest, digest.lower())
        int(digest, 16)


if __name__ == "__main__":
    unittest.main()
