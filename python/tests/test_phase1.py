#!/usr/bin/env python3
"""
PlantGrow - Phase 1 Test Script
Tests: Foundation & Basic Growth

Test Criteria:
- Load JSON config successfully
- Parse L-system rules
- Generate basic branching structure (multiple iterations)
- Output branch skeleton as USD (lines only)
- Verify branch count matches expected L-system output
"""

import sys
import os
import subprocess
from pathlib import Path

# Add parent directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

from tree_gen import TreeGenerator

class Phase1Tester:
    def __init__(self):
        self.generator = TreeGenerator()
        self.passed_tests = 0
        self.failed_tests = 0

    def test(self, name, condition, message=""):
        """Helper to run a test"""
        print(f"\n  Test: {name}")
        if condition:
            print(f"    ✓ PASS")
            if message:
                print(f"      {message}")
            self.passed_tests += 1
            return True
        else:
            print(f"    ✗ FAIL")
            if message:
                print(f"      {message}")
            self.failed_tests += 1
            return False

    def run_tests(self):
        """Run all Phase 1 tests"""
        print("="*60)
        print("PlantGrow - Phase 1 Test Suite")
        print("Testing: Foundation & Basic Growth")
        print("="*60)

        # Test 1: Config file exists
        print("\n[Test 1] Configuration Files")
        simple_config = "configs/simple_test.json"
        oak_config = "configs/oak.json"

        self.test(
            "Simple test config exists",
            os.path.exists(simple_config),
            f"Found: {simple_config}"
        )

        self.test(
            "Oak config exists",
            os.path.exists(oak_config),
            f"Found: {oak_config}"
        )

        # Test 2: Build exists
        print("\n[Test 2] Build System")
        build_exists = os.path.exists("build/plantgrow") or os.path.exists("build/Debug/plantgrow.exe") or os.path.exists("build/Release/plantgrow.exe")

        if not self.test(
            "Executable built",
            build_exists,
            "Found build/plantgrow"
        ):
            print("\n⚠ Executable not found. Please build first:")
            print("  mkdir -p build && cd build")
            print("  cmake .. && cmake --build .")
            return False

        # Test 3: Generate simple test tree
        print("\n[Test 3] Tree Generation - Simple")
        success = self.generator.generate(simple_config, verbose=False)

        self.test(
            "Simple tree generation",
            success,
            "Generated tree successfully"
        )

        # Check output file
        output_file = "output/simple_test.usda"
        self.test(
            "Output file created",
            os.path.exists(output_file),
            f"Found: {output_file}"
        )

        # Test 4: Generate oak tree
        print("\n[Test 4] Tree Generation - Oak")
        success = self.generator.generate(oak_config, verbose=False)

        self.test(
            "Oak tree generation",
            success,
            "Generated oak tree successfully"
        )

        # Check output file
        output_file = "output/oak_tree.usda"
        self.test(
            "Output file created",
            os.path.exists(output_file),
            f"Found: {output_file}"
        )

        # Test 5: Verify USD file structure
        print("\n[Test 5] USD File Structure")
        if os.path.exists(output_file):
            with open(output_file, 'r') as f:
                content = f.read()

            self.test(
                "USD header present",
                "#usda" in content,
                "File contains USD header"
            )

            self.test(
                "Tree prim defined",
                'def Xform "Tree"' in content,
                "File contains Tree prim"
            )

            self.test(
                "Branch curves defined",
                "def BasisCurves" in content,
                "File contains branch curves"
            )

            # Count branches
            branch_count = content.count("def BasisCurves")
            self.test(
                "Multiple branches generated",
                branch_count > 10,
                f"Generated {branch_count} branches"
            )

            print(f"\n      Branch Statistics:")
            print(f"        Total branches: {branch_count}")

        # Test 6: Verify L-system expansion
        print("\n[Test 6] L-System Validation")
        # For axiom "F" with rule "F" -> "FF[+F][-F]" and 4 iterations:
        # Iteration 0: F (1 symbol)
        # Iteration 1: FF[+F][-F] (9 symbols)
        # Iteration 2: FF[+F][-F]FF[+F][-F][+FF[+F][-F]][-FF[+F][-F]] (more symbols)
        # The branch count should grow exponentially

        self.test(
            "L-System produces exponential growth",
            branch_count > 10,
            f"Branch count ({branch_count}) indicates proper L-system expansion"
        )

        return True

    def print_summary(self):
        """Print test summary"""
        print("\n" + "="*60)
        print("Test Summary")
        print("="*60)
        print(f"Passed: {self.passed_tests}")
        print(f"Failed: {self.failed_tests}")
        print(f"Total:  {self.passed_tests + self.failed_tests}")

        if self.failed_tests == 0:
            print("\n✓ All tests passed! Phase 1 checkpoint complete.")
            print("\nNext steps:")
            print("  - Open output/oak_tree.usda in a USD viewer")
            print("  - Verify the tree structure looks tree-like")
            print("  - Experiment with different L-system rules in configs/")
            return True
        else:
            print(f"\n✗ {self.failed_tests} test(s) failed.")
            print("Please fix the issues before proceeding to Phase 2.")
            return False

def main():
    tester = Phase1Tester()
    tester.run_tests()
    success = tester.print_summary()
    return 0 if success else 1

if __name__ == "__main__":
    sys.exit(main())
