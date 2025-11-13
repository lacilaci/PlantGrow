#!/bin/bash
# Setup script for PlantGrow GUI

set -e

echo "=== PlantGrow GUI Setup ==="
echo ""

# Check if we're on macOS
if [[ "$(uname)" != "Darwin" ]]; then
    echo "⚠️  This script is designed for macOS"
    echo "   You may need to adapt it for your platform"
fi

# Check for Homebrew
if ! command -v brew &> /dev/null; then
    echo "❌ Homebrew not found. Please install from https://brew.sh"
    exit 1
fi

echo "✓ Homebrew found"

# Install dependencies
echo ""
echo "Installing dependencies..."
brew list glfw &>/dev/null || brew install glfw
echo "✓ GLFW installed"

# Download ImGui if not present
IMGUI_DIR="external/imgui"
if [ ! -d "$IMGUI_DIR" ]; then
    echo ""
    echo "Downloading Dear ImGui..."
    mkdir -p external
    cd external

    # Download ImGui v1.90.1 (docking branch)
    curl -L https://github.com/ocornut/imgui/archive/refs/tags/v1.90.1.tar.gz -o imgui.tar.gz
    tar -xzf imgui.tar.gz
    mv imgui-1.90.1 imgui
    rm imgui.tar.gz

    cd ..
    echo "✓ ImGui downloaded"
else
    echo "✓ ImGui already present"
fi

echo ""
echo "=== Setup Complete ==="
echo ""
echo "Build the GUI application:"
echo "  mkdir -p build && cd build"
echo "  cmake .. -DBUILD_GUI=ON"
echo "  cmake --build ."
echo "  ./plantgrow_gui"
