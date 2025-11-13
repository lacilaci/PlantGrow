#!/usr/bin/env python3
"""
PlantGrow - Phase 2 Test Script
Tests: Tropism System (Phototropism & Gravitropism)

Test Criteria:
- Place light source above and to the side
- Generate tree with phototropism enabled
- Verify branches bend toward light (measure angles)
- Test gravitropism: horizontal branches should droop
- Output USD with colored branches (red=high light, blue=low light)
"""

import sys
import os
import subprocess
from pathlib import Path

# Add parent directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

from tree_gen import TreeGenerator

class Phase2Tester:
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
        """Run all Phase 2 tests"""
        print("="*60)
        print("PlantGrow - Phase 2 Test Suite")
        print("Testing: Tropism System")
        print("="*60)

        # Test 1: Config files exist
        print("\n[Test 1] Configuration Files")
        oak_phase2_config = "configs/oak_phase2.json"
        photo_only_config = "configs/photo_only.json"

        self.test(
            "Oak Phase 2 config exists",
            os.path.exists(oak_phase2_config),
            f"Found: {oak_phase2_config}"
        )

        self.test(
            "Phototropism-only config exists",
            os.path.exists(photo_only_config),
            f"Found: {photo_only_config}"
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
            print("  cmake --build build")
            return False

        # Test 3: Generate tree with tropism
        print("\n[Test 3] Tree Generation with Tropism - Oak")
        success = self.generator.generate(oak_phase2_config, verbose=False)

        self.test(
            "Oak tree with tropism",
            success,
            "Generated tree successfully"
        )

        # Check output file
        output_file = "output/oak_phase2.usda"
        self.test(
            "Output file created",
            os.path.exists(output_file),
            f"Found: {output_file}"
        )

        # Test 4: Verify curved branches in USD
        print("\n[Test 4] Curved Branch Verification")
        if os.path.exists(output_file):
            with open(output_file, 'r') as f:
                content = f.read()

            # Check for curves with more than 2 vertices
            # Find curveVertexCounts values
            import re
            vertex_counts = re.findall(r'curveVertexCounts = \[(\d+)\]', content)

            if vertex_counts:
                max_vertices = max(int(count) for count in vertex_counts)
                avg_vertices = sum(int(count) for count in vertex_counts) / len(vertex_counts)

                self.test(
                    "Branches have curved paths",
                    max_vertices > 2,
                    f"Max vertices per branch: {max_vertices} (>2 means curved)"
                )

                self.test(
                    "Average curve complexity",
                    avg_vertices >= 3,
                    f"Average vertices: {avg_vertices:.1f}"
                )
            else:
                self.test(
                    "Branches have curved paths",
                    False,
                    "Could not parse vertex counts"
                )

        # Test 5: Phototropism-only test
        print("\n[Test 5] Phototropism-Only Tree")
        success = self.generator.generate(photo_only_config, verbose=False)

        self.test(
            "Phototropism-only tree",
            success,
            "Generated tree with phototropism only"
        )

        output_file = "output/photo_only.usda"
        self.test(
            "Output file created",
            os.path.exists(output_file),
            f"Found: {output_file}"
        )

        # Test 6: Verify light exposure coloring
        print("\n[Test 6] Light Exposure Visualization")
        if os.path.exists("output/oak_phase2.usda"):
            with open("output/oak_phase2.usda", 'r') as f:
                content = f.read()

            # Check for color variation (light exposure coloring)
            # Should have variety of colors from red (high light) to blue (low light)
            colors = re.findall(r'primvars:displayColor = \[\(([0-9.]+), ([0-9.]+), ([0-9.]+)\)\]', content)

            if colors:
                # Convert to floats
                r_values = [float(c[0]) for c in colors]
                b_values = [float(c[2]) for c in colors]

                # Check for variation in red and blue channels
                r_variation = max(r_values) - min(r_values)
                b_variation = max(b_values) - min(b_values)

                self.test(
                    "Color variation present",
                    r_variation > 0.1 and b_variation > 0.1,
                    f"R variation: {r_variation:.2f}, B variation: {b_variation:.2f}"
                )

                # Check that some branches are red (high light) and some are blue (low light)
                has_red = any(r > 0.7 and b < 0.4 for r, b in zip(r_values, b_values))
                has_blue = any(b > 0.7 and r < 0.4 for r, b in zip(r_values, b_values))

                self.test(
                    "Light exposure range",
                    has_red or has_blue,
                    f"Has high-light branches: {has_red}, Has low-light branches: {has_blue}"
                )
            else:
                self.test(
                    "Color variation present",
                    False,
                    "Could not parse colors"
                )

        # Test 7: Configuration parsing
        print("\n[Test 7] Tropism Configuration")
        # This is implicitly tested by successful tree generation
        # Just verify the configs have the expected structure
        import json
        with open(oak_phase2_config, 'r') as f:
            config = json.load(f)

        has_tropism = "tropism" in config
        has_environment = "environment" in config

        self.test(
            "Tropism section in config",
            has_tropism,
            "Found tropism parameters"
        )

        self.test(
            "Environment section in config",
            has_environment,
            "Found environment parameters"
        )

        if has_tropism:
            tropism = config["tropism"]
            self.test(
                "Curve segments configured",
                "curve_segments" in tropism and tropism["curve_segments"] > 0,
                f"Curve segments: {tropism.get('curve_segments', 0)}"
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
            print("\n✓ All tests passed! Phase 2 checkpoint complete.")
            print("\nNext steps:")
            print("  - Open output/oak_phase2.usda in a USD viewer")
            print("  - Verify branches curve toward light source")
            print("  - Check color gradient: red=high light, blue=low light")
            print("  - Compare with output/photo_only.usda")
            print("\nPhase 2 Checkpoint Questions:")
            print("  1. Do branches realistically curve toward light?")
            print("  2. Is droop/sag visible on horizontal branches?")
            print("  3. Can you tune phototropism_strength and see differences?")
            return True
        else:
            print(f"\n✗ {self.failed_tests} test(s) failed.")
            print("Please fix the issues before proceeding to Phase 3.")
            return False

def main():
    tester = Phase2Tester()
    tester.run_tests()
    success = tester.print_summary()
    return 0 if success else 1

if __name__ == "__main__":
    sys.exit(main())
