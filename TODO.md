# TODO

- [ ] Serialization for specifications like MSAA
- [ ] ECS
  - [ ] Material component

- [ ] Shadow maps
  - [ ] Light goes inside walls
- [ ] Cubemaps and IBL
- [ ] Instanced rendering
- [ ] Parallax mapping
- [ ] Per Frame Allocators
- [ ] Async GLTF model loading progress

## Scripting

- [ ] Proper script execution system inside of the scene
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
- [ ] Scripting

## Long Term

- [ ] Global illumination
- [ ] Deferred rendering G Buffers.
- [ ] Text rendering.
- [ ] LODs
