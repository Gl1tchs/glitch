/**
 * @file buffer.h
 *
 */

#pragma once

#include "glitch/core/defines.h"

#include <glgpu/glgpu.h>

#include <memory>

namespace gl {

/**
 * Storage buffer wrapper for easier access and data transfers
 */
class GL_API StorageBuffer {
public:
	~StorageBuffer();

	static std::shared_ptr<StorageBuffer> create(size_t size, const void* data = nullptr);

	void upload(const void* data);

	BufferDeviceAddress get_device_address() const;

private:
	Buffer _buffer;
	size_t _size;
	BufferDeviceAddress _gpu_addr;
};

} //namespace gl