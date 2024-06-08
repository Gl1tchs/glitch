#include "renderer/scene_graph.h"

SceneGraph::SceneGraph() : root(create_ref<Node>()) {}

Ref<Node> SceneGraph::get_root() { return root; }

void SceneGraph::push_node(Ref<Node> p_node) { root->add_child(p_node); }

Node* SceneGraph::find_node(const UID& p_uid) {
	Node* result = nullptr;
	traverse([&](Node* node) {
		if (node->uid == p_uid) {
			result = node;
			return true;
		}
		return false;
	});
	return result;
}

bool SceneGraph::remove_node(const UID& p_uid) {
	return _remove_node(root.get(), p_uid);
}

void SceneGraph::traverse(const std::function<bool(Node*)>& p_callback) {
	_traverse_node(root.get(), p_callback);
}

bool SceneGraph::_remove_node(Node* p_parent, const UID& p_uid) {
	auto& children = p_parent->children;
	for (auto it = children.begin(); it != children.end(); ++it) {
		if ((*it)->uid == p_uid) {
			children.erase(it);
			return true;
		} else {
			if (_remove_node(it->get(), p_uid)) {
				return true;
			}
		}
	}
	return false;
}

void SceneGraph::_traverse_node(
		Node* p_node, const std::function<bool(Node*)>& p_callback) {
	if (!p_node) {
		return;
	}

	// end the recursion if callback resulted with `true`
	if (p_callback(p_node)) {
		return;
	}

	for (const auto& child : p_node->children) {
		_traverse_node(child.get(), p_callback);
	}
}
