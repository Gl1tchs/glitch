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
	using ValueType = T;
	using ErrorType = E;

	constexpr Result() noexcept = delete;

	constexpr Result(ValueType&& p_val) noexcept : _data(std::move(p_val)) {}
	constexpr Result(const ValueType& p_val) noexcept : _data(p_val) {}

	constexpr explicit Result(ErrorType&& p_err) : _data(std::move(p_err)) {}
	constexpr explicit Result(const ErrorType& p_err) : _data(p_err) {}

	constexpr bool has_value() const noexcept {
		return std::holds_alternative<ValueType>(_data);
	}

	constexpr bool has_error() const noexcept {
		return std::holds_alternative<ErrorType>(_data);
	}

	constexpr explicit operator bool() const noexcept { return has_value(); }

	constexpr ValueType& get_value() noexcept {
		return std::get<ValueType>(_data);
	}
	constexpr const ValueType& get_value() const noexcept {
		return std::get<ValueType>(_data);
	}

	constexpr ErrorType& get_error() noexcept {
		return std::get<ErrorType>(_data);
	}
	constexpr const ErrorType& get_error() const noexcept {
		return std::get<ErrorType>(_data);
	}

	constexpr ValueType& operator*() noexcept { return get_value(); }
	constexpr const ValueType& operator*() const noexcept {
		return get_value();
	}

	constexpr bool operator==(const Result& other) const noexcept {
		return _data == other._data;
	}

private:
	std::variant<ValueType, ErrorType> _data;

	template <typename U, typename V> friend constexpr Result<U, V> make_err(V);
};

template <typename T, typename E> constexpr Result<T, E> make_err(E p_err) {
	return Result<T, E>(std::forward<E>(p_err));
}

} //namespace gl