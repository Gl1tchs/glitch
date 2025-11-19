# Glitch — Yet Another Rendering Engine & Application Framework

Glitch is a modular, modern C++ rendering engine built primarily on Vulkan, designed for real-time graphics, game development
and interactive simulations.

## Building

### Dependencies

You’ll need the following:

- [Vulkan SDK](https://www.lunarg.com/vulkan-sdk)
- C++20 compiler
- `cmake` ≥ 3.20
- `ninja`

### Cloning and Building

Glitch uses CMakeFetch content to retrieve some third party dependencies. So this may take a while.

```bash
git clone https://github.com/Gl1tchs/glitch.git
cd glitch
cmake --preset release
cmake --build --preset build-release
```
