#!/usr/bin/env python3
"""Validate an Unreal Automation report as a Torque Atlas verification gate."""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path


COUNTER_FIELDS = (
    "succeeded",
    "succeededWithWarnings",
    "failed",
    "notRun",
    "inProcess",
)


def fail(message: str) -> int:
    print(f"Automation report validation FAILED: {message}", file=sys.stderr)
    return 1


def non_negative_int(value: object, field: str) -> int:
    if isinstance(value, bool) or not isinstance(value, int) or value < 0:
        raise ValueError(f"{field} must be a non-negative integer")
    return value


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("report_dir", type=Path)
    parser.add_argument("--prefix", default="TorqueAtlas.")
    parser.add_argument("--expected-count", type=int)
    parser.add_argument("--summary-output", type=Path)
    args = parser.parse_args()

    if (
        not args.prefix
        or not args.prefix.startswith("TorqueAtlas.")
        or "*" in args.prefix
        or "?" in args.prefix
    ):
        return fail(
            "--prefix must be a literal non-empty TorqueAtlas.* namespace prefix "
            "without wildcard characters"
        )

    if args.expected_count is not None and args.expected_count <= 0:
        return fail("--expected-count must be greater than zero")

    report_path = args.report_dir / "index.json"
    if not report_path.is_file():
        return fail(f"missing report file: {report_path}")

    try:
        report = json.loads(report_path.read_text(encoding="utf-8-sig"))
    except (OSError, json.JSONDecodeError) as exc:
        return fail(f"cannot parse {report_path}: {exc}")

    if not isinstance(report, dict):
        return fail("report root must be an object")

    tests = report.get("tests")
    if not isinstance(tests, list) or not tests:
        return fail("report contains no tests")

    malformed_indices = [
        index for index, test in enumerate(tests) if not isinstance(test, dict)
    ]
    if malformed_indices:
        preview = ", ".join(str(index) for index in malformed_indices[:10])
        return fail(f"tests[] contains non-object entries at index(es): {preview}")

    try:
        counters = {
            field: non_negative_int(report.get(field), field)
            for field in COUNTER_FIELDS
        }
    except ValueError as exc:
        return fail(str(exc))

    aggregate_total = sum(counters.values())
    if aggregate_total != len(tests):
        return fail(
            "aggregate counters do not match tests[] length: "
            f"{aggregate_total} != {len(tests)}"
        )

    matching = [
        test
        for test in tests
        if str(test.get("fullTestPath", "")).startswith(args.prefix)
    ]

    if not matching:
        return fail(f"no test path starts with {args.prefix!r}")

    non_matching = [
        str(test.get("fullTestPath", ""))
        for test in tests
        if not str(test.get("fullTestPath", "")).startswith(args.prefix)
    ]

    if non_matching:
        preview = ", ".join(non_matching[:5])
        return fail(
            "report contains tests outside requested prefix "
            f"{args.prefix!r}: {preview}"
        )

    if args.expected_count is not None and len(matching) != args.expected_count:
        return fail(
            f"expected exactly {args.expected_count} test(s) under "
            f"{args.prefix!r}, found {len(matching)}"
        )

    failed = counters["failed"]
    not_run = counters["notRun"]
    in_process = counters["inProcess"]
    succeeded = counters["succeeded"]
    succeeded_with_warnings = counters["succeededWithWarnings"]

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
        try:
            errors = non_negative_int(test.get("errors"), "test.errors")
        except ValueError as exc:
            return fail(
                f"{test.get('fullTestPath', '<unknown>')}: {exc}"
            )

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
        "expected_count": args.expected_count,
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
        f"{len(matching)} test(s) under {args.prefix!r}, "
        f"{succeeded} succeeded, "
        f"{succeeded_with_warnings} succeeded with warnings."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
