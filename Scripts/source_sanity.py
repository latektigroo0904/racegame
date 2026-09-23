#!/usr/bin/env python3
"""Fast source-level checks that do not require an Unreal Engine installation."""

from __future__ import annotations

import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOTS = [ROOT / "Source", ROOT / "Plugins"]

ERRORS: list[str] = []


def error(message: str) -> None:
    ERRORS.append(message)


def iter_text_files():
    extensions = {".h", ".hpp", ".cpp", ".cs", ".uproject", ".uplugin", ".md", ".py", ".ps1", ".sh"}
    for base in [*SOURCE_ROOTS, ROOT / "Scripts", ROOT / ".github"]:
        if not base.exists():
            continue
        for path in base.rglob("*"):
            if path.is_file() and path.suffix.lower() in extensions:
                yield path


def relative(path: Path) -> str:
    return path.relative_to(ROOT).as_posix()


def strip_cpp_comments_and_literals(text: str) -> str:
    """Preserve newlines while replacing comments/string/char literals with spaces."""
    out: list[str] = []
    i = 0
    state = "code"

    while i < len(text):
        ch = text[i]
        nxt = text[i + 1] if i + 1 < len(text) else ""

        if state == "code":
            if ch == "/" and nxt == "/":
                out.extend("  ")
                i += 2
                state = "line_comment"
                continue
            if ch == "/" and nxt == "*":
                out.extend("  ")
                i += 2
                state = "block_comment"
                continue
            if ch == '"':
                out.append(" ")
                i += 1
                state = "string"
                continue
            if ch == "'":
                out.append(" ")
                i += 1
                state = "char"
                continue
            out.append(ch)
            i += 1
            continue

        if state == "line_comment":
            if ch == "\n":
                out.append("\n")
                state = "code"
            else:
                out.append(" ")
            i += 1
            continue

        if state == "block_comment":
            if ch == "*" and nxt == "/":
                out.extend("  ")
                i += 2
                state = "code"
            else:
                out.append("\n" if ch == "\n" else " ")
                i += 1
            continue

        if state in {"string", "char"}:
            quote = '"' if state == "string" else "'"
            if ch == "\\":
                out.append(" ")
                if i + 1 < len(text):
                    out.append("\n" if text[i + 1] == "\n" else " ")
                    i += 2
                else:
                    i += 1
                continue
            if ch == quote:
                out.append(" ")
                i += 1
                state = "code"
                continue
            out.append("\n" if ch == "\n" else " ")
            i += 1
            continue

    return "".join(out)


def check_conflict_markers(path: Path, text: str) -> None:
    for marker in ("<<<<<<<", "=======", ">>>>>>>"):
        if marker in text:
            error(f"{relative(path)} contains merge-conflict marker {marker!r}")


def check_cpp_delimiters(path: Path, text: str) -> None:
    if path.suffix.lower() not in {".h", ".hpp", ".cpp"}:
        return

    clean = strip_cpp_comments_and_literals(text)
    pairs = {"{": "}", "(": ")", "[": "]"}
    closing = {value: key for key, value in pairs.items()}
    stack: list[tuple[str, int]] = []

    for lineno, line in enumerate(clean.splitlines(), start=1):
        for ch in line:
            if ch in pairs:
                stack.append((ch, lineno))
            elif ch in closing:
                if not stack or stack[-1][0] != closing[ch]:
                    error(f"{relative(path)}:{lineno} has unmatched {ch}")
                    return
                stack.pop()

    if stack:
        ch, lineno = stack[-1]
        error(f"{relative(path)}:{lineno} has unclosed {ch}")


def check_generated_header_order(path: Path, text: str) -> None:
    if path.suffix.lower() not in {".h", ".hpp"}:
        return

    include_lines = [
        (index, line.strip())
        for index, line in enumerate(text.splitlines(), start=1)
        if line.lstrip().startswith("#include")
    ]

    generated = [
        (index, line)
        for index, line in include_lines
        if ".generated.h" in line
    ]

    if not generated:
        return

    if len(generated) != 1:
        error(f"{relative(path)} must include exactly one .generated.h header")
        return

    if generated[0] != include_lines[-1]:
        error(
            f"{relative(path)}:{generated[0][0]} .generated.h must be the final #include"
        )


def check_automation_test_uniqueness() -> None:
    symbol_owner: dict[str, str] = {}
    name_owner: dict[str, str] = {}

    pattern = re.compile(
        r"IMPLEMENT_(?:SIMPLE|COMPLEX)_AUTOMATION_TEST\s*\(\s*"
        r"([A-Za-z_]\w*)\s*,\s*"
        r"(?:TEXT\s*\(\s*)?\"([^\"]+)\"",
        re.MULTILINE,
    )

    for path in SOURCE_ROOTS:
        if not path.exists():
            continue
        for cpp in path.rglob("*.cpp"):
            text = cpp.read_text(encoding="utf-8")
            for symbol, test_name in pattern.findall(text):
                owner = relative(cpp)

                if symbol in symbol_owner:
                    error(
                        f"duplicate Automation test symbol {symbol}: "
                        f"{symbol_owner[symbol]} and {owner}"
                    )
                else:
                    symbol_owner[symbol] = owner

                if test_name in name_owner:
                    error(
                        f"duplicate Automation test name {test_name}: "
                        f"{name_owner[test_name]} and {owner}"
                    )
                else:
                    name_owner[test_name] = owner

    if not name_owner:
        error("no Unreal Automation tests were discovered")


def check_unreal_descriptors() -> None:
    project_path = ROOT / "TorqueAtlas.uproject"
    if not project_path.exists():
        error("TorqueAtlas.uproject is missing")
        return

    project = json.loads(project_path.read_text(encoding="utf-8"))
    if project.get("EngineAssociation") != "5.8":
        error("TorqueAtlas.uproject EngineAssociation must be 5.8")

    plugin_path = ROOT / "Plugins" / "TorqueVehicleSimulation" / "TorqueVehicleSimulation.uplugin"
    if not plugin_path.exists():
        error("TorqueVehicleSimulation.uplugin is missing")
        return

    plugin = json.loads(plugin_path.read_text(encoding="utf-8"))
    modules = [entry.get("Name") for entry in plugin.get("Modules", [])]

    expected = {
        "TA_Core",
        "TA_Surface",
        "TA_Tire",
        "TA_Powertrain",
        "TA_Structure",
        "TA_Damage",
        "TA_Vehicle",
        "TA_Telemetry",
    }

    if set(modules) != expected:
        error(
            "TorqueVehicleSimulation.uplugin module set mismatch: "
            f"expected {sorted(expected)}, got {sorted(modules)}"
        )

    for module in expected:
        build_file = (
            ROOT
            / "Plugins"
            / "TorqueVehicleSimulation"
            / "Source"
            / module
            / f"{module}.Build.cs"
        )
        if not build_file.exists():
            error(f"missing module rules file: {relative(build_file)}")


def main() -> int:
    for path in iter_text_files():
        try:
            text = path.read_text(encoding="utf-8")
        except UnicodeDecodeError:
            error(f"{relative(path)} is not UTF-8 text")
            continue

        check_conflict_markers(path, text)
        check_cpp_delimiters(path, text)
        check_generated_header_order(path, text)

    check_automation_test_uniqueness()
    check_unreal_descriptors()

    if ERRORS:
        print("Torque Atlas source sanity FAILED", file=sys.stderr)
        for message in ERRORS:
            print(f" - {message}", file=sys.stderr)
        return 1

    print("Torque Atlas source sanity passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
