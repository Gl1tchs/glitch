# Glitch | A rendering engine and application framework

Glitch is an experimental rendering engine with a goal to be used for games
and simulations. It currently only supports Vulkan backend but others can
be integrated by deriving RenderBackend class.

## Building

In order to build the engine you need to have the dependencies installed on your
system:
- [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/): vulkan backend.

**Clonning and building:**
```
git clone --recursive https://github.com/Gl1tchs/glitch.git \
    cd glitch \
    make 
```

## Examples

Example of usage can be found [here](https://github.com/Gl1tchs/glitch-sandbox),
a sandbox project of mine which i use to learn graphics programming.

## Installation

After building the project in release mode you can just run `sudo ./build.py install`
to install the project to your system. And then using `FindGlitch.cmake` file under
cmake/ directory you can use the library from you CMake project by doing so:

```cmake
list(APPEND CMAKE_MODULE_PATH "/path/to/FindGlitch.cmake")

find_package(Glitch REQUIRED)

target_include_directories(your_target PUBLIC ${GLITCH_INCLUDE_DIRS})
target_link_libraries(your_target PRIVATE ${GLITCH_LIBRARIES})
```
