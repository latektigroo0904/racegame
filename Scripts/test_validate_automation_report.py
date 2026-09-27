#!/usr/bin/env python3

from __future__ import annotations

import json
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
VALIDATOR = ROOT / "Scripts" / "validate_automation_report.py"


def run_validator(
    report: dict,
    prefix: str = "TorqueAtlas.",
    expected_count: int | None = None,
):
    with tempfile.TemporaryDirectory() as temp_dir:
        root = Path(temp_dir)
        report_dir = root / "AutomationReport"
        report_dir.mkdir(parents=True)

        (report_dir / "index.json").write_text(
            json.dumps(report),
            encoding="utf-8",
        )

        summary_path = root / "summary.json"
        command = [
            sys.executable,
            str(VALIDATOR),
            str(report_dir),
            "--prefix",
            prefix,
            "--summary-output",
            str(summary_path),
        ]
        if expected_count is not None:
            command.extend(["--expected-count", str(expected_count)])

        completed = subprocess.run(
            command,
            capture_output=True,
            text=True,
            check=False,
        )

        summary = None
        if summary_path.exists():
            summary = json.loads(
                summary_path.read_text(encoding="utf-8")
            )

        return completed, summary


def valid_report() -> dict:
    return {
        "succeeded": 2,
        "succeededWithWarnings": 1,
        "failed": 0,
        "notRun": 0,
        "inProcess": 0,
        "tests": [
            {
                "fullTestPath": "TorqueAtlas.Vehicle.Runtime.Static",
                "state": "Success",
                "errors": 0,
            },
            {
                "fullTestPath": "TorqueAtlas.Vehicle.Runtime.Braking",
                "state": "SuccessWithWarnings",
                "errors": 0,
            },
            {
                "fullTestPath": "TorqueAtlas.Crash.EndToEnd.Functional",
                "state": "Success",
                "errors": 0,
            },
        ],
    }


def report_under_prefix(prefix: str) -> dict:
    report = valid_report()
    for index, test in enumerate(report["tests"], start=1):
        test["fullTestPath"] = f"{prefix}Regression.Case{index}"
    return report


def assert_true(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def main() -> int:
    completed, summary = run_validator(valid_report())
    assert_true(
        completed.returncode == 0,
        f"valid report should pass: {completed.stderr}",
    )
    assert_true(summary is not None, "valid report should emit summary")
    assert_true(
        summary["matching_test_count"] == 3,
        "summary should count all Torque Atlas tests",
    )

    completed, summary = run_validator(valid_report(), expected_count=3)
    assert_true(
        completed.returncode == 0,
        f"matching expected count should pass: {completed.stderr}",
    )
    assert_true(
        summary is not None and summary["expected_count"] == 3,
        "summary should preserve expected_count",
    )

    completed, _ = run_validator(valid_report(), expected_count=2)
    assert_true(
        completed.returncode != 0,
        "mismatched expected count must be rejected",
    )

    report = valid_report()
    report["failed"] = 1
    report["succeeded"] = 1
    report["tests"][0]["state"] = "Fail"
    report["tests"][0]["errors"] = 1

    completed, _ = run_validator(report)
    assert_true(
        completed.returncode != 0,
        "failed Automation report must be rejected",
    )

    report = valid_report()
    report["notRun"] = 1
    report["succeeded"] = 1
    report["tests"][0]["state"] = "NotRun"
    completed, _ = run_validator(report)
    assert_true(
        completed.returncode != 0,
        "not-run Automation tests must be rejected",
    )

    report = valid_report()
    report["inProcess"] = 1
    report["succeeded"] = 1
    report["tests"][0]["state"] = "InProcess"
    completed, _ = run_validator(report)
    assert_true(
        completed.returncode != 0,
        "in-process Automation tests must be rejected",
    )

    report = valid_report()
    report["tests"][0]["fullTestPath"] = "OtherProject.Test"
    completed, _ = run_validator(report)
    assert_true(
        completed.returncode != 0,
        "report containing non-TorqueAtlas tests must be rejected",
    )

    report = valid_report()
    report["tests"] = []
    report["succeeded"] = 0
    report["succeededWithWarnings"] = 0
    completed, _ = run_validator(report)
    assert_true(
        completed.returncode != 0,
        "empty report must be rejected",
    )

    for prefix in (
        "TorqueAtlas.",
        "TorqueAtlas.Vehicle.",
        "TorqueAtlas._Internal.",
    ):
        completed, _ = run_validator(report_under_prefix(prefix), prefix=prefix)
        assert_true(
            completed.returncode == 0,
            f"valid prefix {prefix!r} should pass: {completed.stderr}",
        )

    for prefix in (
        "TorqueAtlas",
        "TorqueAtlas..Vehicle.",
        "TorqueAtlas.Vehicle-Load.",
        "TorqueAtlas.9Vehicle.",
        "TorqueAtlas.*.",
        "TorqueAtlas.?.",
        " TorqueAtlas.",
        "TorqueAtlas. ",
    ):
        completed, _ = run_validator(valid_report(), prefix=prefix)
        assert_true(
            completed.returncode != 0,
            f"invalid prefix {prefix!r} must be rejected",
        )

    print("Automation report validator tests passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
