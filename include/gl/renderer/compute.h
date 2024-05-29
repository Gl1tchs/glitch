#pragma once

#include "gl/renderer/node.h"
#include "gl/renderer/shader.h"

struct ComputeEffectCreateInfo {
	Ref<Shader> shader;
	Vec3u group_count;
};

struct ComputeEffectNode : public Node {
	GL_IMPL_NODE(NodeType::COMPUTE)

	virtual ~ComputeEffectNode() = default;

	static Ref<ComputeEffectNode> create(const ComputeEffectCreateInfo* info);

	static void destroy(const ComputeEffectNode* effect);
};
