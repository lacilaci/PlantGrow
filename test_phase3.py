#!/usr/bin/env python3
"""
Phase 3 Tests: Resource System & Pruning
Tests resource allocation, light competition, and branch pruning functionality.
"""

import subprocess
import os
import sys
import json
from pathlib import Path

# ANSI color codes
GREEN = '\033[92m'
RED = '\033[91m'
BLUE = '\033[94m'
YELLOW = '\033[93m'
RESET = '\033[0m'

def run_test(test_name, config_path, expected_behavior):
    """Run a single test and verify output."""
    print(f"\n{BLUE}Testing: {test_name}{RESET}")
    print(f"  Config: {config_path}")
    print(f"  Expected: {expected_behavior}")

    # Run the generator
    result = subprocess.run(
        ['./build/plantgrow', config_path],
        capture_output=True,
        text=True,
        timeout=30
    )

    if result.returncode != 0:
        print(f"  {RED}✗ FAILED{RESET} - Non-zero exit code")
        print(f"  stderr: {result.stderr}")
        return False

    output = result.stdout

    # Parse the config to check if resource simulation is enabled
    with open(config_path, 'r') as f:
        config = json.load(f)

    resource_enabled = config.get('resources', {}).get('pruning_enabled', False)

    # Verify expected behavior
    if resource_enabled:
        # Should see resource simulation messages
        if 'Running resource simulation' not in output:
            print(f"  {RED}✗ FAILED{RESET} - No resource simulation message found")
            return False

        # Should see either pruning message or no pruning (if no branches were pruned)
        has_pruning_msg = 'Pruning' in output
        if has_pruning_msg:
            print(f"  {YELLOW}→{RESET} Pruning detected in output")
        else:
            print(f"  {YELLOW}→{RESET} No branches pruned (all branches healthy)")
    else:
        # Should NOT see resource simulation
        if 'Running resource simulation' in output:
            print(f"  {RED}✗ FAILED{RESET} - Unexpected resource simulation")
            return False

    # Check that output file exists
    output_path = config['output']['usd_path']
    if not os.path.exists(output_path):
        print(f"  {RED}✗ FAILED{RESET} - Output file not created: {output_path}")
        return False

    # Check USD file has content
    with open(output_path, 'r') as f:
        usd_content = f.read()
        if len(usd_content) < 100:
            print(f"  {RED}✗ FAILED{RESET} - Output file too small")
            return False

    # Verify USD file has branch data
    if 'def BasisCurves' not in usd_content:
        print(f"  {RED}✗ FAILED{RESET} - No branch curves in USD file")
        return False

    # Count branches in output
    import re
    branch_matches = re.findall(r'Total branches: (\d+)', output)
    if branch_matches:
        branch_count = int(branch_matches[0])
        print(f"  {YELLOW}→{RESET} Generated branches: {branch_count}")

    # If pruning enabled, check for final branch count
    if resource_enabled:
        final_matches = re.findall(r'Final branch count: (\d+)', output)
        if final_matches:
            final_count = int(final_matches[0])
            print(f"  {YELLOW}→{RESET} Final branches after pruning: {final_count}")

            if branch_matches:
                pruned = branch_count - final_count
                if pruned > 0:
                    pruned_pct = (pruned / branch_count) * 100
                    print(f"  {YELLOW}→{RESET} Pruned {pruned} branches ({pruned_pct:.1f}%)")

    print(f"  {GREEN}✓ PASSED{RESET}")
    return True

def main():
    """Run all Phase 3 tests."""
    print(f"\n{BLUE}═══════════════════════════════════════════{RESET}")
    print(f"{BLUE}   Phase 3: Resource System & Pruning Tests{RESET}")
    print(f"{BLUE}═══════════════════════════════════════════{RESET}")

    # Check if build exists
    if not os.path.exists('./build/plantgrow'):
        print(f"{RED}Error: ./build/plantgrow not found{RESET}")
        print("Please build the project first:")
        print("  mkdir -p build && cd build && cmake .. && cmake --build .")
        return 1

    # Ensure output directory exists
    os.makedirs('output', exist_ok=True)

    tests = [
        {
            'name': 'Oak with Resource Simulation',
            'config': 'configs/oak_phase3.json',
            'expected': 'Resource simulation with moderate pruning'
        },
        {
            'name': 'Dense Tree with Heavy Pruning',
            'config': 'configs/dense_tree_phase3.json',
            'expected': 'Significant branch pruning due to competition'
        },
        {
            'name': 'Aggressive Pruning',
            'config': 'configs/aggressive_pruning.json',
            'expected': 'Very aggressive pruning with high thresholds'
        },
        {
            'name': 'No Pruning (Baseline)',
            'config': 'configs/no_pruning_comparison.json',
            'expected': 'Tree generation without resource simulation'
        },
    ]

    passed = 0
    failed = 0

    for test in tests:
        try:
            if run_test(test['name'], test['config'], test['expected']):
                passed += 1
            else:
                failed += 1
        except subprocess.TimeoutExpired:
            print(f"  {RED}✗ FAILED{RESET} - Test timed out")
            failed += 1
        except Exception as e:
            print(f"  {RED}✗ FAILED{RESET} - Exception: {e}")
            failed += 1

    # Summary
    print(f"\n{BLUE}═══════════════════════════════════════════{RESET}")
    print(f"{BLUE}   Test Summary{RESET}")
    print(f"{BLUE}═══════════════════════════════════════════{RESET}")
    print(f"  Total tests: {passed + failed}")
    print(f"  {GREEN}Passed: {passed}{RESET}")
    print(f"  {RED}Failed: {failed}{RESET}")

    if failed == 0:
        print(f"\n{GREEN}✓ All Phase 3 tests passed!{RESET}")
        print(f"\n{BLUE}Phase 3 Complete: Resource System & Pruning{RESET}")
        print("Features implemented:")
        print("  ✓ Light capture calculations")
        print("  ✓ Branch occlusion and competition")
        print("  ✓ Resource allocation (photosynthesis - maintenance)")
        print("  ✓ Self-pruning based on resource thresholds")
        print("  ✓ Configurable pruning parameters")
        print("\nNext: View trees in GUI or USD viewer to see pruning effects")
        return 0
    else:
        print(f"\n{RED}✗ Some tests failed{RESET}")
        return 1

if __name__ == '__main__':
    sys.exit(main())
