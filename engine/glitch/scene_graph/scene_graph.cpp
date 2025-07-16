#include "glitch/scene_graph/scene_graph.h"

namespace gl {

void SceneNode::add_child(Ref<SceneNode> p_node) { children.push_back(p_node); }

SceneGraph::SceneGraph() {
	root = create_ref<SceneNode>();
	root->debug_name = "root";
}

Ref<SceneNode> SceneGraph::get_root() const { return root; }

Ref<SceneNode> SceneGraph::find_by_id(const UID& p_uid) {
	Ref<SceneNode> result = nullptr;

	std::function<void(Ref<SceneNode>)> traverse;
	traverse = [&](Ref<SceneNode> p_node) {
		if (result != nullptr) {
			return;
		}

		if (p_node->debug_id == p_uid) {
			result = p_node;
			return;
		}

		for (const auto& child : p_node->children) {
			traverse(child);
		}
	};

	traverse(root);

	return result;
}

RenderQueue SceneGraph::construct_render_queue(Frustum p_frustum) {
	RenderQueue render_queue;
	_collect_render_items(root, p_frustum, render_queue);
	return render_queue;
}

void SceneGraph::update_transforms() {
	_update_node_transform(root, glm::mat4(1.0f));
}

void SceneGraph::_collect_render_items(
		const Ref<SceneNode>& p_node, Frustum p_frustum, RenderQueue& p_queue) {
	if (p_node->mesh) {
		for (auto& prim : p_node->mesh->primitives) {
			// If objects is not inside of the view frustum, discard it.
			AABB aabb = prim->aabb.transform(p_node->world_transform);
			if (!aabb.is_inside_frustum(p_frustum)) {
				continue;
			}

			RenderObject item;
			item.transform = p_node->world_transform;
			item.primitive = prim;
			p_queue.add(item);
		}
	}

	for (const auto& child : p_node->children) {
		_collect_render_items(child, p_frustum, p_queue);
	}
}

void SceneGraph::_update_node_transform(
		const Ref<SceneNode>& p_node, const glm::mat4& p_parent_transform) {
	if (!p_node) {
		return;
	}

	p_node->world_transform = p_parent_transform * p_node->transform.to_mat4();

	// Recurse to children
	for (auto& child : p_node->children) {
		_update_node_transform(child, p_node->world_transform);
	}
}

} //namespace gl