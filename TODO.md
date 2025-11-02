# TODO

- [ ] Serialization for specifications like MSAA
- [ ] Shadow maps
  - [ ] Light goes inside walls
- [ ] Cubemaps and IBL
- [ ] Instanced rendering
- [ ] Parallax mapping
- [ ] Per Frame Allocators
- [ ] Async GLTF model loading progress

## Scripting

- [ ] Some more functions
  - [ ] const char* Entity_GetName(uint32_t entity)
  - [ ] uint32_t Entity_GetParent(uint32_t entity)
  - [ ] void Entity_SetParent(uint32_t entity, uint32_t parent)
  - [ ] void Entity_FindChildById(uint32_t entity, uint32_t id)
  - [ ] void Entity_FindChildByName(uint32_t entity, const char* name)
  - [ ] void Entity_Destroy(uint32_t entity)
  - [ ] uint32_t Entity_Create()
  - [ ] uint32_t Entity_CreateNamed(const char* name)
  - [ ] bool Entity_IsValid(uint32_t entity)
  - [ ] bool Transform_Rotate(Transform* transform, Vec3 rotation)
  - [ ] Vec3 Transform_GetForward(Transform* transform)
  - [ ] Vec3 Transform_GetRight(Transform* transform)
  - [ ] Vec3 Transform_GetUp(Transform* transform)
  - [ ] void Transform_LookAt(Transform* transform, Vec3 point)
  - [ ] void Window_SetTitle(const char* title)
  - [ ] void Window_SetCursorMode(WindwoCursorMode mode)
  - [ ] Vec2 Window_GetSize()
  - [ ] void Window_SetSize(Vec2 size)
  - [ ] Timer type
  - [ ] Mesh API
- [ ] Maybe all scripts need to be loaded before starting to run more efficiently
- [ ] LuaJIT linux build

## Bugs

- [ ] Swapchain resize gives descriptor set = 0 error
- [ ] Can't close with Alt-F4 or by window when materials active
- [ ] Vulkan resource deinitialization issua that i couldn't find where
- [ ] Fix immediate commands running in a separate thread causes race conditions between queue submits.
- [ ] Renderer::create_render_image expects same format as swapchain for color attachments

# Short Term

- [ ] Renderer recreation
- [ ] Animation
- [ ] Post processing
- [ ] Transparent materials
- [ ] Asset system and serialization
- [ ] GPU Particle system
- [ ] Physics engine

## Long Term

- [ ] Global illumination
- [ ] Deferred rendering G Buffers.
- [ ] Text rendering.
- [ ] LODs
