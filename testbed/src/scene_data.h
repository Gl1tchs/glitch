#pragma once

struct SceneData {
	glm::mat4 view;
	glm::mat4 projection;
	glm::vec3 camera_position;
	float delta_time;
};
