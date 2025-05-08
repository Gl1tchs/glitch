/**
 * @file mesh_renderer.h
 */

#pragma once

#include "glitch/renderer/material.h"

class MeshRenderer {
public:
	MeshRenderer();
	~MeshRenderer();

	void render(Ref<MaterialInstance> p_material);
};
