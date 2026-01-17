/**
 * @file texture.h
 */

#pragma once

#include "glitch/asset/asset.h"
#include "glitch/core/hash.h"

#include <glgpu/glgpu.h>

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

	static std::shared_ptr<Texture> create(
			const Color& color, const Vec2u& size = { 1, 1 }, TextureSamplerOptions sampler = {});

	static std::shared_ptr<Texture> create(DataFormat format, const Vec2u& size,
			const void* data = nullptr, TextureSamplerOptions sampler = {});

	static bool save(const std::filesystem::path& metadata_path, std::shared_ptr<Texture> texture);
	static std::shared_ptr<Texture> load(const std::filesystem::path& metadata_path);

	static std::shared_ptr<Texture> load_from_file(
			const std::filesystem::path& asset_path, const TextureSamplerOptions& sampler = {});

	ShaderUniform get_uniform(uint32_t binding) const;

	DataFormat get_format() const;

	const Vec2u get_size() const;

	const Image get_image() const;

	const Sampler get_sampler() const;

	const std::string& get_path() const;

private:
	DataFormat _format;
	Image _image;
	Sampler _sampler;
	Vec2u _size;

	std::string _asset_path;
	TextureSamplerOptions _sampler_options;
};

static_assert(IsCreatableAsset<Texture, Color, Vec2u, TextureSamplerOptions>);
static_assert(IsLoadableAsset<Texture>);

template <> size_t hash64(const Texture& texture);

} //namespace gl
