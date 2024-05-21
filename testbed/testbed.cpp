#include "testbed.h"
#include "gl/core/transform.h"

#include <gl/core/input.h>
#include <gl/renderer/image.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glm/packing.hpp>

TestBedApplication::TestBedApplication(const ApplicationCreateInfo& info) :
		Application(info) {}

TestBedApplication::~TestBedApplication() {}

void TestBedApplication::_on_start() {
	camera.zoom_level = 2.0f;
	get_renderer()->attach_camera(&camera);

	std::vector<Vertex> vertices = {
		{ { 0.5f, 0.5f, 0.0f }, 1.0f, { 0.0f, 0.0f, 0.0f }, 1.0f,
				{ 0.9f, 0.6f, 0.34f, 1.0f } },
		{ { 0.5f, -0.5f, 0.0f }, 1.0f, { 0.0f, 0.0f, 0.0f }, 0.0f,
				{ 0.2f, 0.4f, 0.8f, 1.0f } },
		{ { -0.5f, -0.5f, 0.0f }, 0.0f, { 0.0f, 0.0f, 0.0f }, 0.0f,
				{ 0.4f, 0.8f, 0.6f, 1.0f } },
		{ { -0.5f, 0.5f, 0.0f }, 0.0f, { 0.0f, 0.0f, 0.0f }, 1.0f,
				{ 0.8f, 0.2f, 0.4f, 1.0f } },
	};

	std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };

	mesh = Mesh::create(vertices, indices);

	material = MetallicRoughnessMaterial::create();

	MetallicRoughnessMaterial::MaterialConstants constants = {
		.color_factors = { 1.0f, 1.0f, 1.0, 1.0f },
		.metal_rough_factors = { 1.0f, 0.5f, 1.0f, 1.0f },
	};

	uint32_t white = glm::packUnorm4x8(glm::vec4(1, 1, 1, 1));
	white_image = Image::create(
			(void*)&white, Vec2u{ 1, 1 }, ImageFormat::R8G8B8A8_UNORM);

	Vec2u size = {};
	int channel_count;
	uint8_t* image_data = stbi_load("assets/t_texture.png", (int*)&size.x,
			(int*)&size.y, &channel_count, STBI_rgb_alpha);

	color_image = Image::create(image_data, size, ImageFormat::R8G8B8A8_UNORM);

	stbi_image_free(image_data);

	image_data = stbi_load("assets/texture.jpg", (int*)&size.x, (int*)&size.y,
			&channel_count, STBI_rgb_alpha);

	color_image2 = Image::create(image_data, size, ImageFormat::R8G8B8A8_UNORM);

	stbi_image_free(image_data);

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

	current_material = material_instance;
}

void TestBedApplication::_on_update(float dt) {
	camera.aspect_ratio = get_window()->get_aspect_ratio();

	if (!space_pressed && Input::is_key_pressed(KeyCode::SPACE)) {
		if (current_material == material_instance) {
			current_material = material_instance2;
		} else {
			current_material = material_instance;
		}
		space_pressed = true;
	}
	if (space_pressed && Input::is_key_released(KeyCode::SPACE)) {
		space_pressed = false;
	}

	static float angle = 0.0f;

	constexpr float radius = 1.0f;
	constexpr float speed = 2.0f;

	angle += speed * dt;

	Transform transform;
	transform.local_position =
			glm::vec3(radius * cos(angle), radius * sin(angle), 0.0f);

	InstanceSubmitData submit_data = {
		.transform = transform.get_transform_matrix(),
	};

	get_renderer()->submit_mesh(mesh, current_material, submit_data);
}

void TestBedApplication::_on_destroy() {
	// wait for operations to finish
	// before cleaning other resources
	get_renderer()->wait_for_device();

	Mesh::destroy(mesh);

	Image::destroy(color_image);
	Image::destroy(color_image2);
	Image::destroy(white_image);

	MetallicRoughnessMaterial::destroy(material);
}
