# TODOS

## Renderer
- [ ] Implement more detailed scene graph system with components (proper ECS)
- [ ] ID based resource management for Material, Image and etc. so that we
don't need to worry about deletion of the objects.
```cpp
struct VulkanContext {
    // ...
    std::vector<VulkanImage> allocated_images;
    std::vector<VulkanMaterial> allocated_materials;
    // ...
};
```
- [ ] Get rid of material instances and bind textures as a list to the shaders
and use texture indexing.
```glsl
layout(buffer_reference, std430) readonly buffer TextureBuffer {
    sampler2D textures[];
}
```

## Vulkan Renderer
- [ ] Font rendering
