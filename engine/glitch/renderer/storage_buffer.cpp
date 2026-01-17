#include "glitch/renderer/storage_buffer.h"

#include "glitch/renderer/renderer.h"

#include <cstring>

namespace gl {

StorageBuffer::~StorageBuffer() {
	Device* device = Renderer::get_device();
	device->buffer_free(_buffer);
}

std::shared_ptr<StorageBuffer> StorageBuffer::create(size_t size, const void* data) {
	Device* device = Renderer::get_device();

	auto buffer_result = device->buffer_create(size,
			BUFFER_USAGE_STORAGE_BUFFER_BIT | BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
					BUFFER_USAGE_TRANSFER_DST_BIT,
			MemoryAllocationType::GPU);

	if (!buffer_result) {
		return nullptr;
	}

	std::shared_ptr<StorageBuffer> sbo = std::make_shared<StorageBuffer>();
	sbo->_buffer = buffer_result.value();
	sbo->_size = size;
	sbo->_gpu_addr = device->buffer_get_device_address(buffer_result.value()).value();

	if (data) {
		sbo->upload(data);
	}

	return sbo;
}

void StorageBuffer::upload(const void* data) {
	GL_ASSERT(data != nullptr);

	Device* device = Renderer::get_device();

	auto staging_buffer_result =
			device->buffer_create(_size, BUFFER_USAGE_TRANSFER_SRC_BIT, MemoryAllocationType::CPU);

	if (!staging_buffer_result) {
		return;
	}

	auto staging_buffer = staging_buffer_result.value();

	uint8_t* staging_data = device->buffer_map(staging_buffer).value();
	memcpy(staging_data, data, _size);
	device->buffer_unmap(staging_buffer);

	// TODO: async data upload
	device->command_immediate_submit([&](CommandBuffer cmd) {
		BufferCopyRegion copy = {};
		copy.src_offset = 0;
		copy.dst_offset = 0;
		copy.size = _size;

		device->command_copy_buffer(cmd, staging_buffer, _buffer, copy);
	});

	device->buffer_free(staging_buffer);
}

BufferDeviceAddress StorageBuffer::get_device_address() const { return _gpu_addr; }

} //namespace gl