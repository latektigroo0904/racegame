#!/usr/bin/env python3
"""Safely apply Torque Atlas aero asset closure to a local checkout.

This is intentionally narrow and idempotent. It refuses to edit when the
expected source anchors are missing or ambiguous, avoiding destructive whole-
file replacement. Run from the repository root, then run
scripts/verify_aero_asset_closure.py.
"""
from __future__ import annotations

from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
HEADER = ROOT / "Plugins/TorqueVehicleSimulation/Source/TA_Vehicle/Public/TAVehicleDefinition.h"
CPP = ROOT / "Plugins/TorqueVehicleSimulation/Source/TA_Vehicle/Private/TAVehicleDefinition.cpp"


def replace_once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{label}: expected exactly one anchor, found {count}")
    return text.replace(old, new, 1)


def apply_header(text: str) -> tuple[str, bool]:
    changed = False
    if '#include "TAAerodynamicsDefinition.h"' not in text:
        text = replace_once(
            text,
            '#include "TACoreTypes.h"\n',
            '#include "TACoreTypes.h"\n#include "TAAerodynamicsDefinition.h"\n',
            "header aero include",
        )
        changed = True

    if "FTAAerodynamicsDefinition Aerodynamics;" not in text:
        anchor = (
            '    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physics|Structure")\n'
            '    FTAVehicleStructureAuthoringDefinition Structure;\n'
        )
        insertion = anchor + (
            '\n    /**\n'
            '     * Vehicle aerodynamic calibration. The authored application point is\n'
            '     * vehicle-origin-local; BuildCompiledConfig converts it to COM-local\n'
            '     * exactly once through TAVehicleAerodynamicsAssetCompiler.\n'
            '     */\n'
            '    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physics|Aerodynamics")\n'
            '    FTAAerodynamicsDefinition Aerodynamics;\n'
        )
        text = replace_once(text, anchor, insertion, "header aero property")
        changed = True
    return text, changed


def apply_cpp(text: str) -> tuple[str, bool]:
    changed = False
    if '#include "TAVehicleAerodynamicsAssetCompiler.h"' not in text:
        text = replace_once(
            text,
            '#include "TAVehicleDefinition.h"\n',
            '#include "TAVehicleDefinition.h"\n#include "TAVehicleAerodynamicsAssetCompiler.h"\n',
            "cpp compiler include",
        )
        changed = True

    if 'TEXT("Vehicle.InvalidAerodynamics")' not in text:
        anchor = (
            '    ValidateCoolingAuthoring(\n'
            '        Cooling,\n'
            '        OutValidation);\n'
        )
        insertion = anchor + (
            '\n    if (!TAAerodynamicsDefinition::Validate(Aerodynamics))\n'
            '    {\n'
            '        AddValidation(\n'
            '            OutValidation,\n'
            '            ETAValidationSeverity::Error,\n'
            '            TEXT("Vehicle.InvalidAerodynamics"),\n'
            '            TEXT("Aerodynamic reference area, coefficients or application point are invalid."));\n'
            '    }\n'
        )
        text = replace_once(text, anchor, insertion, "cpp validation")
        changed = True

    if "TAVehicleAerodynamicsAssetCompiler::CompileValidatedAndHash(" not in text:
        anchor = (
            '    Hash = HashStructureRuntime(\n'
            '        Hash,\n'
            '        OutConfig.StructureRuntime);\n'
        )
        insertion = anchor + (
            '\n    if (!TAVehicleAerodynamicsAssetCompiler::CompileValidatedAndHash(\n'
            '            Aerodynamics,\n'
            '            OutConfig.CenterOfMassMeters,\n'
            '            Hash,\n'
            '            VehicleRuntime.Aerodynamics))\n'
            '    {\n'
            '        AddValidation(\n'
            '            OutValidation,\n'
            '            ETAValidationSeverity::Error,\n'
            '            TEXT("Vehicle.InvalidAerodynamics"),\n'
            '            TEXT("Aerodynamic calibration failed canonical COM-local compilation."));\n'
            '        return false;\n'
            '    }\n'
        )
        text = replace_once(text, anchor, insertion, "cpp compile/hash call")
        changed = True
    return text, changed


def main() -> int:
    for path in (HEADER, CPP):
        if not path.is_file():
            print(f"ERROR: missing canonical source file: {path.relative_to(ROOT)}", file=sys.stderr)
            return 2

    try:
        header_old = HEADER.read_text(encoding="utf-8")
        cpp_old = CPP.read_text(encoding="utf-8")
        header_new, header_changed = apply_header(header_old)
        cpp_new, cpp_changed = apply_cpp(cpp_old)
    except RuntimeError as exc:
        print(f"REFUSED: {exc}", file=sys.stderr)
        return 3

    # Transactional at validation level: do not write either file until both edits
    # have been fully constructed and all anchors have passed.
    if header_changed:
        HEADER.write_text(header_new, encoding="utf-8", newline="\n")
    if cpp_changed:
        CPP.write_text(cpp_new, encoding="utf-8", newline="\n")

    if header_changed or cpp_changed:
        print("PASS: canonical aero asset closure applied safely.")
    else:
        print("PASS: canonical aero asset closure already present; no changes needed.")
    print("NEXT: python scripts/verify_aero_asset_closure.py")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
