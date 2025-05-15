# TODO

- [ ] ImGui viewports causing errors

- [ ] Scene resource management for destructors
- [x] Handle cameras using ECS
  - [ ] Better way to send data to gpu
- [x] Meshes
  - [x] Materials
  - [ ] Use a buffer to transfer data

## Renderer

- [ ] Blender as Editor driven developement

  - [ ] Export blender model as gltf and import scene information from custom format that
        would also been exported by a custom blender plugin.
  - [ ] Use hot-reloadable meshes and references
  - [ ] Edit scene information via debug panel

- [ ] Scene graph.
  - [ ] Culling with BoundingBoxComponent
  - [ ] Instanced rendering.
  - [ ] Text rendering.
  - [ ] Deferred rendering.
- [ ] Custom render passes.
- [ ] Render scale wont make program faster.
- [ ] Mipmaps
- [ ] Infinite far plane

## Graphics Programming

- [ ] Learn how to structure meshes correctly in vulkan environment such as how
      and when to create buffers and how to handle relations.
