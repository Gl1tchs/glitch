#include "glitch/renderer/scene_graph.h"

void SceneNode::add_child(Ref<SceneNode> p_node) {
	p_node->transform.parent = &transform;
	children.push_back(p_node);
}

SceneGraph::SceneGraph() {
	root = create_ref<SceneNode>();
	root->debug_name = "root";
}

Ref<SceneNode> SceneGraph::get_root() const { return root; }

Ref<SceneNode> SceneGraph::create_node(const std::string& name) {
	auto node = create_ref<SceneNode>();
	node->debug_name = name;
	return node;
}

void SceneGraph::update_transforms() { _update_node_transform(root, nullptr); }

void SceneGraph::_update_node_transform(
		const Ref<SceneNode>& node, const Transform* parent) {
	if (parent) {
		node->transform.parent = parent;
	}

	for (auto& child : node->children) {
		_update_node_transform(child, &node->transform);
	}
}
