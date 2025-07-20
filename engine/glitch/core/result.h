/**
 * @file result.h
 *
 */

#pragma once

namespace gl {

/**
 * @brief Class encapsulating an union of type T and E
 * which can be used to return type safe error types.
 *
 */
template <typename T, typename E> class GL_API Result {
public:
	constexpr Result() : _has_value(false) {}

	constexpr Result(T&& p_val) : _has_value(true) {
		new (&_val) T(std::move(p_val));
	}

	~Result() {
		if (_has_value) {
			_val.~T();
		} else {
			_err.~E();
		}
	}

	constexpr T& get_value() { return _val; }
	constexpr E& get_error() { return _err; }

	constexpr bool has_value() const { return _has_value; }
	constexpr bool has_error() const { return !_has_value; }

	constexpr operator bool() const { return _has_value; }
	constexpr T& operator*() { return _val; }

private:
	union {
		T _val;
		E _err;
	};

	bool _has_value;

	template <typename U, typename V>
	friend constexpr Result<U, V> make_err(V&&);
};

template <typename _T, typename E> constexpr Result<_T, E> make_err(E&& p_err) {
	Result<_T, E> res;
	new (&res._err) E(std::move(p_err));
	res._has_value = false;

	return res;
}

} //namespace gl