/**
 * @file buffer.h
 *
 */

#pragma once

#include "glitch/renderer/types.h"

/**
 * Storage buffer wrapper for easier access and data transfers
 */
class StorageBuffer {
public:
	~StorageBuffer();

	static Ref<StorageBuffer> create(
			size_t p_size, const void* p_data = nullptr);

	void upload(const void* p_data);

	BufferDeviceAddress get_device_address() const;

private:
	Buffer buffer;
	size_t size;
	BufferDeviceAddress gpu_addr;
};