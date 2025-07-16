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
			_ptr(&p_ptr), _size(1) {}
	VectorView(const T* p_ptr, size_t p_size) : _ptr(p_ptr), _size(p_size) {}
	VectorView(const std::vector<T>& p_lv) :
			_ptr(p_lv.data()), _size(p_lv.size()) {}
	VectorView(const VectorView& p_other) :
			_ptr(p_other._ptr), _size(p_other._size) {}
	VectorView(VectorView&& p_other) :
			_ptr(std::move(p_other._ptr)), _size(std::move(p_other._size)) {}

	VectorView& operator=(const VectorView& p_other) {
		_ptr = p_other._ptr;
		_size = p_other._size;
		return *this;
	}

	const T& operator[](size_t p_index) const {
		GL_ASSERT(p_index < _size);
		return _ptr[p_index];
	}

	const T* data() const { return _ptr; }
	size_t size() const { return _size; }

	size_t len() const { return _size / sizeof(T); }

	// For STL USAGE
	const T* begin() const { return _ptr; }
	const T* end() const { return _ptr + _size; }

private:
	const T* _ptr = nullptr;
	size_t _size = 0;
};

} //namespace gl