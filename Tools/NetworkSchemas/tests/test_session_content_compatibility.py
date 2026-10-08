from __future__ import annotations

import importlib.util
import json
from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[2]
CONTENT_ROOT = ROOT / "ContentSchemas"
NETWORK_ROOT = ROOT / "NetworkSchemas"


def load_module(name: str, path: Path):
    spec = importlib.util.spec_from_file_location(name, path)
    assert spec is not None and spec.loader is not None
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


HASHER = load_module(
    "hash_manifest",
    CONTENT_ROOT / "hash_manifest.py",
)

SESSION_VALIDATOR = load_module(
    "validate_session",
    NETWORK_ROOT / "validate_session.py",
)


class SessionContentCompatibilityTests(unittest.TestCase):
    def test_real_content_hash_is_valid_ranked_identity(self) -> None:
        content = json.loads(
            (CONTENT_ROOT / "examples" / "base_minimal_manifest.json")
            .read_text(encoding="utf-8")
        )

        session = json.loads(
            (NETWORK_ROOT / "examples" / "ranked_session_v1.json")
            .read_text(encoding="utf-8")
        )

        session["allowed_content_manifest_sha256"] = [
            HASHER.manifest_sha256(content)
        ]

        self.assertEqual(
            SESSION_VALIDATOR.validate_session(session),
            [],
        )


if __name__ == "__main__":
    unittest.main()
