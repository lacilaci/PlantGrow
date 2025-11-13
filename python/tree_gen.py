#!/usr/bin/env python3
"""
PlantGrow - Tree Generation Wrapper
Phase 1: Foundation & Basic Growth

This script provides a Python interface to the C++ tree generator.
"""

import subprocess
import sys
import os
from pathlib import Path

class TreeGenerator:
    def __init__(self, executable_path="./build/plantgrow"):
        self.executable = executable_path

    def generate(self, config_path, verbose=True):
        """
        Generate a tree from a JSON configuration file.

        Args:
            config_path: Path to JSON configuration file
            verbose: Print output from generator

        Returns:
            True if successful, False otherwise
        """
        if not os.path.exists(self.executable):
            print(f"Error: Executable not found at {self.executable}")
            print("Please build the project first:")
            print("  mkdir -p build && cd build")
            print("  cmake .. && cmake --build .")
            return False

        if not os.path.exists(config_path):
            print(f"Error: Config file not found: {config_path}")
            return False

        try:
            result = subprocess.run(
                [self.executable, config_path],
                capture_output=True,
                text=True,
                check=False
            )

            if verbose:
                print(result.stdout)
                if result.stderr:
                    print(result.stderr, file=sys.stderr)

            return result.returncode == 0

        except Exception as e:
            print(f"Error running generator: {e}")
            return False

    def batch_generate(self, config_files, verbose=False):
        """
        Generate multiple trees from a list of config files.

        Args:
            config_files: List of paths to JSON configuration files
            verbose: Print output from generator

        Returns:
            Dictionary of {config_path: success_bool}
        """
        results = {}

        for i, config_path in enumerate(config_files):
            print(f"\n[{i+1}/{len(config_files)}] Generating from: {config_path}")
            success = self.generate(config_path, verbose=verbose)
            results[config_path] = success

            if success:
                print(f"  ✓ Success")
            else:
                print(f"  ✗ Failed")

        return results

def main():
    if len(sys.argv) < 2:
        print("PlantGrow - Tree Generation Wrapper")
        print("\nUsage:")
        print(f"  {sys.argv[0]} <config.json>")
        print(f"  {sys.argv[0]} configs/*.json")
        print("\nExample:")
        print(f"  {sys.argv[0]} configs/oak.json")
        return 1

    generator = TreeGenerator()

    # Single file or multiple files
    config_files = sys.argv[1:]

    if len(config_files) == 1:
        # Single file generation
        success = generator.generate(config_files[0], verbose=True)
        return 0 if success else 1
    else:
        # Batch generation
        results = generator.batch_generate(config_files, verbose=False)

        # Summary
        success_count = sum(1 for v in results.values() if v)
        total_count = len(results)

        print(f"\n=== Batch Generation Complete ===")
        print(f"Success: {success_count}/{total_count}")

        return 0 if success_count == total_count else 1

if __name__ == "__main__":
    sys.exit(main())
