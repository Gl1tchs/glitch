#pragma once

#include "renderer/mesh.h"

struct Model {
	std::string name = "";
	std::vector<Ref<Mesh>> meshes;

	static Ref<Model> load_gltf(const fs::path& p_path);

	static void destroy(const Ref<Model> p_model);
};
