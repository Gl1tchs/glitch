#include "glitch/renderer/storage_buffer.h"

#include "glitch/renderer/renderer.h"

StorageBuffer::~StorageBuffer() {
	Ref<RenderBackend> backend = Renderer::get_backend();
	backend->buffer_free(buffer);
}

Ref<StorageBuffer> StorageBuffer::create(size_t p_size, const void* p_data) {
	Ref<RenderBackend> backend = Renderer::get_backend();

	Buffer buffer = backend->buffer_create(p_size,
			BUFFER_USAGE_STORAGE_BUFFER_BIT |
					BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
					BUFFER_USAGE_TRANSFER_DST_BIT,
			MEMORY_ALLOCATION_TYPE_GPU);

	Ref<StorageBuffer> sbo = create_ref<StorageBuffer>();
	sbo->buffer = buffer;
	sbo->size = p_size;
	sbo->gpu_addr = backend->buffer_get_device_address(buffer);

	if (p_data) {
		sbo->upload(p_data);
	}

	return sbo;
}

void StorageBuffer::upload(const void* p_data) {
	GL_ASSERT(p_data != nullptr);

	Ref<RenderBackend> backend = Renderer::get_backend();

	Buffer staging_buffer = backend->buffer_create(
			size, BUFFER_USAGE_TRANSFER_SRC_BIT, MEMORY_ALLOCATION_TYPE_CPU);

	void* staging_data = backend->buffer_map(staging_buffer);
	memcpy(staging_data, p_data, size);
	backend->buffer_unmap(staging_buffer);

	// TODO: async data upload
	backend->command_immediate_submit([&](CommandBuffer cmd) {
		BufferCopyRegion copy = {};
		copy.src_offset = 0;
		copy.dst_offset = 0;
		copy.size = size;

		backend->command_copy_buffer(cmd, staging_buffer, buffer, copy);
	});

	backend->buffer_free(staging_buffer);
}

BufferDeviceAddress StorageBuffer::get_device_address() const {
	return gpu_addr;
}