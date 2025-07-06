/**
 * @file scene_graph.h
 *
 */

#pragma once

#include "glitch/core/transform.h"
#include "glitch/core/uid.h"
#include "glitch/renderer/mesh.h"
#include "glitch/renderer/render_queue.h"

struct GL_API SceneNode {
	Transform transform;
	glm::mat4 world_transform;

	std::vector<Ref<SceneNode>> children;

	UID debug_id;
	std::string debug_name = "";

	// Components
	Ref<Mesh> mesh = nullptr;

	void add_child(Ref<SceneNode> p_node);
};

class GL_API SceneGraph {
public:
	SceneGraph();

	Ref<SceneNode> get_root() const;

	Ref<SceneNode> find_by_id(const UID& p_uid);

	RenderQueue construct_render_queue(Frustum p_frustum);

	void update_transforms();

private:
	void _collect_render_items(const Ref<SceneNode>& p_node, Frustum p_frustum,
			RenderQueue& p_queue);

	void _update_node_transform(
			const Ref<SceneNode>& p_node, const glm::mat4& p_parent_transform);

private:
	Ref<SceneNode> root;
};