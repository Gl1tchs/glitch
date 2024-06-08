#include "renderer/node.h"

void Node::add_child(Ref<Node> p_node) {
	p_node->parent = this;
	p_node->transform.parent = &this->transform;

	children.push_back(p_node);
}
