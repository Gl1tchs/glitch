#include "camera_controller.h"

#include <glitch/core/event/input.h>

CameraController::CameraController() :
		last_mouse_pos(
				Input::get_mouse_position().x, Input::get_mouse_position().y) {}

void CameraController::set_camera(Camera* p_camera) {
	if (!p_camera) {
		return;
	}

	camera = p_camera;
}

void CameraController::update(float p_dt) {
	if (!camera) {
		return;
	}

	glm::vec2 mouse_delta = Input::get_mouse_position() - last_mouse_pos;

	yaw += -mouse_delta.x * sensitivity;
	pitch += -mouse_delta.y * sensitivity;

	// Clamp pitch to avoid flipping
	pitch = glm::clamp(pitch, -89.0f, 89.0f);

	camera->transform.rotation = { pitch, yaw, 0.0f };

	// store last mouse pos to prevent instant rotations
	last_mouse_pos = Input::get_mouse_position();

	if (Input::is_key_pressed(KEY_CODE_LEFT_SHIFT)) {
		speed = SPEED_TURBO;
	} else if (Input::is_key_released(KEY_CODE_LEFT_SHIFT)) {
		speed = SPEED_DEFAULT;
	}

	// forward / backward controls
	if (Input::is_key_pressed(KEY_CODE_W)) {
		camera->transform.position +=
				camera->transform.get_forward() * speed * p_dt;
	}
	if (Input::is_key_pressed(KEY_CODE_S)) {
		camera->transform.position -=
				camera->transform.get_forward() * speed * p_dt;
	}

	// right / left controls
	if (Input::is_key_pressed(KEY_CODE_D)) {
		camera->transform.position +=
				camera->transform.get_right() * speed * p_dt;
	}
	if (Input::is_key_pressed(KEY_CODE_A)) {
		camera->transform.position -=
				camera->transform.get_right() * speed * p_dt;
	}

	// up / down controls
	if (Input::is_key_pressed(KEY_CODE_E)) {
		camera->transform.position += WORLD_UP * speed * p_dt;
	}
	if (Input::is_key_pressed(KEY_CODE_Q)) {
		camera->transform.position -= WORLD_UP * speed * p_dt;
	}
}

void CameraController::set_speed(float p_value) { speed = p_value; }

float CameraController::get_speed() const { return speed; }

void CameraController::set_sensitivity(float p_value) { sensitivity = p_value; }

float CameraController::get_sensitivity() const { return sensitivity; }
