#include "camera_controller.h"

#include <core/event/input.h>

CameraController::CameraController() :
		last_mouse_pos(
				Input::get_mouse_position().x, Input::get_mouse_position().y) {}

void CameraController::set_camera(Camera* p_camera, Transform* p_transform) {
	if (!p_camera || !p_transform) {
		return;
	}

	camera = p_camera;
	transform = p_transform;

	p_transform->local_position = { -5, 3, 10 };
	p_transform->local_rotation = { -15, -30, 0 };
	p_transform->local_scale = { 1, 1, 1 };
}

void CameraController::update(float p_dt) {
	if (!camera) {
		return;
	}

	glm::vec2 mouse_delta = glm::vec2(Input::get_mouse_position().x,
									Input::get_mouse_position().y) -
			last_mouse_pos;

	glm::vec3 new_rotation = transform->local_rotation +
			glm::vec3(-mouse_delta.y, -mouse_delta.x, 0.0f) * sensitivity;

	// clamp between (-90,90) to make it realistic :)
	new_rotation.x = glm::clamp(new_rotation.x, -89.0f, 89.0f);

	transform->local_rotation = new_rotation;

	// store last mouse pos to prevent instant rotations
	last_mouse_pos = glm::vec2(
			Input::get_mouse_position().x, Input::get_mouse_position().y);

	if (Input::is_key_pressed(KEY_CODE_LEFT_SHIFT)) {
		speed = SPEED_TURBO;
	} else if (Input::is_key_released(KEY_CODE_LEFT_SHIFT)) {
		speed = SPEED_DEFAULT;
	}

	// forward / backward controls
	if (Input::is_key_pressed(KEY_CODE_W)) {
		transform->local_position += transform->get_forward() * speed * p_dt;
	}
	if (Input::is_key_pressed(KEY_CODE_S)) {
		transform->local_position -= transform->get_forward() * speed * p_dt;
	}

	// right / left controls
	if (Input::is_key_pressed(KEY_CODE_D)) {
		transform->local_position += transform->get_right() * speed * p_dt;
	}
	if (Input::is_key_pressed(KEY_CODE_A)) {
		transform->local_position -= transform->get_right() * speed * p_dt;
	}

	// up / down controls
	if (Input::is_key_pressed(KEY_CODE_E)) {
		transform->local_position += WORLD_UP * speed * p_dt;
	}
	if (Input::is_key_pressed(KEY_CODE_Q)) {
		transform->local_position -= WORLD_UP * speed * p_dt;
	}
}

void CameraController::set_speed(float p_value) { speed = p_value; }

float CameraController::get_speed() const { return speed; }

void CameraController::set_sensitivity(float p_value) { sensitivity = p_value; }

float CameraController::get_sensitivity() const { return sensitivity; }
