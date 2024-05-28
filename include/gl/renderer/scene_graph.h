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

	template <NodeDerived T>
	void traverse(const std::function<bool(T*)>& callback) {
		_traverse_node<T>(root.get(), callback);
	}

private:
	static void _traverse_node(
			Node* node, const std::function<bool(Node*)>& callback);

	template <NodeDerived T>
	static void _traverse_node(
			Node* node, const std::function<bool(T*)>& callback) {
		// TODO: maybe this is a bad practice
		_traverse_node(node, [=](Node* node) {
			if (node->get_type() != T::get_static_type()) {
				return false;
			}

			return callback(reinterpret_cast<T*>(node));
		});
	}

private:
	Ref<Node> root;
};
