/**
 * @file texture.h
 */

#pragma once

#include "glitch/core/color.h"
#include "glitch/renderer/types.h"

/**
 * High level abstraction over Image handle.  Provides functionality to load
 * image files as well as constructing from raw data.
 */
class Texture {
public:
	~Texture();

	static Ref<Texture> create(const Color& p_color,
			const Vec2u& p_size = { 1, 1 },
			ImageFiltering p_filtering = IMAGE_FILTERING_LINEAR,
			ImageWrappingMode p_wrapping = IMAGE_WRAPPING_MODE_CLAMP_TO_EDGE);

	static Ref<Texture> create(DataFormat p_format, const Vec2u& p_size,
			const void* p_data = nullptr,
			ImageFiltering p_filtering = IMAGE_FILTERING_LINEAR,
			ImageWrappingMode p_wrapping = IMAGE_WRAPPING_MODE_CLAMP_TO_EDGE);

	static Ref<Texture> load_from_path(const fs::path& p_path,
			ImageFiltering p_filtering = IMAGE_FILTERING_LINEAR,
			ImageWrappingMode p_wrapping = IMAGE_WRAPPING_MODE_CLAMP_TO_EDGE);

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
