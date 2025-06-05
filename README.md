# 🚀 Glitch — Rendering Engine & Application Framework

Glitch is a modular, modern C++ rendering engine built primarily around Vulkan. It's designed for **real-time graphics**, **game development**, and **interactive simulations**.

---

## 🧠 Features (WIP)

- 🖼️ Vulkan-based renderer with modern GPU pipeline
- 🧱 Scene graph system for spatial hierarchy
- 📦 Mesh, material, and texture abstraction
- 🧠 GLTF 2.0 model loading (via `tinygltf`)
- 🔧 Modular rendering backend system

---

## ⚙️ Building

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

---

## 🛠️ Usage & Sandbox

Looking for examples? Check out the [testbed](testbed/), where Glitch is tested and extended with real-time experiments.

---

## 🛣️ Roadmap / What's Next?

Here’s the current plan for Glitch, split by priority:

### 🔜 Short Term

- [x] Scene graph traversal with transform propagation
- [x] Basic mesh/material rendering
- [x] Camera + transform control
- [x] GLTF loader
- [ ] Bounding volume hierarchy for frustum culling
- [ ] Material sorting and draw call batching
- [ ] Directional lighting with shadows (PCF)
- [ ] Instanced rendering
- [ ] IBL support (diffuse/specular probes)
- [ ] Forward+ lighting with clustered culling
- [ ] Particle system (GPU-based)

### 🧠 Long Term

- [ ] Decals (deferred or forward projection)
- [ ] Post-processing framework (FXAA, bloom, SSAO)
- [ ] GPU profiling / debug UI
- [ ] Hot-reloadable shaders
- [ ] Multi-threaded render graph
