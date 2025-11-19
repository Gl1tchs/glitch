#include "glitch/renderer/texture.h"

#include "glitch/asset/asset_system.h"
#include "glitch/core/hash.h"
#include "glitch/core/json.h"
#include "glitch/renderer/renderer.h"
#include "glitch/renderer/types.h"
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace gl {

Texture::~Texture() {
	auto backend = Renderer::get_backend();

	backend->image_free(image);
	backend->sampler_free(sampler);
}

std::shared_ptr<Texture> Texture::create(
		const Color& p_color, const glm::uvec2& p_size, TextureSamplerOptions p_sampler) {
	auto backend = Renderer::get_backend();

	const uint32_t color_data = p_color.as_uint();

	std::shared_ptr<Texture> tx = std::make_shared<Texture>();
	tx->format = DataFormat::R8G8B8A8_UNORM;
	tx->size = p_size;
	tx->image = backend->image_create(
			DataFormat::R8G8B8A8_UNORM, p_size, &color_data, IMAGE_USAGE_SAMPLED_BIT, true);
	tx->sampler =
			backend->sampler_create(p_sampler.min_filter, p_sampler.mag_filter, p_sampler.wrap_u,
					p_sampler.wrap_v, p_sampler.wrap_w, backend->image_get_mip_levels(tx->image));

	return tx;
}

std::shared_ptr<Texture> Texture::create(DataFormat p_format, const glm::uvec2& p_size,
		const void* p_data, TextureSamplerOptions p_sampler) {
	auto backend = Renderer::get_backend();

	std::shared_ptr<Texture> tx = std::make_shared<Texture>();
	tx->format = p_format;
	tx->size = p_size;
	tx->image = backend->image_create(p_format, p_size, p_data, IMAGE_USAGE_SAMPLED_BIT, true);
	tx->sampler =
			backend->sampler_create(p_sampler.min_filter, p_sampler.mag_filter, p_sampler.wrap_u,
					p_sampler.wrap_v, p_sampler.wrap_w, backend->image_get_mip_levels(tx->image));
	tx->sampler_options = p_sampler;
	tx->asset_path = "";

	return tx;
}

bool Texture::save(const fs::path& p_metadata_path, std::shared_ptr<Texture> p_texture) {
	if (!p_texture) {
		GL_LOG_ERROR(
				"[Texture::save] Unable to save Texture metadata to path, invalid texture object.");
		return false;
	}

	if (!fs::exists(p_metadata_path)) {
		GL_LOG_ERROR(
				"[Texture::save] Unable to save Texture metadata to path, path doesn't exist.");
		return false;
	}

	if (p_texture->asset_path.empty()) {
		GL_LOG_ERROR("[Texture::save] Unable to save Texture metadata to path, asset path should "
					 "not be empty.");
		return false;
	}

	json j;
	j["path"] = p_texture->asset_path;
	j["min_filter"] = p_texture->sampler_options.min_filter;
	j["mag_filter"] = p_texture->sampler_options.mag_filter;
	j["wrap_u"] = p_texture->sampler_options.wrap_u;
	j["wrap_v"] = p_texture->sampler_options.wrap_v;
	j["wrap_w"] = p_texture->sampler_options.wrap_w;

	const auto res = json_save(p_metadata_path.string(), j);
	if (res != JSONLoadError::NONE) {
		if (res == JSONLoadError::FILE_OPEN_ERROR) {
			GL_LOG_ERROR(
					"[Texture::save] Unable to save Texture metadata to path, file open error.");
		} else if (res == JSONLoadError::INVALID_PATH) {
			GL_LOG_ERROR("[Texture::save] Unable to save Texture metadata to path, invalid path.");
		}
		return false;
	}

	return true;
}

std::shared_ptr<Texture> Texture::load(const fs::path& p_path) {
	/**
	 * Example metadata reference:
	 * {
	 *  "path" : "res://texture.png",
	 *  "min_filter" : "linear",
	 *  "mag_filter" : "linear",
	 *  "wrap_u" : "clamp_to_edge",
	 *  "wrap_v" : "clamp_to_edge",
	 *  "wrap_w" : "clamp_to_edge"
	 * }
	 */

	if (!fs::exists(p_path)) {
		GL_LOG_ERROR("[Texture::load] Unable to load texture, given metadata path do not exists.");
		return nullptr;
	}

	const auto res = json_load(p_path.string());
	if (!res) {
		GL_LOG_ERROR("[Texture::load] Unable to load texture, error while parsing metadata.");
		return nullptr;
	}

	const json& j = *res;

	if (!j.contains("path")) {
		GL_LOG_ERROR("[Texture::load] Unable to load texture, metadata does not contain path.");
		return nullptr;
	}

	const auto asset_path_rel = j["path"].get<std::string>();
	const auto asset_path = AssetSystem::get_absolute_path(asset_path_rel);
	if (!asset_path || !fs::exists(*asset_path)) {
		GL_LOG_ERROR("[Texture::load] Unable to load texture, invalid textue path in metadata.");
		return nullptr;
	}

	TextureSamplerOptions sampler_options;
	if (j.contains("min_filter")) {
		j["min_filter"].get_to(sampler_options.min_filter);
	}
	if (j.contains("mag_filter")) {
		j["mag_filter"].get_to(sampler_options.min_filter);
	}
	if (j.contains("wrap_u")) {
		j["wrap_u"].get_to(sampler_options.wrap_u);
	}
	if (j.contains("wrap_v")) {
		j["wrap_v"].get_to(sampler_options.wrap_v);
	}
	if (j.contains("wrap_w")) {
		j["wrap_w"].get_to(sampler_options.wrap_w);
	}

	return load_from_file(*asset_path, sampler_options);
}

std::shared_ptr<Texture> Texture::load_from_file(
		const fs::path& p_asset_path, const TextureSamplerOptions& p_sampler) {
	if (!fs::exists(p_asset_path)) {
		GL_LOG_ERROR(
				"[Texture::load_from_file] Unable to load texture from file, file do not exist.");
		return nullptr;
	}

	auto backend = Renderer::get_backend();

	int w, h;
	stbi_uc* data = stbi_load(p_asset_path.string().c_str(), &w, &h, nullptr, STBI_rgb_alpha);

	std::shared_ptr<Texture> tx = std::make_shared<Texture>();
	tx->format = DataFormat::R8G8B8A8_UNORM;
	tx->size = { w, h };
	tx->image =
			backend->image_create(DataFormat::R8G8B8A8_UNORM, { (uint32_t)w, (uint32_t)h }, data);
	tx->sampler = backend->sampler_create(p_sampler.min_filter, p_sampler.mag_filter,
			p_sampler.wrap_u, p_sampler.wrap_v, p_sampler.wrap_w);
	tx->sampler_options = p_sampler;
	tx->asset_path = p_asset_path.string();

	stbi_image_free(data);

	return tx;
}

ShaderUniform Texture::get_uniform(uint32_t p_binding) const {
	ShaderUniform uniform;
	uniform.type = UNIFORM_TYPE_SAMPLER_WITH_TEXTURE;
	uniform.binding = p_binding;
	uniform.data.push_back(sampler);
	uniform.data.push_back(image);

	return uniform;
}

DataFormat Texture::get_format() const { return format; }

const glm::uvec2 Texture::get_size() const { return size; }

const Image Texture::get_image() const { return image; }

const Sampler Texture::get_sampler() const { return sampler; }

const std::string& Texture::get_path() const { return asset_path; }

template <> size_t hash64(const Texture& p_texture) {
	size_t seed = 0;
	hash_combine(seed, static_cast<int>(p_texture.get_format()));
	hash_combine(seed, p_texture.get_image());
	hash_combine(seed, p_texture.get_sampler());
	return seed;
}

} //namespace gl
