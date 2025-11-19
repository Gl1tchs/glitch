# TODO

## Critical Bugs

- [ ] ImGui Popups (AddComponent) are not working and cause VkDeviceDestroy problems during shut-down 

## Core

- [ ] Per Frame Allocators
- [ ] Headless build creation.

## Asset System

- [ ] Proper project system for environment variables
- [ ] Script as asset:
    - [ ] ScriptMetadata { fields }
- [ ] Asset Metadatas
    - [ ] e.g. GLTFSourceMetadata { model_path, loading_options  }
    - [x] Texture metadata
- [ ] Make AssetSystem thread safe
- [ ] Execute AssetSystem::collect_garbage in a good place in the engine
- [ ] Async GLTF model loading progress
- [ ] Async asset (de)serialization

## Editor

- [ ] Material texture inspector
- [ ] Content browser

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

## Bugs

- [ ] ImGui viewports are not working.
- [ ] Swapchain resize gives descriptor set = 0 error`
- [ ] Fix immediate commands running in a separate thread causes race conditions between queue submits.
- [ ] Renderer::create_render_image expects same format as swapchain for color attachments

