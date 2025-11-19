# TODO

## Critical Bugs

## Core

- [ ] Per Frame Allocators
- [ ] Headless build creation.

## Asset System

- [ ] Proper project system for environment variables
- [ ] GLTF Metadata
- [ ] Make AssetSystem thread safe
- [ ] Execute AssetSystem::collect_garbage in a good place in the engine
- [ ] Async GLTF model loading progress
- [ ] Async asset (de)serialization

## Editor

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

- [ ] Fix immediate commands running in a separate thread causes race conditions between queue submits.
- [ ] Renderer::create_render_image expects same format as swapchain for color attachments

## Refactors

- [ ] JSON Serialization should ensure type safety and should return json not take reference
- [ ] All Errors should be handled