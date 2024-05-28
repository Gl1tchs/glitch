#pragma once

#include "gl/core/transform.h"
#include "gl/renderer/material.h"
#include "gl/renderer/mesh.h"

enum class NodeType { NONE, GEOMETRY, COMPUTE, CAMERA, LIGHT };

#define GL_IMPL_NODE(type)                                                     \
	inline NodeType get_type() const override { return type; }

struct Node {
	Transform transform;

	Node* parent = nullptr;
	std::vector<Ref<Node>> children;

	void add_child(Ref<Node> node);

	inline virtual NodeType get_type() const { return NodeType::NONE; }
};

struct GeometryNode : public Node {
	GL_IMPL_NODE(NodeType::GEOMETRY)

	Ref<Mesh> mesh;
	Ref<MaterialInstance> material;

	static void destroy(const GeometryNode* node);
};
