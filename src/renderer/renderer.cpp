#include "renderer/renderer.h"

#include "core/application.h"

#include "renderer/camera.h"
#include "renderer/mesh.h"
#include "renderer/node.h"
#include "renderer/types.h"

#include "platform/vulkan/vk_backend.h"

#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <imgui.h>

#ifndef IMGUI_BUILD_FOR_GLFW
#define IMGUI_BUILD_FOR_GLFW
#include <backends/imgui_impl_glfw.cpp>
#endif

[[nodiscard]] GraphicsAPI find_proper_api() noexcept {
	return GRAPHICS_API_VULKAN;
}

static GraphicsAPI s_api;

Renderer* Renderer::s_instance = nullptr;

Renderer::Renderer(Ref<Window> p_window) : window(p_window) {
	GL_ASSERT(
			s_instance == nullptr, "Only one instance of renderer can exists!");
	s_instance = this;

	s_api = find_proper_api();

	backend = create_ref<VulkanRenderBackend>();
	backend->init(p_window);

	swapchain = backend->swapchain_create();
	backend->swapchain_resize(backend->queue_get(QUEUE_TYPE_PRESENT), swapchain,
			p_window->get_size());

	draw_image = backend->image_create(DATA_FORMAT_R16G16B16A16_SFLOAT,
			p_window->get_size(), nullptr,
			IMAGE_USAGE_TRANSFER_SRC_BIT | IMAGE_USAGE_TRANSFER_DST_BIT |
					IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

	depth_image =
			backend->image_create(DATA_FORMAT_D32_SFLOAT, p_window->get_size(),
					nullptr, IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

	for (uint8_t i = 0; i < SWAPCHAIN_BUFFER_SIZE; i++) {
		FrameData& frame_data = frames[i];

		frame_data.command_pool = backend->command_pool_create(
				backend->queue_get(QUEUE_TYPE_GRAPHICS));
		frame_data.command_buffer =
				backend->command_pool_allocate(frame_data.command_pool);

		frame_data.image_available_semaphore = backend->semaphore_create();
		frame_data.render_finished_semaphore = backend->semaphore_create();

		frame_data.render_fence = backend->fence_create();
	}

	// initialize imgui context
	_imgui_init();

	// initialize default data
	default_material = Material::create();

	constexpr uint32_t gray_color = 0x808080;

	default_image = backend->image_create(
			DATA_FORMAT_R8G8B8A8_UNORM, { 1, 1 }, &gray_color);
	default_sampler = backend->sampler_create();

	MaterialResources resources = {};
	resources.color_image = default_image;
	resources.sampler = default_sampler;

	default_material_instance = default_material->create_instance(resources);
}

Renderer::~Renderer() {
	backend->device_wait();

	Material::destroy(default_material);

	backend->image_free(default_image);
	backend->sampler_free(default_sampler);

	_destroy_scene_graph();

	for (uint8_t i = 0; i < SWAPCHAIN_BUFFER_SIZE; i++) {
		FrameData& frame_data = frames[i];

		frame_data.deletion_queue.flush();

		backend->command_pool_free(frame_data.command_pool);

		backend->semaphore_free(frame_data.image_available_semaphore);
		backend->semaphore_free(frame_data.render_finished_semaphore);

		backend->fence_free(frame_data.render_fence);
	}

	backend->image_free(depth_image);
	backend->image_free(draw_image);

	backend->swapchain_free(swapchain);

	// destroy imgui
	ImGui_ImplGlfw_Shutdown();

	backend->shutdown();
}

void Renderer::wait_and_render() {
	_reset_stats();

	backend->fence_wait(_get_current_frame().render_fence);

	_get_current_frame().deletion_queue.flush();

	Optional<Image> swapchain_image = backend->swapchain_acquire_image(
			swapchain, _get_current_frame().image_available_semaphore);
	if (!swapchain_image.has_value()) {
		_request_resize();
		return;
	}

	backend->fence_reset(_get_current_frame().render_fence);

	const Vec2u swapchain_extent = backend->swapchain_get_extent(swapchain);
	const Vec3u draw_image_extent = backend->image_get_size(draw_image);

	// set render scale
	const float render_scale = get_settings().render_scale;
	draw_extent = Vec2u(
			std::min(swapchain_extent.x, draw_image_extent.x) * render_scale,
			std::min(swapchain_extent.y, draw_image_extent.y) * render_scale);

	CommandBuffer cmd = _get_current_frame().command_buffer;

	backend->command_reset(cmd);

	backend->command_begin(cmd);
	{
		backend->command_transition_image(
				cmd, draw_image, IMAGE_LAYOUT_UNDEFINED, IMAGE_LAYOUT_GENERAL);

		backend->command_clear_color(
				cmd, draw_image, { 0.1f, 0.1f, 0.1f, 1.0f });

		backend->command_transition_image(cmd, draw_image, IMAGE_LAYOUT_GENERAL,
				IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		backend->command_transition_image(cmd, depth_image,
				IMAGE_LAYOUT_UNDEFINED, IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

		_geometry_pass(cmd);

		backend->command_transition_image(cmd, draw_image,
				IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		backend->command_transition_image(cmd, swapchain_image.value(),
				IMAGE_LAYOUT_UNDEFINED, IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		backend->command_copy_image_to_image(cmd, draw_image,
				swapchain_image.value(), draw_extent, swapchain_extent);

		if (imgui_being_used) {
			backend->command_transition_image(cmd, swapchain_image.value(),
					IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

			_imgui_pass(cmd, swapchain_image.value());
		}

		backend->command_transition_image(cmd, swapchain_image.value(),
				imgui_being_used ? IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
								 : IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				IMAGE_LAYOUT_PRESENT_SRC);
	}
	backend->command_end(cmd);

	backend->queue_submit(backend->queue_get(QUEUE_TYPE_GRAPHICS), cmd,
			_get_current_frame().render_fence,
			_get_current_frame().image_available_semaphore,
			_get_current_frame().render_finished_semaphore);

	if (!backend->queue_present(backend->queue_get(QUEUE_TYPE_GRAPHICS),
				swapchain, _get_current_frame().render_finished_semaphore)) {
		_request_resize();
	}

	// reset the state
	imgui_being_used = false;

	frame_number++;
}

void Renderer::wait_for_device() { backend->device_wait(); }

void Renderer::submit(RenderState p_state, RenderFunc p_function) {
	submit_funcs[p_state].push_back(p_function);
}

void Renderer::imgui_begin() {
	imgui_being_used = true;

	backend->imgui_new_frame_for_platform();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void Renderer::imgui_end() { ImGui::Render(); }

void Renderer::_geometry_pass(CommandBuffer p_cmd) {
	backend->command_begin_rendering(
			p_cmd, draw_extent, draw_image, depth_image);
	{
		// scene data
		Buffer scene_data_buffer = backend->buffer_create(sizeof(GPUSceneData),
				BUFFER_USAGE_UNIFORM_BUFFER_BIT | BUFFER_USAGE_TRANSFER_SRC_BIT,
				MEMORY_ALLOCATION_TYPE_CPU);

		_get_current_frame().deletion_queue.push_function(
				[=, this]() { backend->buffer_free(scene_data_buffer); });

		GPUSceneData* scene_uniform_data =
				(GPUSceneData*)backend->buffer_map(scene_data_buffer);
		{
			get_scene_graph().traverse<CameraNode>([scene_uniform_data](
														   CameraNode* camera) {
				scene_uniform_data->camera_pos =
						glm::vec4(camera->transform.get_position(), 1.0f);
				scene_uniform_data->view = camera->get_view_matrix();
				scene_uniform_data->proj = camera->get_projection_matrix();
				scene_uniform_data->view_proj =
						scene_uniform_data->proj * scene_uniform_data->view;
				scene_uniform_data->sun_direction = { -1, -1, -1 };
				scene_uniform_data->sun_power = 1.0f;
				scene_uniform_data->sun_color = { 1, 1, 1, 1 };
				return true;
			});
		}
		backend->buffer_unmap(scene_data_buffer);

		// start rendering
		static const auto get_proper_material =
				[this](Ref<MaterialInstance> material)
				-> Ref<MaterialInstance> {
			return !material
					? default_material_instance
					: std::dynamic_pointer_cast<MaterialInstance>(material);
		};

		std::map<Ref<MaterialInstance>, std::vector<Mesh*>> mesh_map;
		get_scene_graph().traverse<Mesh>([&](Mesh* mesh) {
			mesh_map[get_proper_material(mesh->material)].push_back(mesh);
			return false;
		});

		UniformSet scene_data_set = GL_NULL_HANDLE;
		_get_current_frame().deletion_queue.push_function(
				[=, this]() { backend->uniform_set_free(scene_data_set); });

		bool global_descriptor_set = false;

		for (const auto& [material, meshes] : mesh_map) {
			if (meshes.empty()) {
				continue;
			}

			if (!global_descriptor_set) {
				std::vector<BoundUniform> scene_data_uniforms(1);
				scene_data_uniforms[0].binding = 0;
				scene_data_uniforms[0].type = UNIFORM_TYPE_UNIFORM_BUFFER;
				scene_data_uniforms[0].ids.push_back(scene_data_buffer);

				scene_data_set = backend->uniform_set_create(
						scene_data_uniforms, material->shader, 0);

				global_descriptor_set = true;
			}

			backend->command_bind_graphics_pipeline(p_cmd, material->pipeline);

			// set dynamic state
			backend->command_set_viewport(
					p_cmd, { (float)draw_extent.x, (float)draw_extent.y });
			backend->command_set_scissor(p_cmd, draw_extent);

			std::vector<UniformSet> descriptors = {
				scene_data_set,
				material->uniform_set,
			};
			backend->command_bind_uniform_sets(
					p_cmd, material->shader, 0, descriptors);

			for (const auto& mesh : meshes) {
				backend->command_bind_index_buffer(
						p_cmd, mesh->index_buffer, 0, mesh->index_type);

				GPUDrawPushConstants push_constants = {
					.transform = mesh->transform.get_transform_matrix(),
					.vertex_buffer = mesh->vertex_buffer_address,
				};

				backend->command_push_constants(p_cmd, material->shader, 0,
						sizeof(GPUDrawPushConstants), &push_constants);

				backend->command_draw_indexed(p_cmd, mesh->index_count);

				stats.draw_calls++;
				stats.triangle_count = mesh->index_count / 3;
			}
		}
	}

	if (const auto it = submit_funcs.find(RENDER_STATE_GEOMETRY);
			it != submit_funcs.end()) {
		for (const auto& submit : it->second) {
			submit(backend, p_cmd, _get_current_frame().deletion_queue);
		}
		submit_funcs[RENDER_STATE_GEOMETRY].clear();
	}

	backend->command_end_rendering(p_cmd);
}

void Renderer::_imgui_pass(CommandBuffer p_cmd, Image p_target_image) {
	backend->command_begin_rendering(
			p_cmd, backend->swapchain_get_extent(swapchain), p_target_image);
	{ backend->imgui_render_for_platform(p_cmd); }
	backend->command_end_rendering(p_cmd);
}

void Renderer::_imgui_init() {
	ImGui::CreateContext();
	ImGui_ImplGlfw_Init(window->get_native_window(), true,
			s_api == GRAPHICS_API_VULKAN ? GlfwClientApi_Vulkan
										 : GlfwClientApi_Unknown);
	backend->imgui_init_for_platform();
}

void Renderer::_request_resize() {
	Application::get_instance()->enqueue_main_thread([&]() {
		backend->swapchain_resize(backend->queue_get(QUEUE_TYPE_PRESENT),
				swapchain, window->get_size());
	});
}

void Renderer::_reset_stats() { memset(&stats, 0, sizeof(RenderStats)); }

void Renderer::_destroy_scene_graph() {
	scene_graph.traverse([this](Node* node) {
		switch (node->get_type()) {
			case NodeType::NONE: {
				break;
			}
			case NodeType::GEOMETRY: {
				Mesh::destroy((const Mesh*)node);
				break;
			}
			case NodeType::COMPUTE: {
				break;
			}
			case NodeType::CAMERA: {
				break;
			}
			case NodeType::LIGHT: {
				break;
			}
		}

		return false;
	});
}
