/**
 * @file counted_ref.h
 *
 */

#pragma once

#include "glitch/core/core.h"

#include <atomic>
#include <cstdint>
#include <utility>

namespace gl {

/**
 * Atomic reference counter implementation that can be copied and moved,
 * by incrementing the internal ref_count
 *
 */
template <typename T> class RefCounted {
public:
	constexpr RefCounted() : _value(nullptr), _ref_count(nullptr) {}

	explicit RefCounted(T value) :
			_value(new T(std::move(value))), _ref_count(new std::atomic_uint32_t(1)) {}

	RefCounted(const RefCounted& rhs) : _value(rhs._value), _ref_count(rhs._ref_count) {
		_acquire_ref();
	}

	RefCounted& operator=(const RefCounted& rhs) {
		if (this != &rhs) {
			release(); // Release current data
			_value = rhs._value;
			_ref_count = rhs._ref_count;
			_acquire_ref(); // Acquire new data
		}
		return *this;
	}

	constexpr RefCounted(RefCounted&& rhs) noexcept :
			_value(rhs._value), _ref_count(rhs._ref_count) {
		// Nullify the source so its destructor doesn't release the data
		rhs._value = nullptr;
		rhs._ref_count = nullptr;
	}

	RefCounted& operator=(RefCounted&& rhs) noexcept {
		if (this != &rhs) {
			release(); // Clean up our current data

			// Steal the new data
			_value = rhs._value;
			_ref_count = rhs._ref_count;

			// Nullify the source
			rhs._value = nullptr;
			rhs._ref_count = nullptr;
		}
		return *this;
	}

	virtual ~RefCounted() { release(); }

	void release() {
		if (_ref_count) {
			// Atomic decrement returns the PREVIOUS value.
			// If previous was 1, it is now 0, so we delete.
			if (_ref_count->fetch_sub(1) == 1) {
				delete _value;
				delete _ref_count;
			}
		}
	}

	constexpr bool is_valid() const { return _ref_count && _value; }

	T& get_value() {
		GL_ASSERT(_value);
		return *_value;
	}
	const T& get_value() const {
		GL_ASSERT(_value);
		return *_value;
	}

	uint32_t get_ref_count() const { return _ref_count ? _ref_count->load() : 0; }

	constexpr operator bool() const { return is_valid(); }

	bool operator<(const RefCounted& other) const {
		if (!_value && !other._value) {
			return false; // They are equivalent (both null)
		}
		if (!_value) {
			return true; // Null is considered "less than" any non-null
		}
		if (!other._value) {
			return false; // Non-null is not "less than" null
		}

		// Compare the underlying values of type T
		return *_value < *other._value;
	}

	bool operator==(const RefCounted& other) const {
		if (!_value && !other._value) {
			return true;
		}
		if (!_value || !other._value) {
			return false;
		}
		return *_value == *other._value;
	}

private:
	void _acquire_ref() {
		if (_ref_count) {
			(*_ref_count)++; // Atomic increment
		}
	}

private:
	T* _value;
	std::atomic_uint32_t* _ref_count;
};

template <typename T> void to_json(json& j, const RefCounted<T>& ref) {
	if (ref.get_ref_count() > 0) {
		j = ref.get_value();
	} else {
		j = json::value_t::null;
	}
}

template <typename T> void from_json(const json& j, RefCounted<T>& ref) {
	if (j.is_null()) {
		ref = RefCounted<T>();
	} else {
		ref = RefCounted<T>(j.get<T>());
	}
}

} //namespace gl

namespace std {
template <typename T> struct hash;

template <typename T> struct hash<gl::RefCounted<T>> {
	size_t operator()(const gl::RefCounted<T>& ref) const { // Check for null pointer first
		if (ref.get_ref_count() == 0) {
			return 0;
		}
		return std::hash<T>{}(ref.get_value());
	}
};
} //namespace std
