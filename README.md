# Glitch

Glitch is an experimental rendering engine with a goal to be used for games
and simulations.

## Building

In order to build the engine you need to have the dependencies installed on your
system:
- [Python](https://www.python.org/): build system automation.
- [Clang](https://clang.llvm.org/): C/C++ compiler.
- [GLSLC](https://github.com/google/shaderc): shader compiler.
- [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/): vulkan backend.

**Clonning and building:**
```
git clone https://github.com/Gl1tchs/glitch.git \
cd glitch \
python build.py build
```
**And to run it:**
```
python build.py testbed
```
