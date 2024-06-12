#pragma once

#include "core/transform.h"
#include "core/uid.h"

enum NodeType {
	NODE_TYPE_NONE,
	NODE_TYPE_GEOMETRY,
	NODE_TYPE_COMPUTE,
	NODE_TYPE_CAMERA,
	NODE_TYPE_LIGHT,
};

#define GL_IMPL_NODE(type)                                                     \
	inline NodeType get_type() const override { return type; }                 \
	inline static NodeType get_static_type() { return type; }

struct Node {
	UID uid;
	std::string name = "Node";

	Transform transform;

	Node* parent = nullptr;
	std::vector<Ref<Node>> children;

	void add_child(Ref<Node> p_node);

	inline virtual NodeType get_type() const { return NODE_TYPE_NONE; }
	inline static NodeType get_static_type() { return NODE_TYPE_NONE; }
};

template <typename T>
concept NodeDerived = std::is_base_of_v<Node, T>;
