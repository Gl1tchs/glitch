# Glitch — Yet Another Rendering Engine & Application Framework

Glitch is a modular, modern C++ rendering engine built primarily on Vulkan, designed for real-time graphics, game development
and interactive simulations.

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
