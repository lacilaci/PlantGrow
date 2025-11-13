# PlantGrow - Procedural Tree Generation Tool

A biologically-inspired procedural tree generator built in C++ with USD export support.

## Project Status

**Current Phase: Phase 1 - Foundation & Basic Growth** ✓

### Completed Features
- ✓ L-System based growth simulation
- ✓ JSON configuration system
- ✓ Basic branch skeleton generation
- ✓ USD export (line-based visualization)
- ✓ Python execution wrapper

### Roadmap
- [ ] Phase 2: Tropism System (phototropism, gravitropism)
- [ ] Phase 3: Resource System & Pruning
- [ ] Phase 4: Growth Strategy Implementation
- [ ] Phase 5: Basic Meshing (Cylinders)
- [ ] Phase 6: Bark Detail & UV Generation
- [ ] Phase 7: VDB Junction Blending
- [ ] Phase 8: Foliage Instancing
- [ ] Phase 9: Species Presets & Polish
- [ ] Phase 10: Optimization & Final Polish

## Quick Start

### Prerequisites
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.15+
- Python 3.6+

### Building

```bash
# Create build directory
mkdir -p build
cd build

# Configure and build
cmake ..
cmake --build .

# Return to project root
cd ..
```

### Running

Generate a tree from a configuration file:

```bash
# Using the C++ executable directly
./build/plantgrow configs/oak.json

# Or using the Python wrapper
python3 python/tree_gen.py configs/oak.json
```

### Testing

Run the Phase 1 test suite:

```bash
python3 python/tests/test_phase1.py
```

## Configuration

Trees are defined using JSON configuration files. See `configs/oak.json` for a complete example.

### Basic Configuration Structure

```json
{
  "species": "oak",
  "growth_parameters": {
    "simulation_years": 30,
    "random_seed": 12345
  },
  "l_system": {
    "axiom": "F",
    "rules": {
      "F": "F[+F][-F]F"
    },
    "iterations": 5
  },
  "branching": {
    "base_angle_degrees": 30.0,
    "angle_variation": 10.0
  },
  "output": {
    "usd_path": "output/oak_tree.usda",
    "include_branches": true
  }
}
```

### L-System Symbols

- `F` - Draw forward (create branch segment)
- `f` - Move forward without drawing
- `+` - Rotate right
- `-` - Rotate left
- `&` - Pitch down
- `^` - Pitch up
- `[` - Push state (start new branch)
- `]` - Pop state (end branch)

## Output

Trees are exported as USD (Universal Scene Description) files compatible with:
- Houdini
- Blender (with USD support)
- Maya (with USD plugin)
- usdview (Pixar's USD viewer)
- Octane Render

### Viewing USD Files

**Using usdview (if installed):**
```bash
usdview output/oak_tree.usda
```

**Using Blender:**
1. File → Import → Universal Scene Description (.usd, .usdc, .usda)
2. Select your output file
3. Enable "Visible Purposes: Guide" to see the curves

## Project Structure

```
PlantGrow/
├── src/
│   ├── core/           # Core engine (L-system, tree structure)
│   ├── strategy/       # Growth strategies (Phase 4+)
│   ├── meshing/        # Mesh generation (Phase 5+)
│   ├── foliage/        # Foliage placement (Phase 8+)
│   ├── export/         # USD export
│   └── main.cpp        # Main entry point
├── python/
│   ├── tree_gen.py     # Python wrapper
│   └── tests/          # Test scripts for each phase
├── configs/            # JSON configuration files
├── output/             # Generated USD files
└── CMakeLists.txt      # Build configuration
```

## Development

### Phase 1 Goals
- [x] Basic project structure
- [x] JSON config parser
- [x] L-system core implementation
- [x] Simple path generation (straight branches)
- [x] Python execution wrapper
- [x] Line-based USD export

### Checkpoint Validation

To verify Phase 1 is complete:

1. Run the test suite: `python3 python/tests/test_phase1.py`
2. Open `output/oak_tree.usda` in a USD viewer
3. Verify the structure looks tree-like with branching
4. Modify `configs/oak.json` and verify changes are reflected

## Architecture

### Core Principles
- **Layered System**: Universal growth engine → Growth strategy → Species implementations
- **Biologically-Inspired**: Based on real plant growth processes (GrowFX principles)
- **Checkpoint-Driven**: Each phase has clear validation criteria

### Data Flow

```
JSON Config → ConfigParser → LSystemParams
                                    ↓
                         LSystem::generate() → L-string
                                    ↓
                         LSystem::interpret() → Tree (branches)
                                    ↓
                         USDExporter → .usda file
```

## Contributing

This project follows a phased development approach. Each phase must pass its checkpoint before moving to the next phase.

## License

[Add your license here]

## Credits

Inspired by:
- **GrowFX** - Professional plant growth system
- **Algorithmic Botany** - L-system research (Prusinkiewicz & Lindenmayer)
- **USD** - Pixar's Universal Scene Description

---

**Status:** Phase 1 Complete | Ready for Phase 2 Development
