#include "renderer/scene_graph.h"

SceneGraph::SceneGraph() : root(create_ref<Node>()) {}

Ref<Node> SceneGraph::get_root() { return root; }

void SceneGraph::push_node(Ref<Node> node) { root->add_child(node); }

Node* SceneGraph::find_node(const UID& uid) {
	Node* result = nullptr;
	traverse([&](Node* node) {
		if (node->uid == uid) {
			result = node;
			return true;
		}
		return false;
	});
	return result;
}

bool SceneGraph::remove_node(const UID& uid) {
	return _remove_node(root.get(), uid);
}

void SceneGraph::traverse(const std::function<bool(Node*)>& callback) {
	_traverse_node(root.get(), callback);
}

bool SceneGraph::_remove_node(Node* parent, const UID& uid) {
	auto& children = parent->children;
	for (auto it = children.begin(); it != children.end(); ++it) {
		if ((*it)->uid == uid) {
			children.erase(it);
			return true;
		} else {
			if (_remove_node(it->get(), uid)) {
				return true;
			}
		}
	}
	return false;
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
