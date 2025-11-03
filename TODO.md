# TODO

- [ ] Headless build creation.
- [ ] Shadow maps
  - [ ] Light goes inside walls
- [ ] Cubemaps and IBL
- [ ] Instanced rendering
- [ ] Parallax mapping
- [ ] Per Frame Allocators
- [ ] Async GLTF model loading progress

## Serialization

- [ ] Custom INI parser for configuration files.
  - [ ] Array support
- [ ] Custom scene description language
- [ ] Implementing an asset system.

## Asset System

- [ ] Asset loading and asset handles
- [ ] Custom asset to engine type conversion and asset caching

## Linux

- [ ] Try out Fil-C and make it build 

## Editor

- [ ] Changing transform while scripts are running are not working for meshes (or parents).

## Scripting

- [ ] Assert should only affect scripts
- [ ] Global error handling (errno like functionality)
- [ ] Some more functions
  - [ ] void Transform_LookAt(Transform* transform, Vec3 point)
  - [ ] Timer type
  - [ ] Mesh API
  - [ ] Compute API
- [ ] Maybe all scripts need to be loaded before starting to run more efficiently

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
