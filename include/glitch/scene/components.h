/**
 * @file components.h
 */

#pragma once

#include "glitch/core/hash.h"
#include "glitch/renderer/material.h"
#include "glitch/renderer/texture.h"

enum MaterialType {
	MATERIAL_TYPE_UNLIT,
};

struct MaterialComponent {
	Color base_color = COLOR_WHITE;
	float metallic = 0.0f;
	float roughness = 0.0f;
	Ref<Texture> albedo_texture;
	Ref<Texture> normal_texture;

	MaterialType type = MATERIAL_TYPE_UNLIT;

	// use should provide if `type` == `MATERIAL_TYPE_CUSTOM`
	Ref<MaterialInstance> instance = nullptr;

	size_t hash = 0;
};

template <> size_t hash64(const MaterialComponent& p_material);