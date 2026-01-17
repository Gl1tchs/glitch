/**
 * @file shader_library.h
 */

#pragma once

#include "glitch/core/defines.h"

#include <filesystem>
#include <vector>

namespace gl {

class GL_API ShaderLibrary {
public:
	static ShaderLibrary& get();

	/**
	 * Retrieve shader from bundled shader library by its' path
	 */
	static std::vector<uint32_t> get_bundled_spirv(const char* path);

	/**
	 * Get raw spirv data from file
	 *
	 */
	static std::vector<uint32_t> get_spirv_data(const std::filesystem::path& filepath);
};

} //namespace gl