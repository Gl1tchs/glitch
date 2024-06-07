#include "renderer/renderer.h"

#include "core/application.h"

#include "platform/vulkan/vk_buffer.h"
#include "platform/vulkan/vk_descriptors.h"
#include "renderer/camera.h"
#include "renderer/mesh.h"
#include "renderer/node.h"
#include "renderer/types.h"

#include "platform/vulkan/vk_backend.h"
#include "platform/vulkan/vk_commands.h"
#include "platform/vulkan/vk_image.h"
#include "platform/vulkan/vk_queue.h"
#include "platform/vulkan/vk_swapchain.h"
#include "platform/vulkan/vk_sync.h"

[[nodiscard]] GraphicsAPI find_proper_api() noexcept {
	return GRAPHICS_API_VULKAN;
}

static GraphicsAPI s_api;

Renderer::Renderer(Ref<Window> p_window) : window(p_window) {
	s_api = find_proper_api();

	backend = create_ref<VulkanRenderBackend>();
	context = backend->init(p_window);

	swapchain = vk::swapchain_create();
	vk::swapchain_resize(context,
			backend->get_command_queue(QUEUE_TYPE_PRESENT), swapchain,
			p_window->get_size());

	draw_image = vk::image_create(context, DATA_FORMAT_R16G16B16A16_SFLOAT,
			p_window->get_size(), nullptr,
			IMAGE_USAGE_TRANSFER_SRC_BIT | IMAGE_USAGE_TRANSFER_DST_BIT |
					IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

	depth_image = vk::image_create(context, DATA_FORMAT_D32_SFLOAT,
			p_window->get_size(), nullptr,
			IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

	for (uint8_t i = 0; i < SWAPCHAIN_BUFFER_SIZE; i++) {
		FrameData& frame_data = frames[i];

		frame_data.command_pool = vk::command_pool_create(
				context, backend->get_command_queue(QUEUE_TYPE_GRAPHICS));
		frame_data.command_buffer =
				vk::command_pool_allocate(context, frame_data.command_pool);

		frame_data.image_available_semaphore = vk::semaphore_create(context);
		frame_data.render_finished_semaphore = vk::semaphore_create(context);

		frame_data.render_fence = vk::fence_create(context);
	}

	default_material = Material::create(context);

	constexpr uint32_t magenta_color = 0xFF00FF;

	magenta_image = vk::image_create(
			context, DATA_FORMAT_R8G8B8A8_UNORM, { 1, 1 }, &magenta_color);
	default_sampler = vk::sampler_create(context);

	Material::MaterialResources resources = {};
	resources.color_image = magenta_image;
	resources.color_sampler = default_sampler;

	default_material_instance =
			default_material->create_instance(context, resources);
}

Renderer::~Renderer() {
	backend->wait_for_device();

	Material::destroy(context, default_material);

	vk::image_free(context, magenta_image);
	vk::sampler_free(context, default_sampler);

	_destroy_scene_graph();

	for (uint8_t i = 0; i < SWAPCHAIN_BUFFER_SIZE; i++) {
		FrameData& frame_data = frames[i];

		frame_data.deletion_queue.flush();

		vk::command_pool_free(context, frame_data.command_pool);

		vk::semaphore_free(context, frame_data.image_available_semaphore);
		vk::semaphore_free(context, frame_data.render_finished_semaphore);

		vk::fence_free(context, frame_data.render_fence);
	}

	vk::image_free(context, depth_image);
	vk::image_free(context, draw_image);

	vk::swapchain_free(context, swapchain);

	backend->shutdown(context);
}

void Renderer::wait_and_render() {
	vk::fence_wait(context, _get_current_frame().render_fence);

	_get_current_frame().deletion_queue.flush();

	Optional<Image> swapchain_image = vk::swapchain_acquire_image(
			context, swapchain, _get_current_frame().image_available_semaphore);
	if (!swapchain_image.has_value()) {
		_request_resize();
		return;
	}

	vk::fence_reset(context, _get_current_frame().render_fence);

	const Vec2u swapchain_extent = vk::swapchain_get_extent(swapchain);
	const Vec3u draw_image_extent = vk::image_get_size(draw_image);

	// set render scale
	const float render_scale = get_settings().render_scale;
	draw_extent = Vec2u(
			std::min(swapchain_extent.x, draw_image_extent.x) * render_scale,
			std::min(swapchain_extent.y, draw_image_extent.y) * render_scale);

	CommandBuffer cmd = _get_current_frame().command_buffer;

	vk::command_reset(cmd);

	vk::command_begin(cmd);
	{
		vk::command_transition_image(
				cmd, draw_image, IMAGE_LAYOUT_UNDEFINED, IMAGE_LAYOUT_GENERAL);

		vk::command_clear_color(cmd, draw_image, { 0.1f, 0.1f, 0.1f, 1.0f });

		vk::command_transition_image(cmd, draw_image, IMAGE_LAYOUT_GENERAL,
				IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		vk::command_transition_image(cmd, depth_image, IMAGE_LAYOUT_UNDEFINED,
				IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

		_geometry_pass(cmd);

		vk::command_transition_image(cmd, draw_image,
				IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		vk::command_transition_image(cmd, swapchain_image.value(),
				IMAGE_LAYOUT_UNDEFINED, IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		vk::command_copy_image_to_image(cmd, draw_image,
				swapchain_image.value(), draw_extent, swapchain_extent);

		vk::command_transition_image(cmd, swapchain_image.value(),
				IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, IMAGE_LAYOUT_PRESENT_SRC);
	}
	vk::command_end(cmd);

	vk::queue_submit(backend->get_command_queue(QUEUE_TYPE_GRAPHICS), cmd,
			_get_current_frame().render_fence,
			_get_current_frame().image_available_semaphore,
			_get_current_frame().render_finished_semaphore);

	if (!vk::queue_present(context,
				backend->get_command_queue(QUEUE_TYPE_GRAPHICS), swapchain,
				_get_current_frame().render_finished_semaphore)) {
		_request_resize();
	}

	frame_number++;
}

void Renderer::wait_for_device() { backend->wait_for_device(); }

void Renderer::_geometry_pass(CommandBuffer p_cmd) {
	vk::command_begin_rendering(p_cmd, draw_extent, draw_image, depth_image);
	{
		// scene data
		Buffer scene_data_buffer = vk::buffer_create(context,
				sizeof(GPUSceneData),
				BUFFER_USAGE_UNIFORM_BUFFER_BIT | BUFFER_USAGE_TRANSFER_SRC_BIT,
				MEMORY_ALLOCATION_TYPE_CPU);

		_get_current_frame().deletion_queue.push_function(
				[=, this]() { vk::buffer_free(context, scene_data_buffer); });

		GPUSceneData* scene_uniform_data =
				(GPUSceneData*)vk::buffer_map(context, scene_data_buffer);
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
		vk::buffer_unmap(context, scene_data_buffer);

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
				[=, this]() { vk::uniform_set_free(context, scene_data_set); });

		bool global_descriptor_set = false;

		for (const auto& [material, meshes] : mesh_map) {
			if (meshes.empty()) {
				continue;
			}

			if (!global_descriptor_set) {
				BoundUniform scene_data_uniform;
				scene_data_uniform.binding = 0;
				scene_data_uniform.type = UNIFORM_TYPE_UNIFORM_BUFFER;
				scene_data_uniform.ids.push_back(scene_data_buffer);

				scene_data_set = vk::uniform_set_create(
						context, scene_data_uniform, material->shader, 0);

				global_descriptor_set = true;
			}

			vk::command_bind_graphics_pipeline(p_cmd, material->pipeline);

			// set dynamic state
			vk::command_set_viewport(
					p_cmd, { (float)draw_extent.x, (float)draw_extent.y });
			vk::command_set_scissor(p_cmd, draw_extent);

			std::vector<UniformSet> descriptors = {
				scene_data_set,
				material->uniform_set,
			};
			vk::command_bind_uniform_sets(
					p_cmd, material->shader, 0, descriptors);

			for (const auto& mesh : meshes) {
				vk::command_bind_index_buffer(
						p_cmd, mesh->index_buffer, 0, INDEX_TYPE_UINT32);

				GPUDrawPushConstants push_constants = {
					.transform = mesh->transform.get_transform_matrix(),
					.vertex_buffer = mesh->vertex_buffer_address,
				};

				vk::command_push_constants(p_cmd, material->shader, 0,
						sizeof(GPUDrawPushConstants), &push_constants);

				vk::command_draw_indexed(p_cmd, mesh->index_count);
			}
		}
	}
	vk::command_end_rendering(p_cmd);
}

GraphicsAPI Renderer::get_graphics_api() { return s_api; }

void Renderer::_request_resize() {
	Application::get_instance()->enqueue_main_thread([&]() {
		vk::swapchain_resize(context,
				backend->get_command_queue(QUEUE_TYPE_PRESENT), swapchain,
				window->get_size());
	});
}

void Renderer::_destroy_scene_graph() {
	scene_graph.traverse([this](Node* node) {
		switch (node->get_type()) {
			case NodeType::NONE: {
				break;
			}
			case NodeType::GEOMETRY: {
				Mesh::destroy(context, (const Mesh*)node);
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
