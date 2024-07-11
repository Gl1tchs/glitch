#include "renderer/renderer.h"

#include "core/application.h"

#include "renderer/camera.h"
#include "renderer/mesh.h"
#include "renderer/types.h"

#include "scene/components.h"
#include "scene/scene.h"
#include "scene/view.h"

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
	switch (s_api) {
		case GRAPHICS_API_VULKAN:
			backend = create_ref<VulkanRenderBackend>();
			break;
		default:
			GL_ASSERT(false, "Selected graphics API not implemented.");
			break;
	}

	backend->init(p_window);

	graphics_queue = backend->queue_get(QUEUE_TYPE_GRAPHICS);
	present_queue = backend->queue_get(QUEUE_TYPE_PRESENT);

	swapchain = backend->swapchain_create();
	backend->swapchain_resize(graphics_queue, swapchain, p_window->get_size());

	draw_image = backend->image_create(draw_image_format, p_window->get_size(),
			nullptr,
			IMAGE_USAGE_TRANSFER_SRC_BIT | IMAGE_USAGE_TRANSFER_DST_BIT |
					IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

	depth_image =
			backend->image_create(depth_image_format, p_window->get_size(),
					nullptr, IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

	for (uint8_t i = 0; i < SWAPCHAIN_BUFFER_SIZE; i++) {
		FrameData& frame_data = frames[i];

		frame_data.command_pool = backend->command_pool_create(graphics_queue);
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

	constexpr uint32_t white_color = 0xffffffff;

	default_image = backend->image_create(
			DATA_FORMAT_R8G8B8A8_UNORM, { 1, 1 }, &white_color);
	default_sampler = backend->sampler_create();

	MaterialResources resources = {};
	resources.diffuse_image = default_image;
	resources.sampler = default_sampler;

	default_material_instance = default_material->create_instance(resources);
}

Renderer::~Renderer() {
	backend->device_wait();

	// destroy geometry pipeline resources
	backend->image_free(draw_image);
	backend->image_free(depth_image);

	// destroy default data
	Material::destroy(default_material);
	backend->image_free(default_image);
	backend->sampler_free(default_sampler);

	// destroy per-frame data
	for (uint8_t i = 0; i < SWAPCHAIN_BUFFER_SIZE; i++) {
		FrameData& frame_data = frames[i];

		frame_data.deletion_queue.flush();

		backend->command_pool_free(frame_data.command_pool);

		backend->semaphore_free(frame_data.image_available_semaphore);
		backend->semaphore_free(frame_data.render_finished_semaphore);

		backend->fence_free(frame_data.render_fence);
	}

	// swapchain cleanup
	backend->swapchain_free(swapchain);

	// destroy imgui
	ImGui_ImplGlfw_Shutdown();

	backend->shutdown();
}

void Renderer::wait_and_render() {
	if (!scene) {
		GL_LOG_WARNING("No scene graph found to render skipping!");
		return;
	}

	_reset_stats();

	backend->fence_wait(_get_current_frame().render_fence);

	_get_current_frame().deletion_queue.flush();

	Optional<Image> swapchain_image = backend->swapchain_acquire_image(
			swapchain, _get_current_frame().image_available_semaphore);
	if (!swapchain_image) {
		_request_resize();
		return;
	}

	backend->fence_reset(_get_current_frame().render_fence);

	const Vec2u swapchain_extent = backend->swapchain_get_extent(swapchain);
	const Vec3u draw_image_extent = backend->image_get_size(draw_image);

	// set render scale
	const float render_scale = get_settings().render_scale;
	draw_extent = {
		std::min(swapchain_extent.x, draw_image_extent.x) * render_scale,
		std::min(swapchain_extent.y, draw_image_extent.y) * render_scale,
	};

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
		backend->command_transition_image(cmd, *swapchain_image,
				IMAGE_LAYOUT_UNDEFINED, IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		backend->command_copy_image_to_image(cmd, draw_image, *swapchain_image,
				draw_extent, swapchain_extent);

		if (imgui_being_used) {
			backend->command_transition_image(cmd, *swapchain_image,
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

	backend->queue_submit(graphics_queue, cmd,
			_get_current_frame().render_fence,
			_get_current_frame().image_available_semaphore,
			_get_current_frame().render_finished_semaphore);

	if (!backend->queue_present(graphics_queue, swapchain,
				_get_current_frame().render_finished_semaphore)) {
		_request_resize();
	}

	// reset the state
	imgui_being_used = false;

	frame_number++;
}

void Renderer::wait_for_device() { backend->device_wait(); }

void Renderer::submit(RenderFunc&& p_function) {
	submit_funcs.push_back(std::move(p_function));
}

void Renderer::imgui_begin() {
	imgui_being_used = true;

	backend->imgui_new_frame_for_platform();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void Renderer::imgui_end() { ImGui::Render(); }

static bool _is_mesh_visible(const Transform& transform, const Ref<Mesh> mesh,
		const glm::mat4& viewproj) {
	constexpr std::array<glm::vec3, 8> corners{
		glm::vec3{ 1.0f, 1.0f, 1.0f },
		glm::vec3{ 1.0f, 1.0f, -1.0f },
		glm::vec3{ 1.0f, -1.0f, 1.0f },
		glm::vec3{ 1.0f, -1.0f, -1.0f },
		glm::vec3{ -1.0f, 1.0f, 1.0f },
		glm::vec3{ -1.0f, 1.0f, -1.0f },
		glm::vec3{ -1.0f, -1.0f, 1.0f },
		glm::vec3{ -1.0f, -1.0f, -1.0f },
	};

	glm::mat4 matrix = viewproj * transform.get_transform_matrix();

	glm::vec3 min = { 1.5, 1.5, 1.5 };
	glm::vec3 max = { -1.5, -1.5, -1.5 };

	for (int c = 0; c < 8; c++) {
		// project each corner into clip space
		glm::vec4 v = matrix *
				glm::vec4(mesh->bounds.origin +
								(corners[c] * mesh->bounds.extents),
						1.f);

		// perspective correction
		v.x = v.x / v.w;
		v.y = v.y / v.w;
		v.z = v.z / v.w;

		min = glm::min(glm::vec3{ v.x, v.y, v.z }, min);
		max = glm::max(glm::vec3{ v.x, v.y, v.z }, max);
	}

	// check the clip space box is within the view
	if (min.z > 1.f || max.z < 0.f || min.x > 1.f || max.x < -1.f ||
			min.y > 1.f || max.y < -1.f) {
		return false;
	} else {
		return true;
	}
}

void Renderer::_geometry_pass(CommandBuffer p_cmd) {
	backend->command_begin_rendering(
			p_cmd, draw_extent, draw_image, depth_image);

	// scene data
	Buffer scene_buffer = backend->buffer_create(sizeof(GeometrySceneData),
			BUFFER_USAGE_UNIFORM_BUFFER_BIT | BUFFER_USAGE_TRANSFER_SRC_BIT,
			MEMORY_ALLOCATION_TYPE_CPU);

	_get_current_frame().deletion_queue.push_function(
			[=, this]() { backend->buffer_free(scene_buffer); });

	GeometrySceneData* scene_buffer_data =
			(GeometrySceneData*)backend->buffer_map(scene_buffer);
	{
		for (const Entity entity :
				SceneView<CameraComponent, Transform>(*scene)) {
			const CameraComponent& cc = *scene->get<CameraComponent>(entity);

			if (cc.is_primary) {
				Transform& transform = *scene->get<Transform>(entity);

				scene_buffer_data->camera_pos =
						glm::vec4(transform.get_position(), 1.0f);
				scene_buffer_data->view = cc.camera.get_view_matrix(transform);
				scene_buffer_data->proj = cc.camera.get_projection_matrix();
				scene_buffer_data->view_proj =
						scene_buffer_data->proj * scene_buffer_data->view;
				scene_buffer_data->sun_direction = SUN_DIRECTION;
				scene_buffer_data->sun_power = SUN_POWER;
				scene_buffer_data->sun_color = SUN_COLOR;

				break;
			}
		}
	}
	backend->buffer_unmap(scene_buffer);

	// start rendering
	static const auto get_proper_material =
			[this](Ref<MaterialInstance> material) -> Ref<MaterialInstance> {
		return !material
				? default_material_instance
				: std::dynamic_pointer_cast<MaterialInstance>(material);
	};

	std::map<Ref<MaterialInstance>,
			std::vector<std::pair<Transform, Ref<Mesh>>>>
			mesh_map;

	for (const Entity entity :
			SceneView<MeshRendererComponent, Transform>(*scene)) {
		const MeshRendererComponent& mesh_component =
				*scene->get<MeshRendererComponent>(entity);
		const Transform& transform = *scene->get<Transform>(entity);

		if (mesh_component.model) {
			for (const auto& mesh : mesh_component.model->meshes) {
				mesh_map[get_proper_material(mesh->material)].push_back(
						std::make_pair(transform, mesh));
			}
		}
	}

	ShaderUniform scene_data_uniform;
	scene_data_uniform.binding = 0;
	scene_data_uniform.type = UNIFORM_TYPE_UNIFORM_BUFFER;
	scene_data_uniform.data.push_back(scene_buffer);

	UniformSet scene_data_set = backend->uniform_set_create(
			scene_data_uniform, default_material->shader, 0);

	_get_current_frame().deletion_queue.push_function(
			[=, this]() { backend->uniform_set_free(scene_data_set); });

	// set dynamic state
	backend->command_set_viewport(p_cmd, draw_extent);
	backend->command_set_scissor(p_cmd, draw_extent);

	for (const auto& [material, meshes] : mesh_map) {
		if (meshes.empty()) {
			continue;
		}

		backend->command_bind_graphics_pipeline(p_cmd, material->pipeline);

		const std::vector<UniformSet> descriptors = {
			scene_data_set,
			material->uniform_set,
		};
		backend->command_bind_uniform_sets(
				p_cmd, material->shader, 0, descriptors);

		for (const auto& [transform, mesh] : meshes) {
			if (!_is_mesh_visible(
						transform, mesh, scene_buffer_data->view_proj)) {
				continue;
			}

			backend->command_bind_index_buffer(
					p_cmd, mesh->index_buffer, 0, mesh->index_type);

			GeometryPushConstants push_constants = {
				.transform = transform.get_transform_matrix(),
				.vertex_buffer = mesh->vertex_buffer_address,
			};

			backend->command_push_constants(p_cmd, material->shader, 0,
					sizeof(GeometryPushConstants), &push_constants);

			backend->command_draw_indexed(p_cmd, mesh->index_count);

			stats.triangle_count += mesh->index_count / 3;
			stats.draw_calls++;
		}
	}

	for (const auto& submit : submit_funcs) {
		submit(backend, p_cmd, draw_image, _get_current_frame().deletion_queue);
	}
	submit_funcs.clear();

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
		backend->swapchain_resize(present_queue, swapchain, window->get_size());
	});
}

void Renderer::_reset_stats() { memset(&stats, 0, sizeof(RenderStats)); }
