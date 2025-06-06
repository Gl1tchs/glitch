#include "camera_controller.h"

#include <glitch/core/event/input.h>

CameraController::CameraController() :
		last_mouse_pos(
				Input::get_mouse_position().x, Input::get_mouse_position().y) {}

void CameraController::set_camera(Camera* p_camera, Transform* p_transform) {
	if (!p_camera || !p_transform) {
		return;
	}

	camera = p_camera;
	transform = p_transform;
}

void CameraController::update(float p_dt) {
	if (!camera) {
		return;
	}

	glm::vec2 mouse_delta = Input::get_mouse_position() - last_mouse_pos;

	yaw += -mouse_delta.x * sensitivity;
	pitch += -mouse_delta.y * sensitivity;

	// Clamp pitch to avoid flipping
	pitch = glm::clamp(pitch, glm::radians(-89.0f), glm::radians(89.0f));

	// Rebuild rotation quaternion:
	glm::fquat qPitch = glm::angleAxis(pitch, VEC3_RIGHT);
	glm::fquat qYaw = glm::angleAxis(yaw, VEC3_UP);

	// Final rotation: yaw first, then pitch
	transform->rotation = qYaw * qPitch;

	// store last mouse pos to prevent instant rotations
	last_mouse_pos = Input::get_mouse_position();

	if (Input::is_key_pressed(KEY_CODE_LEFT_SHIFT)) {
		speed = SPEED_TURBO;
	} else if (Input::is_key_released(KEY_CODE_LEFT_SHIFT)) {
		speed = SPEED_DEFAULT;
	}

	// forward / backward controls
	if (Input::is_key_pressed(KEY_CODE_W)) {
		transform->position += transform->get_forward() * speed * p_dt;
	}
	if (Input::is_key_pressed(KEY_CODE_S)) {
		transform->position -= transform->get_forward() * speed * p_dt;
	}

	// right / left controls
	if (Input::is_key_pressed(KEY_CODE_D)) {
		transform->position += transform->get_right() * speed * p_dt;
	}
	if (Input::is_key_pressed(KEY_CODE_A)) {
		transform->position -= transform->get_right() * speed * p_dt;
	}

	// up / down controls
	if (Input::is_key_pressed(KEY_CODE_E)) {
		transform->position += WORLD_UP * speed * p_dt;
	}
	if (Input::is_key_pressed(KEY_CODE_Q)) {
		transform->position -= WORLD_UP * speed * p_dt;
	}
}

void CameraController::set_speed(float p_value) { speed = p_value; }

float CameraController::get_speed() const { return speed; }

void CameraController::set_sensitivity(float p_value) { sensitivity = p_value; }

float CameraController::get_sensitivity() const { return sensitivity; }
