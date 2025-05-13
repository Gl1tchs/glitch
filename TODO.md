# TODO

- [ ] Make get_bundled_spirv_data into a ShaderLibrary class
- [x] Own math classes for linalg
    - [ ] Mat3 class
    - [ ] SIMD Matrix multiplication
    - [ ] Mat4 inverse
    - [ ] SPIRV Dot product

- [ ] Physics integration to ECS.
- [ ] Plugin system and runtime plugin loading via shared libraries.

## Renderer
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
