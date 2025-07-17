# Glitch — Tactical Simulation Rendering Engine & Framework

Glitch is a modular, modern C++ rendering engine built primarily on Vulkan, designed for real-time graphics, game development,
interactive simulations, and now advanced tactical / military-grade applications.

## Features (WIP)

- Vulkan-based renderer with modern GPU pipeline
- Scene graph system for spatial hierarchy
- Mesh, material, and texture abstraction
- GIS/terrain integration with heightmaps & geospatial referencing (work in progress)
- Destructible environment support (planned voxel & mesh fracture) (work in progress)
- Physics integration for ballistics, rockets, and vehicle simulation (work in progress)
- Multi-layered camera systems: drone/satellite views + recon overlays (work in progress)
- Modular rendering backend with future-proof support for compute shaders & advanced post-processing

---

## Building

### 🔗 Dependencies

You’ll need the following:

- [Vulkan SDK](https://www.lunarg.com/vulkan-sdk)
- C++20 compiler
- `cmake` ≥ 3.20
- `make` or `ninja`

### 🧬 Cloning and Building

```bash
git clone --recursive https://github.com/Gl1tchs/glitch.git
cd glitch
make
```

Or with CMake directly:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```
