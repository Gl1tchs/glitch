#include "gl/renderer/node.h"

void Node::add_child(Ref<Node> node) {
	node->parent = this;
	node->transform.parent = &this->transform;

	children.push_back(node);
}

void GeometryNode::destroy(const GeometryNode* node) {
	Mesh::destroy(node->mesh);
}
