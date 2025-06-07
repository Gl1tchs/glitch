/**
 * @file texture.h
 */

#pragma once

#include "glitch/core/color.h"
#include "glitch/renderer/types.h"

struct TextureSamplerOptions {
	ImageFiltering mag_filter = IMAGE_FILTERING_LINEAR;
	ImageFiltering min_filter = IMAGE_FILTERING_LINEAR;
	ImageWrappingMode wrap_u = IMAGE_WRAPPING_MODE_CLAMP_TO_EDGE;
	ImageWrappingMode wrap_v = IMAGE_WRAPPING_MODE_CLAMP_TO_EDGE;
	ImageWrappingMode wrap_w = IMAGE_WRAPPING_MODE_CLAMP_TO_EDGE;
};

/**
 * High level abstraction over Image handle.  Provides functionality to load
 * image files as well as constructing from raw data.
 */
class Texture {
public:
	~Texture();

	static Ref<Texture> create(const Color& p_color,
			const glm::uvec2& p_size = { 1, 1 },
			TextureSamplerOptions p_sampler = {});

	static Ref<Texture> create(DataFormat p_format, const glm::uvec2& p_size,
			const void* p_data = nullptr, TextureSamplerOptions p_sampler = {});

	static Ref<Texture> load_from_path(
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
