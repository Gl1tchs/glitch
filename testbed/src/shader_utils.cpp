#include "shader_utils.h"

std::vector<uint32_t> get_spirv_data(const fs::path& p_filepath) {
	size_t file_size = fs::file_size(p_filepath);

	std::ifstream file(p_filepath, std::ios::in | std::ios::binary);
	if (!file.is_open()) {
		return {};
	}

	std::vector<uint32_t> buffer(file_size);
	file.read(reinterpret_cast<char*>(buffer.data()), file_size);
	return buffer;
}
