#pragma once

enum class ImageFormat {
	// 8-bit per channel formats
	R8_UNORM,
	R8G8_UNORM,
	R8G8B8_UNORM,
	R8G8B8A8_UNORM,
	// 16-bit per channel formats
	R16_UNORM,
	R16G16_UNORM,
	R16G16B16_UNORM,
	R16G16B16A16_UNORM,
	R16G16B16A16_SFLOAT,
	// 32-bit per channel formats
	R32_SFLOAT,
	R32G32_SFLOAT,
	R32G32B32_SFLOAT,
	R32G32B32A32_SFLOAT,
	// packed formats
	B8G8R8_UNORM,
	B8G8R8A8_UNORM,
	// depth/stencil formats
	D16_UNORM,
	D24_UNORM_S8_UINT,
	D32_SFLOAT,
	D32_SFLOAT_S8_UINT,
};

enum class ImageFilteringMode {
	LINEAR,
	NEAREST,
};

struct Image {
	virtual ~Image() = default;

	static Ref<Image> create(
			Vec2u size, ImageFormat format, bool mipmapped = false);
	static Ref<Image> create(const void* data, Vec2u size,
			ImageFormat format, bool mipmapped = false);

	static void destroy(Ref<Image> image);
};
