#include "glitch/scene/components.h"

template <> size_t hash64(const MaterialComponent& p_material) {
	size_t seed = 0;
	hash_combine(seed, hash64(p_material.base_color));
	hash_combine(seed, p_material.metallic);
	hash_combine(seed, p_material.roughness);
	hash_combine(seed, hash64(p_material.albedo_texture));
	hash_combine(seed, hash64(p_material.normal_texture));

	return seed;
}
