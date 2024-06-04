#include "camera_controller.h"

#include <core/event/input.h>

CameraController::CameraController() :
		last_mouse_pos(
				Input::get_mouse_position().x, Input::get_mouse_position().y) {}

void CameraController::set_camera(CameraNode* camera) {
	if (!camera) {
		return;
	}

	this->camera = camera;

	camera->transform.local_position = { -5, 3, 10 };
	camera->transform.local_rotation = { -15, -30, 0 };
	camera->transform.local_scale = { 1, 1, 1 };
}

void CameraController::update(float dt) {
	if (!camera) {
		return;
	}

	glm::vec2 mouse_delta = glm::vec2(Input::get_mouse_position().x,
									Input::get_mouse_position().y) -
			last_mouse_pos;

	glm::vec3 new_rotation = camera->transform.local_rotation +
			glm::vec3(-mouse_delta.y, -mouse_delta.x, 0.0f) * sensitivity;

	// clamp between (-90,90) to make it realistic :)
	new_rotation.x = glm::clamp(new_rotation.x, -89.0f, 89.0f);

	camera->transform.local_rotation = new_rotation;

	// store last mouse pos to prevent instant rotations
	last_mouse_pos = glm::vec2(
			Input::get_mouse_position().x, Input::get_mouse_position().y);

	if (Input::is_key_pressed(KeyCode::LEFT_SHIFT)) {
		speed = SPEED_TURBO;
	} else if (Input::is_key_released(KeyCode::LEFT_SHIFT)) {
		speed = SPEED_DEFAULT;
	}

	// forward / backward controls
	if (Input::is_key_pressed(KeyCode::W)) {
		camera->transform.local_position +=
				camera->transform.get_forward() * speed * dt;
	}
	if (Input::is_key_pressed(KeyCode::S)) {
		camera->transform.local_position -=
				camera->transform.get_forward() * speed * dt;
	}

	// right / left controls
	if (Input::is_key_pressed(KeyCode::D)) {
		camera->transform.local_position +=
				camera->transform.get_right() * speed * dt;
	}
	if (Input::is_key_pressed(KeyCode::A)) {
		camera->transform.local_position -=
				camera->transform.get_right() * speed * dt;
	}

	// up / down controls
	if (Input::is_key_pressed(KeyCode::E)) {
		camera->transform.local_position += WORLD_UP * speed * dt;
	}
	if (Input::is_key_pressed(KeyCode::Q)) {
		camera->transform.local_position -= WORLD_UP * speed * dt;
	}
}

void CameraController::set_speed(float value) { speed = value; }

float CameraController::get_speed() const { return speed; }

void CameraController::set_sensitivity(float value) { sensitivity = value; }

float CameraController::get_sensitivity() const { return sensitivity; }
