#include "glitch/renderer/texture.h"

#include "glitch/core/hash.h"
#include "glitch/renderer/render_backend.h"
#include "glitch/renderer/render_device.h"
#include "glitch/renderer/types.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

Texture::~Texture() {
	Ref<RenderBackend> backend = RenderDevice::get_backend();

	backend->image_free(image);
	backend->sampler_free(sampler);
}

Ref<Texture> Texture::create(const Color& p_color, const glm::uvec2& p_size,
		ImageFiltering p_filtering, ImageWrappingMode p_wrapping) {
	Ref<RenderBackend> backend = RenderDevice::get_backend();

	const uint32_t color_data = p_color.as_uint();

	Ref<Texture> tx = create_ref<Texture>();
	tx->format = DATA_FORMAT_R8G8B8A8_UNORM;
	tx->image = backend->image_create(
			DATA_FORMAT_R8G8B8A8_UNORM, p_size, &color_data);
	tx->sampler = backend->sampler_create(
			p_filtering, p_filtering, p_wrapping, p_wrapping, p_wrapping);

	return tx;
}

Ref<Texture> Texture::create(DataFormat p_format, const glm::uvec2& p_size,
		const void* p_data, ImageFiltering p_filtering,
		ImageWrappingMode p_wrapping) {
	Ref<RenderBackend> backend = RenderDevice::get_backend();

	Ref<Texture> tx = create_ref<Texture>();
	tx->format = p_format;
	tx->image = backend->image_create(p_format, p_size, p_data);
	tx->sampler = backend->sampler_create(
			p_filtering, p_filtering, p_wrapping, p_wrapping, p_wrapping);

	return tx;
}

Ref<Texture> Texture::load_from_path(const fs::path& p_path,
		ImageFiltering p_filtering, ImageWrappingMode p_wrapping) {
	Ref<RenderBackend> backend = RenderDevice::get_backend();

	int w, h;
	stbi_uc* data =
			stbi_load(p_path.string().c_str(), &w, &h, nullptr, STBI_rgb_alpha);

	Ref<Texture> tx = create_ref<Texture>();
	tx->format = DATA_FORMAT_R8G8B8A8_UNORM;
	tx->image = backend->image_create(
			DATA_FORMAT_R8G8B8A8_UNORM, { (uint32_t)w, (uint32_t)h }, data);
	tx->sampler = backend->sampler_create(
			p_filtering, p_filtering, p_wrapping, p_wrapping, p_wrapping);

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

const Image Texture::get_image() const { return image; }

const Sampler Texture::get_sampler() const { return sampler; }

template <> size_t hash64(const Texture& p_texture) {
	size_t seed = 0;
	hash_combine(seed, static_cast<int>(p_texture.get_format()));
	hash_combine(seed, p_texture.get_image());
	hash_combine(seed, p_texture.get_sampler());
	return seed;
}
