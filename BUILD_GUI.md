# Building PlantGrow GUI on macOS

## Prerequisites

### 1. Install Dependencies

```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install GLFW
brew install glfw
```

### 2. Download ImGui (Automatic)

```bash
# Run the setup script
./setup_gui.sh
```

This will automatically download Dear ImGui v1.90.1 to `external/imgui/`.

## Building

### Option 1: Build GUI Only

```bash
# Create build directory
mkdir -p build
cd build

# Configure with GUI enabled
cmake .. -DBUILD_GUI=ON

# Build
cmake --build . -j8

# The GUI executable will be at: build/plantgrow_gui
```

### Option 2: Build Both CLI and GUI

```bash
mkdir -p build
cd build
cmake .. -DBUILD_GUI=ON
cmake --build . -j8

# You'll have both:
# - build/plantgrow (CLI tool)
# - build/plantgrow_gui (GUI application)
```

## Running

```bash
# From project root
./build/plantgrow_gui

# Or with a specific config
./build/plantgrow_gui
```

## Troubleshooting

### GLFW Not Found

```
CMake Warning: GLFW not found. Install with: brew install glfw
```

**Solution:** Install GLFW with Homebrew:
```bash
brew install glfw
```

### ImGui Not Found

```
fatal error: 'imgui.h' file not found
```

**Solution:** Run the setup script:
```bash
./setup_gui.sh
```

Or manually download ImGui:
```bash
mkdir -p external
cd external
curl -L https://github.com/ocornut/imgui/archive/refs/tags/v1.90.1.tar.gz -o imgui.tar.gz
tar -xzf imgui.tar.gz
mv imgui-1.90.1 imgui
rm imgui.tar.gz
```

### OpenGL Errors on macOS

If you see OpenGL deprecation warnings, these are expected on macOS but the app will still work. We're using OpenGL 3.3 Core Profile which is supported on macOS.

### Build on Apple Silicon (M1/M2/M3)

The project should build natively on Apple Silicon without any special configuration. CMake will automatically detect your architecture.

## Controls

Once the GUI is running:

- **Left Mouse Drag**: Orbit camera around tree
- **Right Mouse Drag**: Pan camera
- **Mouse Scroll**: Zoom in/out
- **R**: Reset camera to default position
- **U**: Toggle UI visibility
- **Space**: Regenerate tree with current parameters
- **ESC**: Quit application

## UI Panels

- **Generation**: L-System parameters, branch angles, iterations
- **Tropism**: Phototropism and gravitropism controls
- **Environment**: Light source position
- **Visualization**: Display options, camera info
- **Export**: USD export functionality

## Performance

- Expected framerate: 60 FPS
- Tree generation: <100ms for typical trees
- Rendering: Real-time, handles 1000+ branches smoothly

## Next Steps

1. Experiment with different L-System iterations (try 3-7)
2. Adjust tropism strengths to see branch bending
3. Move the light source around
4. Export your favorite trees to USD for rendering in Houdini/Octane
