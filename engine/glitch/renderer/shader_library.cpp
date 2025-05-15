#include "glitch/renderer/shader_library.h"

#include "shader_bundle.gen.h"

ShaderLibrary& ShaderLibrary::get() {
	static ShaderLibrary s_shader_library;
	return s_shader_library;
}

std::vector<uint32_t> ShaderLibrary::get_bundled_spirv(const char* p_path) {
	BundleFileData shader_data = {};
	bool shader_found = false;

	for (int i = 0; i < BUNDLE_FILE_COUNT; i++) {
		BundleFileData data = BUNDLE_FILES[i];
		if (strcmp(data.path, p_path) == 0) {
			shader_data = data;
			shader_found = true;
			break;
		}
	}

	if (!shader_found) {
		return {};
	}

	uint32_t* bundle_data = (uint32_t*)&BUNDLE_DATA[shader_data.start_idx];

	return std::vector<uint32_t>(bundle_data, bundle_data + shader_data.size);
}
