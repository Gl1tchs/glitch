/**
 * @file material.h
 */

#pragma once

#include "glitch/core/color.h"
#include "glitch/core/uid.h"
#include "glitch/renderer/texture.h"
#include "glitch/renderer/types.h"

struct MaterialParameters {
	Color base_color;
	float metallic;
	float roughness;
};

struct MaterialResources {
	Buffer material_data;
	Ref<Texture> albedo_texture;
	Ref<Texture> normal_texture;
};

typedef UID MaterialID;

struct Material;

/**
 * Instance of a material that can only be generate through
 * Material::create_instance
 */
struct GL_API MaterialInstance {
	MaterialID id;
	Pipeline pipeline;
	Shader shader;

	// uniform set corresponding to set = 1 in shader.
	UniformSet uniform_set;
	MaterialResources resources;

	~MaterialInstance();

private:
	MaterialInstance() = default;

	friend struct Material;
};

/**
 * Struct representing a draw data over pipeline and shader.
 * @note Pipeline and shader assumed to be owned by the material
 * whenever the material goes out of scope `pipeline` and `shader`
 * members will be freed.
 */
struct GL_API Material {
	Pipeline pipeline;
	Shader shader;

	virtual ~Material() = 0;

	/**
	 * Create an instance of this material with specified parameters
	 */
	Ref<MaterialInstance> create_instance(const MaterialResources& p_resources);
};
