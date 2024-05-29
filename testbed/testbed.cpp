#include "testbed.h"

#include <gl/core/input.h>
#include <gl/renderer/image.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glm/packing.hpp>

std::vector<Vertex> vertices = {
	Vertex{ .position = { 0.5f, 0.5f, 0.0f },
			.uv_x = 1.0f,
			.normal = { 0.0f, 0.0f, 0.0f },
			.uv_y = 1.0f,
			.color = { 0.9f, 0.6f, 0.34f, 1.0f } },
	Vertex{ .position = { 0.5f, -0.5f, 0.0f },
			.uv_x = 1.0f,
			.normal = { 0.0f, 0.0f, 0.0f },
			.uv_y = 0.0f,
			.color = { 0.2f, 0.4f, 0.8f, 1.0f } },
	Vertex{ .position = { -0.5f, -0.5f, 0.0f },
			.uv_x = 0.0f,
			.normal = { 0.0f, 0.0f, 0.0f },
			.uv_y = 0.0f,
			.color = { 0.4f, 0.8f, 0.6f, 1.0f } },
	Vertex{ .position = { -0.5f, 0.5f, 0.0f },
			.uv_x = 0.0f,
			.normal = { 0.0f, 0.0f, 0.0f },
			.uv_y = 1.0f,
			.color = { 0.8f, 0.2f, 0.4f, 1.0f } },
};

std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };

TestBedApplication::TestBedApplication(const ApplicationCreateInfo& info) :
		Application(info) {}

TestBedApplication::~TestBedApplication() {}

void TestBedApplication::_on_start() {
	camera = create_ref<OrthographicCameraNode>();
	camera->zoom_level = 2.0f;

	get_renderer()->get_scene_graph().push_root(camera);

	material = MetallicRoughnessMaterial::create();

	MetallicRoughnessMaterial::MaterialConstants constants = {
		.color_factors = { 0.9f, 0.8f, 0.7f, 1.0f },
		.metal_rough_factors = { 1.0f, 0.5f, 1.0f, 1.0f },
	};

	{
		uint32_t white = glm::packUnorm4x8(glm::vec4(1, 1, 1, 1));

		ImageCreateInfo white_image_info = {
			.format = ImageFormat::R8G8B8A8_UNORM,
			.size = { 1, 1 },
			.data = &white,
		};
		white_image = Image::create(&white_image_info);
	}

	{
		Vec2u size;
		uint8_t* image_data = stbi_load("assets/texture1.jpg", (int*)&size.x,
				(int*)&size.y, nullptr, STBI_rgb_alpha);

		ImageCreateInfo color_image_info = {
			.format = ImageFormat::R8G8B8A8_UNORM,
			.size = size,
			.data = image_data,
		};
		color_image = Image::create(&color_image_info);

		stbi_image_free(image_data);
	}

	MetallicRoughnessMaterial::MaterialResources resources = {
		.constants = constants,
		.constants_offset = 0,
		.color_image = color_image,
		.color_filtering = ImageFilteringMode::NEAREST,
		.roughness_image = white_image,
		.roughness_filtering = ImageFilteringMode::LINEAR,
	};

	my_node = create_ref<GeometryNode>();
	my_node->mesh = Mesh::create(vertices, indices);
	my_node->material = material->create_instance(resources);

	my_node->transform.local_position.x += 1;

	my_node2 = create_ref<GeometryNode>();
	my_node2->mesh = Mesh::create(vertices, indices);
	my_node2->material = material->create_instance(resources);

	my_node2->transform.local_position.y += 1.5f;

	my_node->add_child(my_node2);

	get_renderer()->get_scene_graph().push_root(my_node);

	const auto window_size = get_window()->get_size();
	ComputeEffectCreateInfo compute_info = {
        .shader_spv_path = "assets/shaders/compute-shader.comp.spv",
		.group_count = {
				(uint32_t)std::ceil(window_size.x / 16.0f),
				(uint32_t)std::ceil(window_size.y / 16.0f),
				1,
		},
	};
	effect_node = ComputeEffectNode::create(&compute_info);

	get_renderer()->get_scene_graph().push_root(effect_node);
}

void TestBedApplication::_on_update(float dt) {
	camera->aspect_ratio = get_window()->get_aspect_ratio();

	my_node->transform.local_rotation.z += 90 * dt;
}

void TestBedApplication::_on_destroy() {
	// wait for operations to finish
	// before cleaning other resources
	get_renderer()->wait_for_device();

	Image::destroy(color_image);
	Image::destroy(white_image);

	MetallicRoughnessMaterial::destroy(material);
}
