#pragma once

#include "gl/core/transform.h"

enum class NodeType { NONE, GEOMETRY, COMPUTE, CAMERA, LIGHT };

#define GL_IMPL_NODE(type)                                                     \
	inline NodeType get_type() const override { return type; }                 \
	inline static NodeType get_static_type() { return type; }

struct Node {
	Transform transform;

	Node* parent = nullptr;
	std::vector<Ref<Node>> children;

	void add_child(Ref<Node> node);

	inline virtual NodeType get_type() const { return NodeType::NONE; }
	inline static NodeType get_static_type() { return NodeType::NONE; }
};

template <typename T>
concept NodeDerived = std::is_base_of_v<Node, T>;
