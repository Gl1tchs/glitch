#pragma once

#include "gl/renderer/node.h"

class SceneGraph {
public:
	SceneGraph();
	~SceneGraph() = default;

	Ref<Node> get_root();

	/**
	 * @brief pushes `node` into the root node's
	 * children array
	 */
	void push_root(Ref<Node> node);

	/**
	 * @brief execute function `callback` for all nodes
	 * returning `true` from the `callback` function
	 * ends the recursion.
	 *
	 */
	void traverse(const std::function<bool(Node*)>& callback);

private:
	void traverse_node(Node* node, const std::function<bool(Node*)>& callback);

private:
	Ref<Node> root;
};
