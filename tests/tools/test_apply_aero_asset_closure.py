import importlib.util
import pathlib
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
MODULE_PATH = ROOT / "scripts" / "apply_aero_asset_closure.py"
SPEC = importlib.util.spec_from_file_location("apply_aero_asset_closure", MODULE_PATH)
MODULE = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
SPEC.loader.exec_module(MODULE)


HEADER_BASE = '''#pragma once
#include "TACoreTypes.h"

class UTAVehicleDefinition
{
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physics|Structure")
    FTAVehicleStructureAuthoringDefinition Structure;
};
'''

CPP_BASE = '''#include "TAVehicleDefinition.h"

void Build()
{
    ValidateCoolingAuthoring(
        Cooling,
        OutValidation);

    uint32 Hash = 0;
    Hash = HashStructureRuntime(
        Hash,
        OutConfig.StructureRuntime);

    OutConfig.PhysicsConfigHash = Hash;
}
'''


class AeroAssetClosureApplicatorTests(unittest.TestCase):
    def test_header_inserts_include_and_authored_property_once(self):
        transformed, changed = MODULE.apply_header(HEADER_BASE)
        self.assertTrue(changed)
        self.assertEqual(transformed.count('#include "TAAerodynamicsDefinition.h"'), 1)
        self.assertEqual(transformed.count("FTAAerodynamicsDefinition Aerodynamics;"), 1)
        self.assertLess(
            transformed.index("FTAVehicleStructureAuthoringDefinition Structure;"),
            transformed.index("FTAAerodynamicsDefinition Aerodynamics;"),
        )

    def test_cpp_inserts_validation_and_compile_hash_before_final_hash(self):
        transformed, changed = MODULE.apply_cpp(CPP_BASE)
        self.assertTrue(changed)
        self.assertEqual(transformed.count('#include "TAVehicleAerodynamicsAssetCompiler.h"'), 1)
        self.assertEqual(transformed.count('TEXT("Vehicle.InvalidAerodynamics")'), 2)
        self.assertEqual(
            transformed.count("TAVehicleAerodynamicsAssetCompiler::CompileValidatedAndHash("),
            1,
        )
        self.assertLess(
            transformed.index("Hash = HashStructureRuntime("),
            transformed.index("TAVehicleAerodynamicsAssetCompiler::CompileValidatedAndHash("),
        )
        self.assertLess(
            transformed.index("TAVehicleAerodynamicsAssetCompiler::CompileValidatedAndHash("),
            transformed.index("OutConfig.PhysicsConfigHash = Hash;"),
        )
        self.assertIn("OutConfig.CenterOfMassMeters", transformed)
        self.assertIn("VehicleRuntime.Aerodynamics", transformed)

    def test_second_application_is_idempotent(self):
        header_once, _ = MODULE.apply_header(HEADER_BASE)
        cpp_once, _ = MODULE.apply_cpp(CPP_BASE)
        header_twice, header_changed = MODULE.apply_header(header_once)
        cpp_twice, cpp_changed = MODULE.apply_cpp(cpp_once)
        self.assertFalse(header_changed)
        self.assertFalse(cpp_changed)
        self.assertEqual(header_once, header_twice)
        self.assertEqual(cpp_once, cpp_twice)

    def test_header_source_drift_refuses_edit(self):
        drifted = HEADER_BASE.replace(
            'Category="Physics|Structure"',
            'Category="Vehicle|Structure"',
        )
        with self.assertRaisesRegex(RuntimeError, "header aero property"):
            MODULE.apply_header(drifted)

    def test_cpp_source_drift_refuses_edit(self):
        drifted = CPP_BASE.replace("ValidateCoolingAuthoring", "ValidateCooling")
        with self.assertRaisesRegex(RuntimeError, "cpp validation"):
            MODULE.apply_cpp(drifted)

    def test_ambiguous_anchor_refuses_edit(self):
        ambiguous = CPP_BASE.replace(
            "    Hash = HashStructureRuntime(\n        Hash,\n        OutConfig.StructureRuntime);\n",
            "    Hash = HashStructureRuntime(\n        Hash,\n        OutConfig.StructureRuntime);\n"
            "    Hash = HashStructureRuntime(\n        Hash,\n        OutConfig.StructureRuntime);\n",
        )
        with self.assertRaisesRegex(RuntimeError, "expected exactly one anchor, found 2"):
            MODULE.apply_cpp(ambiguous)


if __name__ == "__main__":
    unittest.main()
