# PlantGrow GUI Application

A standalone GUI application for interactive tree generation and visualization.

## Dependencies (macOS Apple Silicon)

Install via Homebrew:

```bash
brew install glfw glew
```

ImGui will be included as a header-only library.

## Building

```bash
# Build GUI application
mkdir -p build
cd build
cmake .. -DBUILD_GUI=ON
cmake --build .

# Run GUI
./plantgrow_gui
```

## GUI Features

- **3D Tree Viewer**: Real-time OpenGL rendering
- **Interactive Camera**: Mouse orbit, zoom, pan
- **Parameter Controls**: Sliders for all tree parameters
- **Real-Time Regeneration**: See changes immediately
- **Config Management**: Load/save JSON configs
- **USD Export**: Export current tree to USD
- **Visualization Modes**: Toggle light exposure coloring

## Controls

- **Left Mouse**: Orbit camera around tree
- **Right Mouse**: Pan camera
- **Scroll**: Zoom in/out
- **R**: Reset camera
- **Space**: Regenerate tree

## UI Panels

1. **Generation**: L-System parameters, branch angles
2. **Tropism**: Phototropism/gravitropism controls
3. **Environment**: Light position, ambient light
4. **Visualization**: Color modes, display options
5. **Export**: USD export with options
