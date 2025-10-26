/**
 * @file vector_view.h
 */

#pragma once

namespace gl {

/**
 * A very light weight span-like vector abstraction with only a pointer
 * and size, that can be indexed and compared.
 */
template <typename T> class VectorView {
public:
	VectorView() = default;
	VectorView(const T& p_ptr) :
			// With this one you can pass a single element very conveniently!
			ptr(&p_ptr), length(1) {}
	VectorView(const T* p_ptr, size_t p_size) : ptr(p_ptr), length(p_size) {}
	VectorView(const std::vector<T>& p_lv) :
			ptr(p_lv.data()), length(p_lv.size()) {}
	VectorView(const VectorView& p_other) :
			ptr(p_other.ptr), length(p_other.length) {}
	VectorView(VectorView&& p_other) :
			ptr(std::move(p_other.ptr)), length(std::move(p_other.length)) {}

	VectorView& operator=(const VectorView& p_other) {
		ptr = p_other.ptr;
		length = p_other.length;
		return *this;
	}

	const T& operator[](size_t p_index) const {
		GL_ASSERT(p_index < length);
		return ptr[p_index];
	}

	const T* data() const { return ptr; }
	size_t size() const { return length; }

	size_t len() const { return length / sizeof(T); }

	// For STL USAGE
	const T* begin() const { return ptr; }
	const T* end() const { return ptr + length; }

private:
	const T* ptr = nullptr;
	size_t length = 0;
};

} //namespace gl