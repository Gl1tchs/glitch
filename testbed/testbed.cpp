#include "testbed.h"

#include <gl/core/input.h>
#include <gl/renderer/image.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

TestBedApplication::TestBedApplication(const ApplicationCreateInfo& info) :
		Application(info) {}

TestBedApplication::~TestBedApplication() {}

void TestBedApplication::_on_start() {
	camera = create_ref<PerspectiveCameraNode>();

	get_window()->set_cursor_mode(WindowCursorMode::DISABLED);
	camera_controller.set_camera(camera.get());

	get_renderer()->get_scene_graph().push_node(camera);

	material = MetallicRoughnessMaterial::create();

	MetallicRoughnessMaterial::MaterialConstants constants = {
		.color_factors = { 0.9f, 0.8f, 0.7f, 1.0f },
		.metal_rough_factors = { 1.0f, 0.5f, 1.0f, 1.0f },
	};

	constexpr uint32_t white = 0xFFFFFF;

	ImageCreateInfo white_image_info = {
		.format = ImageFormat::R8G8B8A8_UNORM,
		.size = { 1, 1 },
		.data = (void*)&white,
	};
	white_image = Image::create(&white_image_info);

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

	MetallicRoughnessMaterial::MaterialResources resources = {
		.constants = constants,
		.constants_offset = 0,
		.color_image = color_image,
		.color_filtering = ImageFilteringMode::NEAREST,
		.roughness_image = white_image,
		.roughness_filtering = ImageFilteringMode::LINEAR,
	};

	// TODO destroy unusued model
	models = Model::load("assets/basicmesh.glb").value();

	// models[0]->material = material->create_instance(resources);
	models[0]->transform.local_position.x += 1.5f;
	get_renderer()->get_scene_graph().push_node(models[0]);

	models[1]->material = material->create_instance(resources);
	models[1]->transform.local_position.x -= 1.5f;
	get_renderer()->get_scene_graph().push_node(models[1]);

	models[2]->material = material->create_instance(resources);
	models[2]->transform.local_position.x = 3.0f;
	models[2]->transform.local_position.z = 3.0f;
	get_renderer()->get_scene_graph().push_node(models[2]);

#if 0
	const auto window_size = get_window()->get_size();
	Ref<Shader> compute_shader =
			Shader::create("assets/shaders/compute-shader.comp.spv");

	ComputeEffectCreateInfo compute_info = {
        .shader =compute_shader, 
		.group_count ={
				(uint32_t)std::ceil(window_size.x / 16.0f),
				(uint32_t)std::ceil(window_size.y / 16.0f),
				1,
		},
	};
	effect_node = ComputeEffectNode::create(&compute_info);

	Shader::destroy(compute_shader);

	get_renderer()->get_scene_graph().push_root(effect_node);
#endif
}

void TestBedApplication::_on_update(float dt) {
	camera->aspect_ratio = get_window()->get_aspect_ratio();
	camera_controller.update(dt);

	models[0]->transform.local_rotation.z += 90 * dt;
	models[1]->transform.local_rotation.z -= 90 * dt;
	models[2]->transform.local_rotation.y += 90 * dt;
}

void TestBedApplication::_on_destroy() {
	// wait for operations to finish
	// before cleaning other resources
	get_renderer()->wait_for_device();

	for (auto model : models) {
		Model::destroy(model.get());
	}

	Image::destroy(color_image);
	Image::destroy(white_image);

	MetallicRoughnessMaterial::destroy(material);
}
