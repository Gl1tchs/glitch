#pragma once

class Shader {
public:
	virtual ~Shader() = default;

	/**
	 * @brief loads shader from file
	 */
	static Ref<Shader> create(const char* file_path);

	/**
	 * @brief loads shader from spirv binary
	 */
	static Ref<Shader> create(uint32_t spirv_size, uint32_t* spirv_data);

	static void destroy(Ref<Shader> shader);
};
