#include "glitch/renderer/types.h"

namespace gl {

size_t get_data_format_size(DataFormat p_format) {
	switch (p_format) {
		// 8-bit types
		case DataFormat::R8_UNORM:
		case DataFormat::R8_SNORM:
		case DataFormat::R8_USCALED:
		case DataFormat::R8_SSCALED:
		case DataFormat::R8_UINT:
		case DataFormat::R8_SINT:
		case DataFormat::R8_SRGB:
			return 1;
		// 16-bit types
		case DataFormat::R8G8_UNORM:
		case DataFormat::R8G8_SNORM:
		case DataFormat::R8G8_USCALED:
		case DataFormat::R8G8_SSCALED:
		case DataFormat::R8G8_UINT:
		case DataFormat::R8G8_SINT:
		case DataFormat::R8G8_SRGB:
		case DataFormat::R16_UNORM:
		case DataFormat::R16_SNORM:
		case DataFormat::R16_USCALED:
		case DataFormat::R16_SSCALED:
		case DataFormat::R16_UINT:
		case DataFormat::R16_SINT:
		case DataFormat::R16_SFLOAT:
		case DataFormat::D16_UNORM:
			return 2;
		// 24-bit types
		case DataFormat::R8G8B8_UNORM:
		case DataFormat::R8G8B8_SNORM:
		case DataFormat::R8G8B8_USCALED:
		case DataFormat::R8G8B8_SSCALED:
		case DataFormat::R8G8B8_UINT:
		case DataFormat::R8G8B8_SINT:
		case DataFormat::R8G8B8_SRGB:
		case DataFormat::B8G8R8_UNORM:
		case DataFormat::B8G8R8_SNORM:
		case DataFormat::B8G8R8_USCALED:
		case DataFormat::B8G8R8_SSCALED:
		case DataFormat::B8G8R8_UINT:
		case DataFormat::B8G8R8_SINT:
		case DataFormat::B8G8R8_SRGB:
			return 3;
		// 32-bit types
		case DataFormat::R8G8B8A8_UNORM:
		case DataFormat::R8G8B8A8_SNORM:
		case DataFormat::R8G8B8A8_USCALED:
		case DataFormat::R8G8B8A8_SSCALED:
		case DataFormat::R8G8B8A8_UINT:
		case DataFormat::R8G8B8A8_SINT:
		case DataFormat::R8G8B8A8_SRGB:
		case DataFormat::B8G8R8A8_UNORM:
		case DataFormat::B8G8R8A8_SNORM:
		case DataFormat::B8G8R8A8_USCALED:
		case DataFormat::B8G8R8A8_SSCALED:
		case DataFormat::B8G8R8A8_UINT:
		case DataFormat::B8G8R8A8_SINT:
		case DataFormat::B8G8R8A8_SRGB:
		case DataFormat::A8B8G8R8_UNORM_PACK32:
		case DataFormat::A8B8G8R8_SNORM_PACK32:
		case DataFormat::A8B8G8R8_USCALED_PACK32:
		case DataFormat::A8B8G8R8_SSCALED_PACK32:
		case DataFormat::A8B8G8R8_UINT_PACK32:
		case DataFormat::A8B8G8R8_SINT_PACK32:
		case DataFormat::A8B8G8R8_SRGB_PACK32:
		case DataFormat::R16G16_UNORM:
		case DataFormat::R16G16_SNORM:
		case DataFormat::R16G16_USCALED:
		case DataFormat::R16G16_SSCALED:
		case DataFormat::R16G16_UINT:
		case DataFormat::R16G16_SINT:
		case DataFormat::R16G16_SFLOAT:
		case DataFormat::R32_UINT:
		case DataFormat::R32_SINT:
		case DataFormat::R32_SFLOAT:
		case DataFormat::D24_UNORM_S8_UINT:
		case DataFormat::D32_SFLOAT:
			return 4;
		// 48-bit types
		case DataFormat::R16G16B16_UNORM:
		case DataFormat::R16G16B16_SNORM:
		case DataFormat::R16G16B16_USCALED:
		case DataFormat::R16G16B16_SSCALED:
		case DataFormat::R16G16B16_UINT:
		case DataFormat::R16G16B16_SINT:
		case DataFormat::R16G16B16_SFLOAT:
			return 6;
		// 64-bit types
		case DataFormat::R16G16B16A16_UNORM:
		case DataFormat::R16G16B16A16_SNORM:
		case DataFormat::R16G16B16A16_USCALED:
		case DataFormat::R16G16B16A16_SSCALED:
		case DataFormat::R16G16B16A16_UINT:
		case DataFormat::R16G16B16A16_SINT:
		case DataFormat::R16G16B16A16_SFLOAT:
		case DataFormat::R32G32_UINT:
		case DataFormat::R32G32_SINT:
		case DataFormat::R32G32_SFLOAT:
			return 8;
		case DataFormat::R32G32B32_UINT:
		case DataFormat::R32G32B32_SINT:
		case DataFormat::R32G32B32_SFLOAT:
			return 12;
		case DataFormat::R32G32B32A32_UINT:
		case DataFormat::R32G32B32A32_SINT:
		case DataFormat::R32G32B32A32_SFLOAT:
			return 16;
		default:
			return 0; // Invalid format
	}
}

} //namespace gl