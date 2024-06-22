#pragma once

#include "core/deletion_queue.h"
#include "core/window.h"

#include "renderer/material.h"
#include "renderer/types.h"

class RenderBackend;

enum GraphicsAPI {
	GRAPHICS_API_VULKAN,
};

[[nodiscard]] GraphicsAPI find_proper_api() noexcept;

struct GPUSceneData {
	glm::vec4 camera_pos;
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 view_proj;
	glm::vec3 sun_direction;
	// in glsl this will be the w component
	// of the `sun_direction`
	float sun_power;
	glm::vec4 sun_color;
};

struct GPUDrawPushConstants {
	glm::mat4 transform;
	BufferDeviceAddress vertex_buffer;
};

struct RendererSettings {
	float render_scale = 1.0f;
	// bool msaa;
};

struct RenderStats {
	uint32_t draw_calls;
	uint32_t triangle_count;
};

struct FrameData {
	CommandPool command_pool;
	CommandBuffer command_buffer;

	Semaphore image_available_semaphore, render_finished_semaphore;
	Fence render_fence;

	DeletionQueue deletion_queue;
};

enum RenderState {
	RENDER_STATE_CLEAR,
	RENDER_STATE_GEOMETRY,
};

typedef std::function<void(Ref<RenderBackend> p_backend, CommandBuffer p_cmd,
		Image p_draw_image, DeletionQueue& frame_deletion)>
		RenderFunc;

class SceneGraph;

class Renderer {
public:
	Renderer(Ref<Window> p_window);
	~Renderer();

	/**
	 * @brief wait for the previous frame to be rendered and presented
	 * and then render the current frame.
	 */
	void wait_and_render();

	/**
	 * @brief wait for rendering device operations to finish
	 */
	void wait_for_device();

	/**
	 * @brief submits function to run at specified pass
	 */
	void submit(RenderState p_state, RenderFunc p_function);

	/**
	 * @brief begin imgui rendering context, all imgui functions
	 * must be runned inside of this scope and this operation is
	 * defined as 1 imgui frame.
	 */
	void imgui_begin();

	/**
	 * @brief ends imgui rendering context
	 */
	void imgui_end();

	void set_scene(SceneGraph* p_scene_graph) { scene_graph = p_scene_graph; }

	RendererSettings& get_settings() { return settings; }

	RenderStats& get_stats() { return stats; }

	static DataFormat get_draw_image_format() {
		return s_instance->draw_image_format;
	}

	static DataFormat get_depth_image_format() {
		return s_instance->depth_image_format;
	}

	static Image get_default_image() { return s_instance->default_image; }
	static Sampler get_default_sampler() { return s_instance->default_sampler; }

	static Ref<RenderBackend> get_backend() { return s_instance->backend; }

private:
	void _geometry_pass(CommandBuffer p_cmd);

	void _imgui_pass(CommandBuffer p_cmd, Image p_target_image);

private:
	void _imgui_init();

	void _request_resize();

	void _reset_stats();

	inline FrameData& _get_current_frame() {
		return frames[frame_number % SWAPCHAIN_BUFFER_SIZE];
	};

private:
	static Renderer* s_instance;

	Ref<Window> window;

	SceneGraph* scene_graph = nullptr;

	// drawing data
	Ref<RenderBackend> backend;

	CommandQueue graphics_queue;
	CommandQueue present_queue;

	Swapchain swapchain;

	Image draw_image;
	const DataFormat draw_image_format = DATA_FORMAT_R16G16B16A16_SFLOAT;

	Image depth_image;
	const DataFormat depth_image_format = DATA_FORMAT_D32_SFLOAT;

	Vec2u draw_extent;

	static constexpr uint8_t SWAPCHAIN_BUFFER_SIZE = 2;

	FrameData frames[SWAPCHAIN_BUFFER_SIZE];
	uint32_t frame_number = 0;

	std::unordered_map<RenderState, std::vector<RenderFunc>> submit_funcs;

	// imgui data
	bool imgui_being_used = false;

	// default data
	Image default_image;
	Sampler default_sampler;

	Ref<Material> default_material;
	Ref<MaterialInstance> default_material_instance;

	// misc
	RendererSettings settings = {};
	RenderStats stats = {};
};
