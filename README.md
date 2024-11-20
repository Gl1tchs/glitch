# Glitch | A rendering engine and application framework

Glitch is an experimental rendering engine with a goal to be used for games
and simulations. It currently only supports Vulkan backend but others can
be integrated by deriving RenderBackend class.

## Building

In order to build the engine you need to have the dependencies installed on your
system:
- [Python](https://www.python.org/): build system automation.
- [Clang](https://clang.llvm.org/): C/C++ compiler.
- [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/): vulkan backend.

**Clonning and building:**
```
git clone --recursive https://github.com/Gl1tchs/glitch.git \
    cd glitch \
    python build.py build
```

## Usage

After building the engine, script automatically exposes the required headers
and a shared library in bin/ directory. An example of use could be found
[here](https://github.com/Gl1tchs/glitch-sandbox).
