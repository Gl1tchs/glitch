/**
 * @file scene_graph.h
 *
 */

#pragma once

#include "glitch/core/transform.h"
#include "glitch/core/uid.h"
#include "glitch/renderer/mesh.h"

struct SceneNode {
	Transform transform;
	std::vector<Ref<SceneNode>> children;

	Ref<Mesh> mesh = nullptr;

	UID debug_id;
	std::string debug_name = "";

	void add_child(Ref<SceneNode> p_node);
};

class SceneGraph {
public:
	SceneGraph();

	Ref<SceneNode> get_root() const;

	Ref<SceneNode> find_by_id(const UID& p_uid);

	void update_transforms(); // optional: global transform propagation

private:
	void _update_node_transform(
			const Ref<SceneNode>& node, const Transform* parent);

private:
	Ref<SceneNode> root;
};