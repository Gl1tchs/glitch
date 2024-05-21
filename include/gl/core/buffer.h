#pragma once

#include "gl/core/assert.h"

struct Buffer {
	uint8_t* data = nullptr;
	uint64_t size = 0;

	Buffer() = default;
	Buffer(uint64_t size);
	Buffer(const void* data, uint64_t size);
	Buffer(const Buffer&) = default;

	static Buffer copy(Buffer other);

	void allocate(uint64_t size);

	void release();

	template <typename T> inline T* as() { return (T*)data; }

	operator bool() const { return (bool)data; }
};

struct ScopedBuffer {
	ScopedBuffer(Buffer buffer);
	ScopedBuffer(uint64_t size);
	~ScopedBuffer();

	uint8_t* get_data();
	const uint8_t* get_data() const;
	const uint64_t& get_size() const;

	template <typename T> inline T* as() { return buffer.as<T>(); }

	operator bool() const { return buffer; }

private:
	Buffer buffer;
};

template <typename T> struct BufferArray {
	BufferArray() = default;
	BufferArray(const uint64_t p_max_elements) :
			buffer(p_max_elements * sizeof(T)), max_elements(p_max_elements) {}

	~BufferArray() {
		if (buffer) {
			buffer.release();
		}
	}

	inline uint8_t* get_data() { return buffer.data; }
	inline const uint8_t* get_data() const { return buffer.data; }

	inline const uint64_t& get_size() const { return buffer.size; }

	inline const uint32_t& get_count() const { return count; }

	inline void allocate(const uint64_t p_max_elements) {
		max_elements = p_max_elements;
		buffer.allocate(max_elements * sizeof(T));
	}

	inline void release() {
		count = 0;
		max_elements = 0;
		buffer.release();
	}

	inline void add(const T& value) {
		// prevent memory leaks
		GL_ASSERT(buffer && count < max_elements);
		buffer.as<T>()[count++] = value;
	}

	inline T& at(const uint32_t idx) {
		GL_ASSERT(buffer && idx < max_elements);
		return buffer.as<T>()[idx];
	}

	inline void clear() {
		GL_ASSERT(buffer);
		int64_t max_elements_copy = max_elements;
		release();
		allocate(max_elements_copy);
	}

	inline void reset_index() { count = 0; }

	inline T& operator[](const uint32_t idx) { return at(idx); }

	operator bool() const { return buffer; }

private:
	Buffer buffer;
	uint32_t count = 0;
	uint32_t max_elements = 0;
};
