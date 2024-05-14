#include "core/buffer.h"

Buffer::Buffer(uint64_t size) { allocate(size); }

Buffer::Buffer(const void* data, uint64_t size) :
		data((uint8_t*)data), size(size) {}

Buffer Buffer::copy(Buffer other) {
	Buffer result(other.size);
	memcpy(result.data, other.data, other.size);
	return result;
}

void Buffer::allocate(uint64_t size) {
	release();
	data = (uint8_t*)malloc(size);
	this->size = size;
}

void Buffer::release() {
	free(data);
	data = nullptr;
	size = 0;
}

ScopedBuffer::ScopedBuffer(Buffer buffer) : buffer(buffer) {}

ScopedBuffer::ScopedBuffer(uint64_t size) : buffer(size) {}

ScopedBuffer::~ScopedBuffer() { buffer.release(); }

uint8_t* ScopedBuffer::get_data() { return buffer.data; }

const uint8_t* ScopedBuffer::get_data() const { return buffer.data; }

const uint64_t& ScopedBuffer::get_size() const { return buffer.size; }
