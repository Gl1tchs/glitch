#include "glitch/renderer/texture.h"

#include "glitch/asset/asset_system.h"
#include "glitch/core/hash.h"
#include "glitch/core/json.h"
#include "glitch/renderer/renderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <filesystem>

namespace gl {

Texture::~Texture() {
	auto device = Renderer::get_device();

	device->image_free(_image);
	device->sampler_free(_sampler);
}

std::shared_ptr<Texture> Texture::create(
		const Color& color, const Vec2u& size, TextureSamplerOptions sampler) {
	auto device = Renderer::get_device();

	const uint32_t color_data = color.as_uint();

	std::shared_ptr<Texture> tx = std::make_shared<Texture>();
	tx->_format = DataFormat::R8G8B8A8_UNORM;
	tx->_size = size;
	tx->_image = device->image_create(ImageCreateInfo{ DataFormat::R8G8B8A8_UNORM, size,
											  &color_data, IMAGE_USAGE_SAMPLED_BIT, true })
						 .value();
	tx->_sampler =
			device->sampler_create(SamplerCreateInfo{ sampler.min_filter, sampler.mag_filter,
										   sampler.wrap_u, sampler.wrap_v, sampler.wrap_w,
										   device->image_get_mip_levels(tx->_image).value() })
					.value();
	tx->_sampler_options = sampler;

	return tx;
}

std::shared_ptr<Texture> Texture::create(
		DataFormat format, const Vec2u& size, const void* data, TextureSamplerOptions sampler) {
	auto device = Renderer::get_device();

	std::shared_ptr<Texture> tx = std::make_shared<Texture>();
	tx->_format = format;
	tx->_size = size;
	tx->_image = device->image_create(
							   ImageCreateInfo{ format, size, data, IMAGE_USAGE_SAMPLED_BIT, true })
						 .value();
	tx->_sampler =
			device->sampler_create(SamplerCreateInfo{ sampler.min_filter, sampler.mag_filter,
										   sampler.wrap_u, sampler.wrap_v, sampler.wrap_w,
										   device->image_get_mip_levels(tx->_image).value() })
					.value();
	tx->_sampler_options = sampler;
	tx->_asset_path = "";

	return tx;
}

bool Texture::save(const std::filesystem::path& metadata_path, std::shared_ptr<Texture> texture) {
	if (!texture) {
		GL_LOG_ERROR(
				"[Texture::save] Unable to save Texture metadata to path, invalid texture object.");
		return false;
	}

	if (texture->_asset_path.empty()) {
		GL_LOG_ERROR("[Texture::save] Unable to save Texture metadata to path, asset path should "
					 "not be empty.");
		return false;
	}

	json j;
	j["path"] = texture->_asset_path;
	j["min_filter"] = texture->_sampler_options.min_filter;
	j["mag_filter"] = texture->_sampler_options.mag_filter;
	j["wrap_u"] = texture->_sampler_options.wrap_u;
	j["wrap_v"] = texture->_sampler_options.wrap_v;
	j["wrap_w"] = texture->_sampler_options.wrap_w;

	const auto res = json_save(metadata_path.string(), j);
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

std::shared_ptr<Texture> Texture::load(const std::filesystem::path& path) {
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

	if (!std::filesystem::exists(path)) {
		GL_LOG_ERROR("[Texture::load] Unable to load texture, given metadata path do not exists.");
		return nullptr;
	}

	const auto res = json_load(path.string());
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
	if (!asset_path || !std::filesystem::exists(*asset_path)) {
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
		const std::filesystem::path& asset_path, const TextureSamplerOptions& sampler) {
	if (!std::filesystem::exists(asset_path)) {
		GL_LOG_ERROR(
				"[Texture::load_from_file] Unable to load texture from file, file do not exist.");
		return nullptr;
	}

	auto device = Renderer::get_device();

	int w, h;
	stbi_uc* data = stbi_load(asset_path.string().c_str(), &w, &h, nullptr, STBI_rgb_alpha);

	std::shared_ptr<Texture> tx = std::make_shared<Texture>();
	tx->_format = DataFormat::R8G8B8A8_UNORM;
	tx->_size = { (uint32_t)w, (uint32_t)h };
	tx->_image = device->image_create(ImageCreateInfo{ DataFormat::R8G8B8A8_UNORM,
											  { (uint32_t)w, (uint32_t)h }, data,
											  IMAGE_USAGE_SAMPLED_BIT, true })
						 .value();
	tx->_sampler =
			device->sampler_create(SamplerCreateInfo{ sampler.min_filter, sampler.mag_filter,
										   sampler.wrap_u, sampler.wrap_v, sampler.wrap_w,
										   device->image_get_mip_levels(tx->_image).value() })
					.value();
	tx->_sampler_options = sampler;
	tx->_asset_path = asset_path.string();

	stbi_image_free(data);

	return tx;
}

ShaderUniform Texture::get_uniform(uint32_t binding) const {
	ShaderUniform uniform;
	uniform.type = ShaderUniformType::SAMPLER_WITH_TEXTURE;
	uniform.binding = binding;
	uniform.data.push_back(_sampler);
	uniform.data.push_back(_image);

	return uniform;
}

DataFormat Texture::get_format() const { return _format; }

const Vec2u Texture::get_size() const { return _size; }

const Image Texture::get_image() const { return _image; }

const Sampler Texture::get_sampler() const { return _sampler; }

const std::string& Texture::get_path() const { return _asset_path; }

template <> size_t hash64(const Texture& texture) {
	size_t seed = 0;
	hash_combine(seed, static_cast<int>(texture.get_format()));
	hash_combine(seed, texture.get_image());
	hash_combine(seed, texture.get_sampler());
	return seed;
}

} //namespace gl
