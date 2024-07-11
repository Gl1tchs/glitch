#pragma once

#include "renderer/mesh.h"

struct Model {
	std::string name = "";
	std::vector<Ref<Mesh>> meshes;

	static Ref<Model> load(const fs::path& p_path, Ref<Material> p_material);

	static void destroy(const Ref<Model> p_model);
};
