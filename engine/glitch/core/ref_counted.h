/**
 * @file counted_ref.h
 *
 */

#pragma once

namespace gl {

/**
 * Atomic reference counter implementation that can be copied and moved,
 * by incrementing the internal ref_count
 *
 */
template <typename T> class RefCounted {
public:
	constexpr RefCounted() : value(nullptr), ref_count(nullptr) {}

	explicit RefCounted(T p_value) :
			value(new T(std::move(p_value))), ref_count(new std::atomic_uint32_t(1)) {}

	RefCounted(const RefCounted& p_rhs) : value(p_rhs.value), ref_count(p_rhs.ref_count) {
		_acquire_ref();
	}

	RefCounted& operator=(const RefCounted& p_rhs) {
		if (this != &p_rhs) {
			release(); // Release current data
			value = p_rhs.value;
			ref_count = p_rhs.ref_count;
			_acquire_ref(); // Acquire new data
		}
		return *this;
	}

	constexpr RefCounted(RefCounted&& p_rhs) noexcept :
			value(p_rhs.value), ref_count(p_rhs.ref_count) {
		// Nullify the source so its destructor doesn't release the data
		p_rhs.value = nullptr;
		p_rhs.ref_count = nullptr;
	}

	RefCounted& operator=(RefCounted&& p_rhs) noexcept {
		if (this != &p_rhs) {
			release(); // Clean up our current data

			// Steal the new data
			value = p_rhs.value;
			ref_count = p_rhs.ref_count;

			// Nullify the source
			p_rhs.value = nullptr;
			p_rhs.ref_count = nullptr;
		}
		return *this;
	}

	virtual ~RefCounted() { release(); }

	void release() {
		if (ref_count) {
			// Atomic decrement returns the PREVIOUS value.
			// If previous was 1, it is now 0, so we delete.
			if (ref_count->fetch_sub(1) == 1) {
				delete value;
				delete ref_count;
			}
		}
	}

	constexpr bool is_valid() const { return ref_count && value; }

	T& get_value() {
		GL_ASSERT(value);
		return *value;
	}
	const T& get_value() const {
		GL_ASSERT(value);
		return *value;
	}

	uint32_t get_ref_count() const { return ref_count ? ref_count->load() : 0; }

	constexpr operator bool() const { return is_valid(); }

	bool operator<(const RefCounted& other) const {
		if (!value && !other.value) {
			return false; // They are equivalent (both null)
		}
		if (!value) {
			return true; // Null is considered "less than" any non-null
		}
		if (!other.value) {
			return false; // Non-null is not "less than" null
		}

		// Compare the underlying values of type T
		return *value < *other.value;
	}

	bool operator==(const RefCounted& other) const {
		if (!value && !other.value) {
			return true;
		}
		if (!value || !other.value) {
			return false;
		}
		return *value == *other.value;
	}

private:
	void _acquire_ref() {
		if (ref_count) {
			(*ref_count)++; // Atomic increment
		}
	}

private:
	T* value;
	std::atomic_uint32_t* ref_count;
};

template <typename T> void to_json(json& p_json, const RefCounted<T>& p_ref) {
	if (p_ref.get_ref_count() > 0) {
		p_json = p_ref.get_value();
	} else {
		p_json = json::value_t::null;
	}
}

template <typename T> void from_json(const json& p_json, RefCounted<T>& p_ref) {
	if (p_json.is_null()) {
		p_ref = RefCounted<T>();
	} else {
		p_ref = RefCounted<T>(p_json.get<T>());
	}
}

} //namespace gl

namespace std {
template <typename T> struct hash;

template <typename T> struct hash<gl::RefCounted<T>> {
	size_t operator()(const gl::RefCounted<T>& p_ref) const { // Check for null pointer first
		if (p_ref.get_ref_count() == 0) {
			return 0;
		}
		return std::hash<T>{}(p_ref.get_value());
	}
};
} //namespace std
