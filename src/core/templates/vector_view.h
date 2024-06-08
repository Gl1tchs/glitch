#pragma once

template <typename T> class VectorView {
public:
	VectorView() = default;
	VectorView(const T& p_ptr) :
			// With this one you can pass a single element very conveniently!
			_ptr(&p_ptr), _size(1) {}
	VectorView(const T* p_ptr, uint32_t p_size) : _ptr(p_ptr), _size(p_size) {}
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

	const T& operator[](uint32_t p_index) {
		GL_ASSERT(p_index < _size);
		return _ptr[p_index];
	}

	inline const T* data() const { return _ptr; }
	inline uint32_t size() const { return _size; }

private:
	const T* _ptr = nullptr;
	uint32_t _size = 0;
};
