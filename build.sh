#!/bin/bash
# PlantGrow Build Script

set -e  # Exit on error

echo "=== PlantGrow Build Script ==="
echo ""

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir -p build
fi

# Enter build directory
cd build

# Configure
echo "Configuring CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
echo ""
echo "Building PlantGrow..."
cmake --build . -j$(nproc 2>/dev/null || echo 4)

echo ""
echo "=== Build Complete ==="
echo ""
echo "Executable: build/plantgrow"
echo ""
echo "To test, run:"
echo "  ./build/plantgrow configs/oak.json"
echo ""
echo "Or run the test suite:"
echo "  python3 python/tests/test_phase1.py"
