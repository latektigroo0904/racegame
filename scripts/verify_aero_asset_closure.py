#!/usr/bin/env python3
"""Static source gate for Torque Atlas canonical aerodynamic asset closure.

This does not replace UHT/UBT/Automation. It fails fast when the authored-aero
property, validation code, canonical compiler call, or final hash ordering is
missing or accidentally duplicated.
"""
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
HEADER = ROOT / "Plugins/TorqueVehicleSimulation/Source/TA_Vehicle/Public/TAVehicleDefinition.h"
CPP = ROOT / "Plugins/TorqueVehicleSimulation/Source/TA_Vehicle/Private/TAVehicleDefinition.cpp"


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise AssertionError(f"missing {label}: {needle}")


def require_once(text: str, needle: str, label: str) -> None:
    count = text.count(needle)
    if count != 1:
        raise AssertionError(f"expected exactly one {label}; found {count}: {needle}")


def main() -> int:
    header = HEADER.read_text(encoding="utf-8")
    cpp = CPP.read_text(encoding="utf-8")

    require(header, '#include "TAAerodynamicsDefinition.h"', "aero definition include")
    require_once(header, "FTAAerodynamicsDefinition Aerodynamics;", "authored aero property")
    require(header, 'Category="Physics|Aerodynamics"', "Blueprint/editor aero category")

    require(cpp, '#include "TAVehicleAerodynamicsAssetCompiler.h"', "asset compiler include")
    require(cpp, "TAAerodynamicsDefinition::Validate(Aerodynamics)", "early authored-aero validation")
    require(cpp, 'TEXT("Vehicle.InvalidAerodynamics")', "stable validation code")
    require_once(
        cpp,
        "TAVehicleAerodynamicsAssetCompiler::CompileValidatedAndHash(",
        "canonical aero compile/hash call",
    )

    compile_pos = cpp.index("TAVehicleAerodynamicsAssetCompiler::CompileValidatedAndHash(")
    structure_hash_pos = cpp.rfind("Hash = HashStructureRuntime(", 0, compile_pos)
    final_hash_pos = cpp.find("OutConfig.PhysicsConfigHash =", compile_pos)
    if structure_hash_pos < 0:
        raise AssertionError("aero compile/hash must follow the existing structure hash")
    if final_hash_pos < 0:
        raise AssertionError("aero compile/hash must precede final PhysicsConfigHash assignment")
    if not structure_hash_pos < compile_pos < final_hash_pos:
        raise AssertionError("invalid hash ordering: structure -> aero -> final hash required")

    call_window = cpp[compile_pos:final_hash_pos]
    require(call_window, "OutConfig.CenterOfMassMeters", "canonical COM input")
    require(call_window, "VehicleRuntime.Aerodynamics", "runtime aero destination")

    print("PASS: canonical aero asset ownership/compile/hash source closure is present exactly once.")
    print("NOTE: UE 5.8 UHT/UBT and TorqueAtlas Automation tests are still required.")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        raise SystemExit(1)
