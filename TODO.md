# TODO

## Core

- [ ] Event System use std::move_only_function
- [ ] Per Frame Allocators
- [ ] Headless build creation.

## Serialization

- [ ] Custom flat INI parser for configuration files with array support.
- [ ] Custom scene description language
- [ ] Implementing an asset system.

## Asset System

- [ ] Asset Metadata and serialization
- [ ] Make AssetSystem thread safe
- [ ] More asset definitions
- [ ] Execute AssetSystem::collect_garbage in a good place in the engine
- [ ] Async GLTF model loading progress

## Editor


## Scripting

- [ ] Global error handling (errno like functionality)
- [ ] Some more functions
  - [ ] Timer type
  - [ ] Mesh API
  - [ ] Compute API
- [ ] Maybe all scripts need to be loaded before starting to run more efficiently

## Renderer

- [ ] Shadow maps
  - [ ] Light goes inside walls
- [ ] Cubemaps and IBL
- [ ] Instanced rendering
- [ ] Parallax mapping
- [ ] Renderer recreation
- [ ] Animation
- [ ] Post processing
- [ ] Transparent materials
- [ ] GPU Particle system
- [ ] Global illumination
- [ ] Deferred rendering G Buffers.
- [ ] Text rendering.
- [ ] LODs

## Physics

- [ ] Physics engine

## Linux

- [ ] Test test platform utilities

## Bugs

- [ ] Swapchain resize gives descriptor set = 0 error
- [ ] Fix immediate commands running in a separate thread causes race conditions between queue submits.
- [ ] Renderer::create_render_image expects same format as swapchain for color attachments

