#pragma once

#include "renderer/node.h"

class SceneGraph {
public:
	SceneGraph();
	~SceneGraph();

	Ref<Node> get_root();

	/**
	 * @brief pushes `node` into the root node's
	 * children array
	 */
	void push_node(Ref<Node> p_node);

	/**
	 * @brief finds node based on `uid`
	 */
	Node* find_node(const UID& p_uid);

	/**
	 * @brief removes node based on `uid`
	 * @returns `true` if successfull `false` otherwise
	 */
	bool remove_node(const UID& p_uid);

	/**
	 * @brief execute function `callback` for all nodes
	 * returning `true` from the `callback` function
	 * ends the recursion.
	 *
	 */
	void traverse(const std::function<bool(Node*)>& p_callback);

	template <NodeDerived T>
	void traverse(const std::function<bool(T*)>& p_callback) {
		_traverse_node<T>(root.get(), p_callback);
	}

private:
	static bool _remove_node(Node* p_parent, const UID& p_uid);

	static void _traverse_node(
			Node* p_node, const std::function<bool(Node*)>& p_callback);

	template <NodeDerived T>
	static void _traverse_node(
			Node* p_node, const std::function<bool(T*)>& p_callback) {
		// TODO: maybe this is a bad practice
		_traverse_node(p_node, [=](Node* node) {
			if (node->get_type() != T::get_static_type()) {
				return false;
			}

			return p_callback(reinterpret_cast<T*>(node));
		});
	}

private:
	Ref<Node> root;
};
