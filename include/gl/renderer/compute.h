#pragma once

struct ComputeEffectCreateInfo {
	uint32_t group_count_x;
	uint32_t group_count_y;
	uint32_t group_count_z;
	const char* shader_spv_path;
};

struct ComputeEffect {
	virtual ~ComputeEffect() = default;

	static Ref<ComputeEffect> create(const ComputeEffectCreateInfo* info);

	static void destroy(Ref<ComputeEffect> effect);
};
