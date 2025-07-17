/**
 * @file shader_library.h
 */

#pragma once

namespace gl {

class GL_API ShaderLibrary {
public:
	static ShaderLibrary& get();

	/**
	 * Retrieve shader from bundled shader library by its' path
	 */
	static std::vector<uint32_t> get_bundled_spirv(const char* p_path);

	/**
	 * Get raw spirv data from file
	 *
	 */
	static std::vector<uint32_t> get_spirv_data(const fs::path& p_filepath);
};

} //namespace gl