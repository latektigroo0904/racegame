#!/usr/bin/env python3
"""Validate an Unreal Automation report as a Torque Atlas verification gate."""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path


def fail(message: str) -> int:
    print(f"Automation report validation FAILED: {message}", file=sys.stderr)
    return 1


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("report_dir", type=Path)
    parser.add_argument("--prefix", default="TorqueAtlas.")
    parser.add_argument("--summary-output", type=Path)
    args = parser.parse_args()

    report_path = args.report_dir / "index.json"
    if not report_path.is_file():
        return fail(f"missing report file: {report_path}")

    try:
        report = json.loads(report_path.read_text(encoding="utf-8-sig"))
    except (OSError, json.JSONDecodeError) as exc:
        return fail(f"cannot parse {report_path}: {exc}")

    tests = report.get("tests")
    if not isinstance(tests, list) or not tests:
        return fail("report contains no tests")

    failed = int(report.get("failed", 0))
    not_run = int(report.get("notRun", 0))
    in_process = int(report.get("inProcess", 0))
    succeeded = int(report.get("succeeded", 0))
    succeeded_with_warnings = int(report.get("succeededWithWarnings", 0))

    matching = [
        test
        for test in tests
        if isinstance(test, dict)
        and str(test.get("fullTestPath", "")).startswith(args.prefix)
    ]

    if not matching:
        return fail(f"no test path starts with {args.prefix!r}")

    non_matching = [
        str(test.get("fullTestPath", ""))
        for test in tests
        if isinstance(test, dict)
        and not str(test.get("fullTestPath", "")).startswith(args.prefix)
    ]

    if non_matching:
        preview = ", ".join(non_matching[:5])
        return fail(
            "report contains tests outside requested prefix "
            f"{args.prefix!r}: {preview}"
        )

    if failed != 0:
        return fail(f"report records {failed} failed test(s)")

    if not_run != 0:
        return fail(f"report records {not_run} not-run test(s)")

    if in_process != 0:
        return fail(f"report records {in_process} in-process test(s)")

    if succeeded + succeeded_with_warnings <= 0:
        return fail("report contains no succeeded tests")

    failed_states = []
    for test in matching:
        state = str(test.get("state", ""))
        errors = int(test.get("errors", 0) or 0)

        if errors > 0:
            failed_states.append(
                f"{test.get('fullTestPath', '<unknown>')} errors={errors}"
            )
        elif state.lower() in {"fail", "failed", "inprocess", "notrun"}:
            failed_states.append(
                f"{test.get('fullTestPath', '<unknown>')} state={state}"
            )

    if failed_states:
        return fail("; ".join(failed_states[:10]))

    summary = {
        "report_file": str(report_path),
        "prefix": args.prefix,
        "test_count": len(tests),
        "matching_test_count": len(matching),
        "succeeded": succeeded,
        "succeeded_with_warnings": succeeded_with_warnings,
        "failed": failed,
        "not_run": not_run,
        "in_process": in_process,
        "passed": True,
    }

    if args.summary_output:
        args.summary_output.parent.mkdir(parents=True, exist_ok=True)
        args.summary_output.write_text(
            json.dumps(summary, indent=2) + "\n",
            encoding="utf-8",
        )

    print(
        "Automation report validation passed: "
        f"{len(matching)} Torque Atlas test(s), "
        f"{succeeded} succeeded, "
        f"{succeeded_with_warnings} succeeded with warnings."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
