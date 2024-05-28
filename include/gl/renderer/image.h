#pragma once

enum class ImageFormat {
	R8_UNORM,
	R8G8_UNORM,
	R8G8B8_UNORM,
	R8G8B8A8_UNORM,
	R16_UNORM,
	R16G16_UNORM,
	R16G16B16_UNORM,
	R16G16B16A16_UNORM,
	R16G16B16A16_SFLOAT,
	R32_SFLOAT,
	R32G32_SFLOAT,
	R32G32B32_SFLOAT,
	R32G32B32A32_SFLOAT,
	B8G8R8_UNORM,
	B8G8R8A8_UNORM,
	D16_UNORM,
	D24_UNORM_S8_UINT,
	D32_SFLOAT,
	D32_SFLOAT_S8_UINT,
};

enum class ImageFilteringMode {
	LINEAR,
	NEAREST,
};

struct ImageCreateInfo {
	ImageFormat format;
	Vec2u size;
	void* data = nullptr;
	bool mipmapped = false;
};

struct Image {
	virtual ~Image() = default;

	static Ref<Image> create(const ImageCreateInfo* info);

	static void destroy(Ref<Image> image);
};
