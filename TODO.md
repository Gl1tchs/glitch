# TODO

## Bugs

- [ ] Fix immediate commands running in a separate thread causes race conditions between queue submits.

# Short Term

- [ ] Graphic Pass abstract class
```c++
class Renderer {
public:
    /* ... */

    Image create_render_image(const std::string& p_name, ...);
    Image get_render_image(const std::string& p_name);

    /* sets final imageto blit into swapchain */
    void set_swapchain_target(const std::string& p_name);

    /* ... */
};

class GraphicsPass {
public:
    virtual ~GraphicsPass() = default;

    /* find a better way */
    virtual void setup(Renderer& p_renderer) = 0;
    virtual void execute(CommandBuffer p_cmd) = 0;

    virtual const char* get_name() const = 0;
};

class PostFXPass {
public:
    void setup(Renderer& p_renderer) {
        pipeline = renderer.load_pipeline("postfx.tonemap");

        renderer.create_render_image("postfx_output", DataFormat::R16G16B16A16_SFLOAT, IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_SAMPLED);
        
        auto input = renderer.get_texture("hdr_scene_color");
        /* descset upload input */
    }
    
    void execute(CommandBuffer p_cmd) {
        auto output = renderer.get_texture("postfx_output");

        command_begin_rendering(p_cmd, output);

        command_end_rendering(p_cmd);
    };

private:
    Ref<Renderer> renderer;
    Pipeline pipeline;
};
```

- [ ] GIS Terrain loading + LOD with quadtrees
- [ ] Renderer recreation
- [ ] Animation
- [ ] Instanced rendering
- [ ] Light objects
- [ ] Post processing
- [ ] Shadows
- [ ] Transparent materials
- [ ] Cubemaps
- [ ] GPU Particle system
- [ ] Physics engine for ballistics and destructible environments
- [ ] Multi-Camera modes (satellite/drone/infantry) with recon overlays
- [ ] Tactical UI overlay with unit commands and targeting


## Long Term

- [ ] Global illumination
- [ ] Deferred rendering G Buffers.
- [ ] Text rendering.
- [ ] LODs
