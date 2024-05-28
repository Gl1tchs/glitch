#include "gl/renderer/scene_graph.h"

SceneGraph::SceneGraph() : root(create_ref<Node>()) {}

Ref<Node> SceneGraph::get_root() { return root; }

void SceneGraph::push_root(Ref<Node> node) { root->add_child(node); }

void SceneGraph::traverse(const std::function<bool(Node*)>& callback) {
	_traverse_node(root.get(), callback);
}

void SceneGraph::_traverse_node(
		Node* node, const std::function<bool(Node*)>& callback) {
	if (!node) {
		return;
	}

	// end the recursion if callback resulted with `true`
	if (callback(node)) {
		return;
	}

	for (const auto& child : node->children) {
		_traverse_node(child.get(), callback);
	}
}
