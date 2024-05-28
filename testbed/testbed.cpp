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
	camera.zoom_level = 2.0f;
	get_renderer()->attach_camera(&camera);

	mesh = Mesh::create(vertices, indices);

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

	{
		Vec2u size;
		uint8_t* image_data = stbi_load("assets/texture.jpg", (int*)&size.x,
				(int*)&size.y, nullptr, STBI_rgb_alpha);

		ImageCreateInfo color_image2_info = {
			.format = ImageFormat::R8G8B8A8_UNORM,
			.size = size,
			.data = image_data,
		};
		color_image2 = Image::create(&color_image2_info);

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
	material_instance = material->create_instance(resources);

	resources.color_image = color_image2;
	material_instance2 = material->create_instance(resources);

	const auto window_size = get_window()->get_size();
	ComputeEffectCreateInfo compute_info = {
        .shader_spv_path = "assets/shaders/compute-shader.comp.spv",
		.group_count = {
				(uint32_t)std::ceil(window_size.x / 16.0f),
				(uint32_t)std::ceil(window_size.y / 16.0f),
				1,
		},
	};
	effect = ComputeEffect::create(&compute_info);
}

void TestBedApplication::_on_update(float dt) {
	camera.aspect_ratio = get_window()->get_aspect_ratio();

	constexpr int element_count = 10;

	static Transform transforms[element_count];

	for (int i = -element_count / 2; i < element_count / 2; ++i) {
		Transform& transform = transforms[i + element_count / 2];

		transform.local_position.x = i / 2.0f;
		transform.local_rotation.z += 90 * dt;
		transform.local_scale = { 0.3f, 0.3, 1.0f };

		InstanceSubmitData submit_data = {
			.transform = transform.get_transform_matrix(),
		};

		if (i % 2 == 0) {
			get_renderer()->submit_mesh(mesh, material_instance, submit_data);
		} else {
			get_renderer()->submit_mesh(mesh, material_instance2, submit_data);
		}
	}

	get_renderer()->submit_compute_effect(effect);
}

void TestBedApplication::_on_destroy() {
	// wait for operations to finish
	// before cleaning other resources
	get_renderer()->wait_for_device();

	ComputeEffect::destroy(effect);

	Mesh::destroy(mesh);

	Image::destroy(color_image);
	Image::destroy(color_image2);
	Image::destroy(white_image);

	MetallicRoughnessMaterial::destroy(material);
}
