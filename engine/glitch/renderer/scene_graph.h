/**
 * @file scene_graph.h
 *
 */

#pragma once

#include "glitch/core/transform.h"
#include "glitch/renderer/mesh.h"

struct SceneNode {
	Transform transform;
	std::vector<Ref<SceneNode>> children;

	Ref<Mesh> mesh = nullptr;

	std::string debug_name = "";

	void add_child(Ref<SceneNode> p_node);
};

class SceneGraph {
public:
	SceneGraph();

	Ref<SceneNode> get_root() const;

	Ref<SceneNode> create_node(const std::string& name = "");

	void update_transforms(); // optional: global transform propagation

private:
	void _update_node_transform(
			const Ref<SceneNode>& node, const Transform* parent);

private:
	Ref<SceneNode> root;
};