/**
 * @file texture.h
 */

#pragma once

#include "glitch/asset/asset.h"
#include "glitch/core/color.h"
#include "glitch/renderer/types.h"

namespace gl {

struct TextureSamplerOptions {
	ImageFiltering mag_filter = ImageFiltering::LINEAR;
	ImageFiltering min_filter = ImageFiltering::LINEAR;
	ImageWrappingMode wrap_u = ImageWrappingMode::CLAMP_TO_EDGE;
	ImageWrappingMode wrap_v = ImageWrappingMode::CLAMP_TO_EDGE;
	ImageWrappingMode wrap_w = ImageWrappingMode::CLAMP_TO_EDGE;
};

/**
 * High level abstraction over Image handle.  Provides functionality to load
 * image files as well as constructing from raw data.
 */
class Texture {
public:
	GL_REFLECT_ASSET("Texture")

	~Texture();

	// AssetType method overrides

	static std::shared_ptr<Texture> create(const Color& p_color,
			const glm::uvec2& p_size = { 1, 1 }, TextureSamplerOptions p_sampler = {});

	static std::shared_ptr<Texture> create(DataFormat p_format, const glm::uvec2& p_size,
			const void* p_data = nullptr, TextureSamplerOptions p_sampler = {});

	static std::shared_ptr<Texture> load(
			const fs::path& p_path, TextureSamplerOptions p_sampler = {});

	ShaderUniform get_uniform(uint32_t p_binding) const;

	DataFormat get_format() const;

	const Image get_image() const;

	const Sampler get_sampler() const;

private:
	DataFormat format;
	Image image;
	Sampler sampler;
};

template <> size_t hash64(const Texture& p_texture);

} //namespace gl
