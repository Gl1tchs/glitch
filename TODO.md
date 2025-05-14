# TODO

- [ ] Shader reflection on material creation
- [ ] ImGui viewports causing errors

- [ ] Meshes
- [x] Rewrite a more compact and extendable materials system
- [ ] Physics integration to ECS.
- [ ] Plugin system and runtime plugin loading via shared libraries.
- [ ] Ditch CMake install

## Renderer

- [ ] Blender as Editor driven developement
  - [ ] Export blender model as gltf and import scene information from custom format that
  would also been exported by a custom blender plugin.
  - [ ] Use hot-reloadable meshes and references
  - [ ] Edit scene information via debug panel


- [ ] Scene graph.
  - [ ] FBX models.
  - [ ] Scene data and camera.
  - [ ] MeshRenderComponent, MaterialComponent
  - [ ] Culling with BoundingBoxComponent
  - [ ] Instanced rendering.
  - [ ] Text rendering.
  - [ ] Deferred rendering.
- [ ] Custom render passes.
- [ ] Shader library.
- [ ] Render scale wont make program faster.
- [ ] Use present queue for presenting images
- [ ] Mipmaps
- [ ] Infinite far plane

## Graphics Programming

- [ ] Learn how to structure meshes correctly in vulkan environment such as how
      and when to create buffers and how to handle relations.
