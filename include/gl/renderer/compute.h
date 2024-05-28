#pragma once

struct ComputeEffectCreateInfo {
	const char* shader_spv_path;
	Vec3u group_count;
};

struct ComputeEffect {
	virtual ~ComputeEffect() = default;

	static Ref<ComputeEffect> create(const ComputeEffectCreateInfo* info);

	static void destroy(Ref<ComputeEffect> effect);
};
